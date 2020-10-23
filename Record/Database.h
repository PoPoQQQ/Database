#pragma once
#include <set>
#include <string>
#include <cstring>
#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#else
#include <io.h>
#include <direct.h>
#endif //__linux__
#include "Table.h"
#include "../Utils/StringValidator.h"
using namespace std;

class Database {
public:
	static set<string> databases;
	static Database *currentDatabase;
	char databaseName[MAX_STRING_LEN + 1];
	map<string, Table*> tables;

	static void ShowDatabases() {
		for(set<string>::iterator it = databases.begin(); it != databases.end(); it++)
			cout << *it << endl;
	}

	static void LoadDatabases() {
		if (_access("Database", 0) == -1)
	        _mkdir("Database");
	
	    databases.clear();

		_finddata_t fileinfo;
		int hFile = _findfirst("Database/*", &fileinfo);
		if(hFile != -1)
		{
			do {
				if(fileinfo.attrib & _A_SUBDIR)  
				{  
				    if(strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)  
				    {
				    	StringValidator::Check(fileinfo.name);
				    	databases.insert(fileinfo.name);
				    }    
				}
			} while(_findnext(hFile, &fileinfo) == 0);  
			_findclose(hFile);
		}

		//ShowDatabases();
	}

	static void CreateDatabase(const char *databaseName) {
		StringValidator::Check(databaseName);
		if(databases.find(databaseName) != databases.end())
		{
			cerr << "Database already exists!" << endl;
			exit(-1);
		}
		databases.insert(databaseName);
		if (_access("Database", 0) == -1)
	        _mkdir("Database");
	    char dir[1000];
	    sprintf(dir, "Database/%s", databaseName);
	    if (_access(dir, 0) == -1)
	        _mkdir(dir);
	}

	static void OpenDatabase(const char *databaseName) {
		StringValidator::Check(databaseName);
		if(databases.find(databaseName) == databases.end())
		{
			cerr << "Database not found!" << endl;
			exit(-1);
		}
		if(currentDatabase != NULL)
		{
			if(!strcmp(databaseName, currentDatabase->databaseName))
				return;
			CloseDatabase();
		}
		currentDatabase = new Database;
		memset(currentDatabase->databaseName, 0, sizeof currentDatabase->databaseName);
		strcpy(currentDatabase->databaseName, databaseName);

		_finddata_t fileinfo;
		static char dir[1000];
		sprintf(dir, "Database/%s/*", databaseName);
		int hFile = _findfirst(dir, &fileinfo);
		if(hFile != -1)
		{
			do {
				if((fileinfo.attrib & _A_SUBDIR) == 0)  
				{
					StringValidator::Check(fileinfo.name);
					currentDatabase->tables[fileinfo.name] = new Table(currentDatabase->databaseName, fileinfo.name);
				}
			} while(_findnext(hFile, &fileinfo) == 0);  
			_findclose(hFile);
		}
	}

	static void CloseDatabase() {
		if(currentDatabase == NULL)
		{
			cerr << "Current database does not exist!" << endl;
			exit(-1);
		}
		for(map<string, Table*>::iterator it = currentDatabase->tables.begin(); it != currentDatabase->tables.end(); it++)
			delete it->second;
		delete currentDatabase;
		currentDatabase = NULL;
	}

	static Table *CreateTable(const char *tableName, TableHeader *tableHeader) {
		if(currentDatabase == NULL) {
			cerr << "Current database does not exist!" << endl;
			exit(-1);
		}
		StringValidator::Check(tableName);
		if(currentDatabase->tables.find(tableName) != currentDatabase->tables.end()) {
			cerr << "Table already exsists!" << endl;
			exit(-1);
		}
		tableHeader->SetDatabaseName(currentDatabase->databaseName);
		tableHeader->SetTableName(tableName);
		return currentDatabase->tables[tableName] = new Table(tableHeader);
	}

	static Table *GetTable(const char *tableName) {
		if(currentDatabase == NULL)
		{
			cerr << "Current database does not exist!" << endl;
			exit(-1);
		}
		StringValidator::Check(tableName);
		if(currentDatabase->tables.find(tableName) != currentDatabase->tables.end()) {
			cerr << "Table not found!" << endl;
			exit(-1);
		}
		return currentDatabase->tables[tableName];
	}
};

set<string> Database::databases;
Database* Database::currentDatabase = NULL;

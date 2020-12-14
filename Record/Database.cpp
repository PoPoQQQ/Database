#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>
#else // default WIN64
#include <io.h>
#include <direct.h>
#endif //__linux__

#include <string>
#include <errno.h>
#include <cstring>
#include <algorithm>
#include "Database.h"

void RemoveDirectory(const char* dir) {
	char __dir[1000];
	sprintf(__dir, "%s/*", dir);
    _finddata_t fileinfo;
    int hFile = _findfirst(__dir, &fileinfo);
	if(hFile != -1)
	{
		do {
			if(!strcmp(fileinfo.name, "."))
				continue;
			if(!strcmp(fileinfo.name, ".."))
				continue;
			char _dir[1000];
			sprintf(_dir, "%s/%s", dir, fileinfo.name);
			if(fileinfo.attrib & _A_SUBDIR)  
				RemoveDirectory(_dir);
			else
				remove(_dir);
		} while(_findnext(hFile, &fileinfo) == 0);  
		_findclose(hFile);
	}
	rmdir(dir);   
}

set<string> Database::databases;
Database* Database::currentDatabase = NULL;

void Database::ShowDatabases() {
	int maxLength = 8;
	for(set<string>::iterator it = databases.begin(); it != databases.end(); it++)
		maxLength = max(maxLength, (int)it->length());

	cout << "+-";
	for(int i = 0; i < maxLength; i++)
		cout << "-";
	cout << "-+" << endl;

	cout << "| Database";
	for(int i = 0; i < maxLength - 8; i++)
		cout << " ";
	cout << " |" << endl;

	cout << "+-";
	for(int i = 0; i < maxLength; i++)
		cout << "-";
	cout << "-+" << endl;

	for(set<string>::iterator it = databases.begin(); it != databases.end(); it++) {
		cout << "| " << *it;
		for(int i = 0; i < maxLength - it->length(); i++)
			cout << " ";
		cout << " |" << endl;
	}

	cout << "+-";
	for(int i = 0; i < maxLength; i++)
		cout << "-";
	cout << "-+" << endl;
}
void Database::LoadDatabases() {
#ifndef __linux__
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
			    	databases.insert(fileinfo.name);
			    }    
			}
		} while(_findnext(hFile, &fileinfo) == 0);  
		_findclose(hFile);
	}
#else
	if (access("Database", 0) == -1) {
		umask(0);
		mkdir("Database", 0777);
	}
   	
    databases.clear();

	struct dirent *fileinfo;
	DIR *dir = opendir("Database");
	if(dir == NULL)
		throw "Can't open Database base directory!";
	
	int iName=0;
	while((fileinfo = readdir(dir)) != NULL){
		if(fileinfo->d_type == DT_DIR){
			if( strcmp( fileinfo->d_name , "." ) != 0 &&
        		strcmp( fileinfo->d_name , "..") != 0){
				databases.insert(fileinfo->d_name);
			}
		}
	}
#endif
}
void Database::CreateDatabase(const char *databaseName) {
	if(strlen(databaseName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(databases.find(databaseName) != databases.end())
		throw "Database already exists!";
	databases.insert(databaseName);
#ifndef __linux__
	if (_access("Database", 0) == -1)
        _mkdir("Database");
    char dir[1000];
    sprintf(dir, "Database/%s", databaseName);
    if (_access(dir, 0) == -1)
        _mkdir(dir);
#else
	if(access("Database", 0) == -1) {
		umask(0);
		mkdir("Database", 0777);
	}
	char dir[1000];
	sprintf(dir, "Database/%s", databaseName);
	if(access(dir, 0) == -1) {
		umask(0);
		mkdir(dir, 0777);
	}
#endif
	cout << "Create database: " << databaseName << " succeed." << endl;
}
void Database::DropDatabase(const char *databaseName) {
	if(databases.find(databaseName) == databases.end())
		throw "Database not found!";
	if(currentDatabase != NULL && !strcmp(currentDatabase->databaseName, databaseName))
		CloseDatabase();
	databases.erase(databases.find(databaseName));
	char dir[1000];
	sprintf(dir, "Database/%s", databaseName);
	RemoveDirectory(dir);
	cout << "Drop database: " << databaseName << " succeed." << endl;
}
void Database::OpenDatabase(const char *databaseName) {
	if(databases.find(databaseName) == databases.end())
		throw "Database not found!";
	if(currentDatabase != NULL) {
		// 如果该数据库已经打开了则返回
		if(!strcmp(databaseName, currentDatabase->databaseName)) {
			cout << "Database changed to " << databaseName << "." << endl;
			return;
		}
		CloseDatabase();
	}
	// 创建数据库实例
	currentDatabase = new Database;
	memset(currentDatabase->databaseName, 0, sizeof currentDatabase->databaseName);
	strcpy(currentDatabase->databaseName, databaseName);
	// 为新创建的数据库实例读取 table 信息
	_finddata_t fileinfo;
	static char dir[1000];
	sprintf(dir, "Database/%s/*", databaseName);
	int hFile = _findfirst(dir, &fileinfo);
	if(hFile != -1)
	{
		do {
			if((fileinfo.attrib & _A_SUBDIR) == 0 && !strstr(fileinfo.name, "-"))  
			{
				currentDatabase->tables[fileinfo.name] = new Table(currentDatabase->databaseName, fileinfo.name);
			}
		} while(_findnext(hFile, &fileinfo) == 0);  
		_findclose(hFile);
	}
	cout << "Database changed to " << databaseName << "." << endl;
}
void Database::CloseDatabase() {
	if(currentDatabase == NULL)
		throw "No database selected!";
	for(map<string, Table*>::iterator it = currentDatabase->tables.begin(); it != currentDatabase->tables.end(); it++)
		delete it->second;
	delete currentDatabase;
	currentDatabase = NULL;
}
Table* Database::CreateTable(const char *tableName, FieldList fieldList) {
	if(currentDatabase == NULL)
		throw "No database selected!";
	if(strlen(tableName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(currentDatabase->tables.find(tableName) != currentDatabase->tables.end())
		throw "Table already exsists!";
	currentDatabase->tables[tableName] = new Table(currentDatabase->databaseName, tableName, fieldList);
	cout << "Table " << tableName << " successfully created." << endl;
	return currentDatabase->tables[tableName];
}
Table* Database::GetTable(const char *tableName) {
	if(currentDatabase == NULL)
		throw "No database selected!";
	if(strlen(tableName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(currentDatabase->tables.find(tableName) != currentDatabase->tables.end())
		throw "Table not found!";
	return currentDatabase->tables[tableName];
}

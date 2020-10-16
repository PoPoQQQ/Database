#pragma once
#include <map>
#include <string>
#include <cstring>
#include <direct.h>
#include "Table.h"
#include "../Utils/StringValidator.h"
using namespace std;

class Database {
public:
	static map<string, Database*> databases;
	char databaseName[MAX_STRING_LEN + 1];
	map<string, Table*> tables;

	Database(const char *databaseName) {
		StringValidator::Check(databaseName);
		memset(this->databaseName, 0, sizeof this->databaseName);
		strcpy(this->databaseName, databaseName);
		string s(databaseName);
		if(databases.find(s) != databases.end())
		{
			cerr << "Database is already exist!" << endl;
			exit(-1);
		}
		databases[s] = this;
		if (_access("Database", 0) == -1)
	        _mkdir("Database");
	    char dir[1000];
	    sprintf(dir, "Database/%s", databaseName);
	    if (_access(dir, 0) == -1)
	        _mkdir(dir);
	}

	Table* CreateTable(const char *tableName, TableHeader *tableHeader) {
		StringValidator::Check(tableName);
		tableHeader->setDatabaseName(databaseName);
		tableHeader->setTableName(tableName);
		Table *table = new Table(tableHeader);
	}
};

map<string, Database*> Database::databases;

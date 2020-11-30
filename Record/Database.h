#pragma once
#include <set>
#include <string>
#include <cstring>
#ifdef __linux__
#include <unistd.h>
#include <fcntl.h>
#else // default WIN64
#include <io.h>
#include <direct.h>
#endif //__linux__
#include "Table.h"
#include "../Utils/StringValidator.h"
using namespace std;

class Database {
public:
	static set<string> databases; // 全局的数据库名称信息（数据库不会嵌套或者重名）
	static Database *currentDatabase; // 当前打开的数据库
	char databaseName[MAX_STRING_LEN + 1];
	map<string, Table*> tables;
	/**
	 * 打印所有的数据库名称
	 */
	static void ShowDatabases() {
		for(set<string>::iterator it = databases.begin(); it != databases.end(); it++)
			cout << *it << endl;
	}
	/**
	 * 清空当前数据库储存的信息
	 * 从 Database 目录下重新读取目录作为全局的数据库信息
	 * 	*/
	static void LoadDatabases() {
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
				    	StringValidator::Check(fileinfo.name);
				    	databases.insert(fileinfo.name);
				    }    
				}
			} while(_findnext(hFile, &fileinfo) == 0);  
			_findclose(hFile);
		}

		//ShowDatabases();
#else
		if (access("Database", 0) == -1) {
			umask(0);
			mkdir("Database", 0777);
		}
	   	
	    databases.clear();

    	struct dirent *fileinfo;
    	DIR *dir = opendir("Database");
		if(dir == NULL){
			cerr << "Can't open Database base directory!" << endl;
			exit(1);
		}
		
    	int iName=0;
    	while((fileinfo = readdir(dir)) != NULL){
			if(fileinfo->d_type == DT_DIR){
				if( strcmp( fileinfo->d_name , "." ) != 0 &&
            		strcmp( fileinfo->d_name , "..") != 0){
					StringValidator::Check(fileinfo->d_name);
					databases.insert(fileinfo->d_name);
				}
			}
		}
#endif
	}
	/**
	 * 在 DataBase 目录下创建一个名为 databaseName 的目录作为一个数据库
	 * @param databaseName 要创建的数据库的名字
	 * */
	static void CreateDatabase(const char *databaseName) {
		StringValidator::Check(databaseName);
		if(databases.find(databaseName) != databases.end())
		{
			cerr << "Database already exists!" << endl;
			exit(-1);
		}
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
	}
	/**
	 * 根据提供的数据库名读取并生成数据库对象
	 * currentDatabase会指向这个最新的数据库对象
	 * @param databaseName 要打开的数据库名称
	 * */
	static void OpenDatabase(const char *databaseName) {
		StringValidator::Check(databaseName);
		if(databases.find(databaseName) == databases.end())
		{
			cerr << "Database not found!" << endl;
			exit(-1);
		}
		if(currentDatabase != NULL)
		{
			// 如果该数据库已经打开了则返回
			if(!strcmp(databaseName, currentDatabase->databaseName))
				return;
			CloseDatabase();
		}
		// 创建数据库实例
		currentDatabase = new Database;
		memset(currentDatabase->databaseName, 0, sizeof currentDatabase->databaseName);
		strcpy(currentDatabase->databaseName, databaseName);
		// 为新创建的数据库实例读取 table 信息
		// _finddata_t fileinfo;
		// static char dir[1000];
		// sprintf(dir, "Database/%s/*", databaseName);
		// int hFile = _findfirst(dir, &fileinfo);
		// if(hFile != -1)
		// {
		// 	do {
		// 		if((fileinfo.attrib & _A_SUBDIR) == 0)  
		// 		{
		// 			StringValidator::Check(fileinfo.name);
		// 			currentDatabase->tables[fileinfo.name] = new Table(currentDatabase->databaseName, fileinfo.name);
		// 		}
		// 	} while(_findnext(hFile, &fileinfo) == 0);  
		// 	_findclose(hFile);
		// }
	}
	/**
	 * 关闭当前打开的数据库
	 * 如果没有打开的数据库则报错并退出
	*/
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
	/**
	 * 在当前打开的数据库中创建一个新的表
	 * 如果当前没有打开数据库或是表已经存在则会报错退出
	 * @param tableName 表名
	 * @param fields 域列表
	 * */
	static Table *CreateTable(const char *tableName, vector<Field> fields) {
		if(currentDatabase == NULL) {
			cerr << "Current database does not exist!" << endl;
			exit(-1);
		}
		StringValidator::Check(tableName);
		if(currentDatabase->tables.find(tableName) != currentDatabase->tables.end()) {
			cerr << "Table already exsists!" << endl;
			exit(-1);
		}
		return currentDatabase->tables[tableName] = new Table(currentDatabase->databaseName, tableName, fields);
	}
	/**
	 * 从当前打开的数据库中获得某个 table 对象指针
	 * 如果当前数据库不存在或者表名不存在则会直接报错退出
	 * @param tableName 表名
	 * @return Table* 指向该表的指针
	 * */
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

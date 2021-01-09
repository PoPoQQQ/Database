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
#include <exception>
#include "Database.h"

int RemoveDirectory(const char* dir) {
#ifndef __linux__
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
	return 0;
#else
	// from https://www.cnblogs.com/StartoverX/p/4600866.html
	DIR* dirp = opendir(dir);    
    if(!dirp){
        return -1;
    }
    struct dirent *dir_entity;
    struct stat st;
    while((dir_entity = readdir(dirp)) != NULL){
        if(strcmp(dir_entity->d_name,".") == 0
        || strcmp(dir_entity->d_name,"..") == 0){
            continue;
        }    
		char sub_path[1000];
		sprintf(sub_path, "%s/", dir_entity->d_name);
        if(lstat(sub_path,&st) == -1){
            cerr << "rm_dir:lstat " << sub_path <<" error" << endl;
            continue;
        }    
        if(S_ISDIR(st.st_mode)){
            if(RemoveDirectory(sub_path) == -1) { // 如果是目录文件，递归删除
                closedir(dirp);
                return -1;
            }
            rmdir(sub_path);
        }
        else if(S_ISREG(st.st_mode)){
            unlink(sub_path);     // 如果是普通文件，则unlink
        }
        else{
            cerr << "rm_dir:st_mode " << sub_path << " error" << endl;
            continue;
        }
    }
    if(rmdir(dir) == -1)//delete dir itself.
    {
        closedir(dirp);
        return -1;
    }
    closedir(dirp);
    return 0;
#endif
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
	
	while((fileinfo = readdir(dir)) != NULL){
		if(fileinfo->d_type == DT_DIR){
			if( strcmp( fileinfo->d_name , "." ) != 0 &&
        		strcmp( fileinfo->d_name , "..") != 0){
				databases.insert(fileinfo->d_name);
			}
		}
	}
	closedir(dir);
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
#ifndef __linux__
	// 为新创建的数据库实例读取 table 信息
	_finddata_t fileinfo;
	static char dir[1000];
	sprintf(dir, "Database/%s/*", databaseName);
	int hFile = _findfirst(dir, &fileinfo);
	if(hFile != -1)
	{
		do {
			if((fileinfo.attrib & _A_SUBDIR) == 0)
			{
				if(!strstr(fileinfo.name, "-"))
				{
					currentDatabase->tables[fileinfo.name] = new Table(currentDatabase->databaseName, fileinfo.name);
				}
				else
				{
					static char ___dir[1000];
					strcpy(___dir, fileinfo.name);
					char* p = strstr(fileinfo.name, "-");
					*p = '\0';
					string tableName(___dir), indexName(p + 1);
					currentDatabase->indexes[indexName] = new Index(currentDatabase->databaseName, tableName, indexName);
				}
			}
		} while(_findnext(hFile, &fileinfo) == 0);  
		_findclose(hFile);
	}
#else
	struct dirent *fileinfo;

	char sub_path[1000];
	sprintf(sub_path, "Database/%s/", databaseName);
	DIR *dir = opendir(sub_path);
	if(dir == NULL)
		throw "Can't open Database base directory!";
	
	while((fileinfo = readdir(dir)) != NULL){
		if(fileinfo->d_type != DT_DIR){
			if(!strstr(fileinfo->d_name, "-")){
				currentDatabase->tables[fileinfo->d_name] = new Table(currentDatabase->databaseName, fileinfo->d_name);
			}
			else
				;//TODO
		}
	}
	closedir(dir);
#endif
	cout << "Database changed to " << databaseName << "." << endl;
}
void Database::CloseDatabase() {
	if(currentDatabase == NULL)
		throw "No database selected!";
	for(map<string, Table*>::iterator it = currentDatabase->tables.begin(); it != currentDatabase->tables.end(); it++)
		delete it->second;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		delete it->second;
	delete currentDatabase;
	currentDatabase = NULL;
}
void Database::ShowTables() {
	if(currentDatabase == NULL)
		throw "No database selected!";

	int maxLength = 10 + strlen(currentDatabase->databaseName);
	for(map<string, Table*>::iterator it = currentDatabase->tables.begin(); it != currentDatabase->tables.end(); it++)
		maxLength = max(maxLength, (signed)it->second->tableName.length());

	cout << "+-";
	for(int i = 0; i < maxLength; i++)
		cout << "-";
	cout << "-+" << endl;

	cout << "| Tables_in_" << currentDatabase->databaseName;
	for(int i = maxLength - 10 - strlen(currentDatabase->databaseName); i > 0; i--)
		cout << " ";
	cout << " |" << endl;

	cout << "+-";
	for(int i = 0; i < maxLength; i++)
		cout << "-";
	cout << "-+" << endl;

	for(map<string, Table*>::iterator it = currentDatabase->tables.begin(); it != currentDatabase->tables.end(); it++) {
		cout << "| " << it->second->tableName;
		for(int i = maxLength - it->second->tableName.length(); i > 0 ; i--)
			cout << " ";
		cout << " |" << endl;
	}

	cout << "+-";
	for(int i = 0; i < maxLength; i++)
		cout << "-";
	cout << "-+" << endl;
}
Table* Database::CreateTable(const string& tableName, const FieldList& fieldList) {
	if(currentDatabase == NULL)
		throw "No database selected!";
	if(tableName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(currentDatabase->tables.find(tableName) != currentDatabase->tables.end())
		throw "Table already exsists!";
	currentDatabase->tables[tableName] = new Table(currentDatabase->databaseName, tableName, fieldList);
	cout << "Table " << tableName << " successfully created." << endl;
	return currentDatabase->tables[tableName];
}
void Database::DropTable(const string& tableName) {
	Table* table = GetTable(tableName);
	delete table;
	currentDatabase->tables.erase(currentDatabase->tables.find(tableName));
	char dir[1000];
	sprintf(dir, "Database/%s/%s", currentDatabase->databaseName, tableName.c_str());
	remove(dir);
	vector<string> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->first);
	for(vector<string>::iterator it = idxes.begin(); it != idxes.end(); it++)
		DropIndex(tableName, *it);
	cout << "Drop table: " << tableName << " succeed." << endl;
}
void Database::RenameTable(const string& oldTbName, const string& newTbName) {
	// 处理二者相同的特殊情况
	if(oldTbName == newTbName){
		return;
	}
	// 调用 GetTable 获得 table, 出现错误会抛出异常
	// 如果有返回值的话说明 table 存在于 currentDatabase
	Table* table = Database::GetTable(oldTbName);

	// 检查 newTbName 是否合法
	if(newTbName.length() > MAX_IDENTIFIER_LEN)
		throw "Error(RenameTable): NewTbName is too long!";
	// 如果 newTbName 对应一个表则不合理
	if(Database::currentDatabase->tables.find(newTbName) != Database::currentDatabase->tables.end()){
		throw "Error(RenameTable): the given new table name has an existing table entry";
	}
	// 从当前的 map 中移除 entry
	Database::currentDatabase->tables.erase(Database::currentDatabase->tables.find(oldTbName));
	// 删除 Table
	delete table;
	// 将文件重命名
	string oldTablePath = string("Database/") + currentDatabase->databaseName + "/" + oldTbName;
	string newTablePath = string("Database/") + currentDatabase->databaseName + "/" + newTbName;
	if(Global::getInstance()->fm->moveFile(oldTablePath.c_str(), newTablePath.c_str())) {
		// 如果成功重命名

		// 重新创建 Table
		table = new Table(currentDatabase->databaseName, newTbName);
		// 将新的 Table 加入数据库
		Database::currentDatabase->tables[newTbName] = table;
	} else {
		cerr << "Warning: rename table failed, rename function returns a false" << endl;
		// 否则重新创建一个原始的 Table (原始文件依然存在)
		table = new Table(currentDatabase->databaseName, oldTbName);
		// 将旧的 Table 重新加回去
		Database::currentDatabase->tables[oldTbName] = table;
	}
	Index* index = nullptr;
	string indexName;
	// 对于找到所有索引中 TableName 相同的索引，和 Table 做类似的处理
	for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
		if(iter->second->tableName == oldTbName) {
			index = iter->second;
			indexName = index->indexName;
			oldTablePath = string("Database/") + currentDatabase->databaseName + "/" + oldTbName + "-" + indexName;
			newTablePath = string("Database/") + currentDatabase->databaseName + "/" + newTbName + "-" + indexName;
			// 因为 indexName 不变，所以不需要移除 entry，而是修改
			// 删除结构
			delete index;
			// 重命名文件
			if(Global::getInstance()->fm->moveFile(oldTablePath.c_str(), newTablePath.c_str())) {
				// 如果成功重命名

				// 重新创建
				index = new Index(currentDatabase->databaseName, newTbName, indexName);
				// 加入数据库
				Database::currentDatabase->indexes[indexName] = index;
			} else {
				cerr << "Warning: rename index failed, rename function returns a false" << endl;
				// 否则重新创建一个原始的 Index (原始文件依然存在)
				index = new Index(currentDatabase->databaseName, oldTbName, indexName);
				// 将旧的 Index 重新加回去
				Database::currentDatabase->indexes[indexName] = index;
			}
		}
	}
}

Table* Database::GetTable(const string &tableName) {
	if(currentDatabase == NULL)
		throw "No database selected!";
	if(tableName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(currentDatabase->tables.find(tableName) == currentDatabase->tables.end())
		throw "Table not found!";
	return currentDatabase->tables[tableName];
}
vector<unsigned int> Database::GetRecordList(string tableName, WhereCondition& whereCondition) {
	Table *table = GetTable(tableName);
	if(!whereCondition.validate(table))
		throw "whereClause Error";
	//Index!

	return table->GetRecordList(whereCondition);
}
void Database::Insert(string tableName, const vector<vector<Data>>& dataLists) {
	vector<Record> recordList;
	Table* table = GetTable(tableName);
	for(vector<vector<Data>>::const_iterator it = dataLists.begin(); it != dataLists.end(); it++) {
		Record record = table->EmptyRecord();
		vector<Data> dataList = *it;
		if(dataList.size() != record.fieldList.FieldCount())
			throw "Insert failed!";
		for(int i = 0; i < (signed)dataList.size(); i++)
			record.FillData(i, dataList[i]);
		record.NullCheck();
		recordList.push_back(record);
	}

	vector<Index*> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->second);
	table->AddRecords(recordList, idxes);
	cout << "Insertion succeeded!" << endl;
	table->PrintTable();
	for(vector<Index*>::iterator it = idxes.begin(); it != idxes.end(); it++)
		(*it)->Print();
}
void Database::Delete(string tableName, const vector<unsigned int>& recordList) {
	Table* table = GetTable(tableName);
	vector<Index*> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->second);
	table->DeleteRecords(recordList, idxes);
	cout << "Deletion succeeded!" << endl;
	table->PrintTable();
	for(vector<Index*>::iterator it = idxes.begin(); it != idxes.end(); it++)
		(*it)->Print();
}
void Database::Update(string tableName, const vector<unsigned int>& recordList, SetClauseObj& setClause) {
	Table* table = GetTable(tableName);
	vector<Index*> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->second);
	table->UpdateRecords(recordList, idxes, setClause);
	cout << "Updation succeeded!" << endl;
	table->PrintTable();
	for(vector<Index*>::iterator it = idxes.begin(); it != idxes.end(); it++)
		(*it)->Print();
}
void Database::CreateIndex(string tableName, string indexName, const vector<string>& columnList) {
	Table* table = GetTable(tableName);
	if(indexName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(currentDatabase->indexes.find(indexName) != currentDatabase->indexes.end())
		throw "Index already exsists!";
	vector<Data> keyTypes = table->GetKeyTypes(columnList);
	Index* index = currentDatabase->indexes[indexName] = new Index(currentDatabase->databaseName, tableName, indexName, columnList, keyTypes);
	table->InsertAllIntoIndex(index);
	index->Print();
}
void Database::DropIndex(string tableName, string indexName) {
	if(currentDatabase == NULL)
		throw "No database selected!";
	if(currentDatabase->indexes.find(indexName) == currentDatabase->indexes.end())
		throw "Index does not exist!";
	Index* index = currentDatabase->indexes[indexName];
	if(tableName == "")
		tableName = index->tableName;
	if(tableName != index->tableName)
		throw "Index does not exist!";
	delete index;
	currentDatabase->indexes.erase(currentDatabase->indexes.find(indexName));
	char dir[1000];
	sprintf(dir, "Database/%s/%s-%s", currentDatabase->databaseName, tableName.c_str(), indexName.c_str());
	remove(dir);
	cout << "Drop index: " << tableName << "-" << indexName << " succeed." << endl;
}

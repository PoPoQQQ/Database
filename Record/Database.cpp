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
#include "../Index/Query.h"
#include "./SelectFieldList.h"

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
					char* p = strstr(___dir, "-");
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
			else {
				
					static char ___dir[1000];
					strcpy(___dir, fileinfo->d_name);
					char* p = strstr(___dir, "-");
					*p = '\0';
					string tableName(___dir), indexName(p + 1);
					currentDatabase->indexes[indexName] = new Index(currentDatabase->databaseName, tableName, indexName);
			}
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
	if(fieldList.pkConstraints.size() > 0)
		CreateIndex(tableName, "-" + tableName, fieldList.pkConstraints[0].colNames);
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
	// 改变文件内部的名字
	table->changeName(newTbName);
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
		table->changeName(oldTbName);
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
			index->changeTableName(newTbName);
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
				index->changeTableName(oldTbName);
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
	Query query;
	Index* index;
	float maxCoverRate = 0.f;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName) {
			Query _query(it->second->colNames, it->second->keyTypes);
			_query.SetUpQuery(whereCondition);
			if(_query.coverRate > maxCoverRate) {
				query = _query;
				index = it->second;
				maxCoverRate = query.coverRate;
			}
		}
	if(maxCoverRate == 0.f)
		return table->GetRecordList(whereCondition);
	vector<unsigned int> gatherer;
	index->Search(query.LowerBound(), query.UpperBound(), gatherer);
	cout << "Index: " << index->indexName << " used!" << endl;
	return table->CheckRecordList(whereCondition, gatherer);
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
	if(table->fieldList.pkConstraints.size() > 0) {
		Index *index = currentDatabase->indexes["-" + tableName];
		if(!index)
			throw "Primary key index not created!";
		vector<unsigned int> gatherer;
		const vector<string>& colNames = index->colNames;
		for(vector<Record>::iterator it = recordList.begin(); it != recordList.end(); it++) {
			vector<Data> datas;
			for(int i = 0; i < (signed)colNames.size(); i++) {
				Data data = it->fieldList.GetColumn(it->fieldList.GetColumnIndex(colNames[i])).GetData();
				if((data.dataType & 0xff) != Data::VARCHAR)
					datas.push_back(data);
				else
					datas.push_back(HashData(data));
			}
			index->Search(datas, datas, gatherer);
			if(gatherer.size() > 0)
				throw "Primary key constraint violated!";
		}
	}
	for(vector<ForeignKeyCstrnt>::iterator _it = table->fieldList.fkConstraints.begin(); _it != table->fieldList.fkConstraints.end(); _it++) {
		Index *index = currentDatabase->indexes["-" + _it->tbName];
		if(!index)
			throw "Foreign key index not created!";
		vector<unsigned int> gatherer;
		const vector<string>& colNames = _it->colNames;
		for(vector<Record>::iterator it = recordList.begin(); it != recordList.end(); it++) {
			gatherer.clear();
			vector<Data> datas;
			for(int i = 0; i < (signed)colNames.size(); i++) {
				Data data = it->fieldList.GetColumn(it->fieldList.GetColumnIndex(colNames[i])).GetData();
				if((data.dataType & 0xff) != Data::VARCHAR)
					datas.push_back(data);
				else
					datas.push_back(HashData(data));
			}
			index->Search(datas, datas, gatherer);
			if(gatherer.size() == 0)
				throw "Foreign key constraint violated!";
			//else
			//	cout << gatherer.back() << endl;
		}
	}
	vector<Index*> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->second);
	table->AddRecords(recordList, idxes);
	cout << "Insertion succeeded!" << endl;
	// table->PrintTable();
	// for(vector<Index*>::iterator it = idxes.begin(); it != idxes.end(); it++)
	// 	(*it)->Print();
}
void Database::Delete(string tableName, const vector<unsigned int>& recordList) {
	Table* table = GetTable(tableName);
	vector<Index*> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->second);
	table->DeleteRecords(recordList, idxes);
	cout << "Deletion succeeded! " << recordList.size() << " rows affected." << endl;
	//table->PrintTable();
	//for(vector<Index*>::iterator it = idxes.begin(); it != idxes.end(); it++)
	//	(*it)->Print();
}
void Database::Update(string tableName, const vector<unsigned int>& recordList, SetClauseObj& setClause) {
	Table* table = GetTable(tableName);
	vector<Index*> idxes;
	for(map<string, Index*>::iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); it++)
		if(it->second->tableName == tableName)
			idxes.push_back(it->second);
	table->UpdateRecords(recordList, idxes, setClause);
	cout << "Updation succeeded!" << recordList.size() << " rows affected." << endl;
	//table->PrintTable();
	//for(vector<Index*>::iterator it = idxes.begin(); it != idxes.end(); it++)
	//	(*it)->Print();
}
void Database::Select(vector<ColObj>& selector, const vector<string>& tbList) {
	if (tbList.size() == 1) {
		// 查询单个 db
		Table* table = Database::GetTable(tbList[0].c_str());
		if(selector.size() == 0) {
			// 如果是 * , 则可以偷懒直接打印
			table->PrintTable();
		} else {
			// 验证每一个 col 是否合法
			for(int i = 0;i < selector.size(); ++i) {
				// 检查 selector
				if(!selector[i].isInTable(*table)) {
					char buf[256];
					snprintf(buf, 256, "Error: Selector (%s.%s) doesn't exist in table %s", selector[i].tbName.c_str(), selector[i].colName.c_str(), table->tableName.c_str());
					throw string(buf);
				}
			}

			FieldList tFieldList;
			for(int i = 0;i < selector.size(); ++i) {
				tFieldList.AddField(table->fieldList.GetColumn(table->fieldList.GetColumnIndex(selector[i].colName.c_str())));
			}
			tFieldList.PrintFields();
			function<void(Record&, BufType)> it = [&tFieldList](Record& record, BufType b) {
				unsigned int bitmap = 0;
				int index = -1;
				for(int i = 0;i < tFieldList.fields.size(); ++i) {
					index = record.fieldList.GetColumnIndex(tFieldList.fields[i].columnName);
					tFieldList.fields[i] = record.fieldList.GetColumn(index);
					bitmap |= (record.bitMap & (1u << index)) ? 1u << i : 0u;
				}
				tFieldList.PrintDatas(bitmap);
			};
			table->IterTable(it);
		}
	} else {
		// 笛卡尔积
		// 1. 检查 tableList 中的名称是否存在且唯一
		// 检查 tbList 中的内容是否存在且唯一
		map<string, Table*> tbMap;
		Table* tTable = nullptr;
		for(int i = 0;i < tbList.size(); ++i) {
			tTable = Database::GetTable(tbList[i].c_str());
			if(tbMap.find(tbList[i]) != tbMap.end()) {
				// 如果存在相同的表名则报错
				throw "Error: table name in selector should be unique";
			} else {
				tbMap[tbList[i]] = tTable;
			}
		}
		// 2. 根据 selector 进行不同的操作
		if(selector.size() == 0) {
			// 如果是 * ，则将所有的 FieldList 连接起来
			SelectFieldList tFieldList;
			for(int i = 0;i < tbList.size(); ++i) {
				FieldList& fieldList = tbMap.find(tbList[i])->second->fieldList;
				for(int j = 0; j < fieldList.fields.size(); ++j) {
					tFieldList.AddSelectField(ColObj(tbList[i], fieldList.fields[j].columnName), fieldList.fields[j]);
				}
			}
			int depth = 0;
			unsigned int bitmap = 0;
			unsigned int bitmapPos = 0;
			function<void(Record&, BufType)> it = [&tFieldList, &depth, &tbList, &tbMap, &bitmap, &bitmapPos, &it](Record& record, BufType b) {
				depth++;
				for(int i = 0;i < record.fieldList.fields.size(); ++i) {
					if(((record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u) == 0) {
						tFieldList.fields[bitmapPos].data = Data();
					} else {
						tFieldList.fields[bitmapPos].data = record.fieldList.fields[i].data;
					}
					bitmap |= (record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u;
					bitmapPos++;
				}
				if(depth == tbList.size()) {
					tFieldList.PrintDatas(bitmap);
				} else {
					tbMap.find(tbList[depth])->second->IterTable(it);
				}
				depth--;
				bitmap &= (0xffffffffu ^ ((1u << bitmapPos)-(1 << (bitmapPos - record.fieldList.fields.size()))));
				bitmapPos -= record.fieldList.fields.size();
			};
			// 进行递归打印笛卡尔积
			tFieldList.PrintFields();
			tbMap.find(tbList[0])->second->IterTable(it);
		} else {
			// 检查 selector 是否都在其中
			for(int i = 0;i < selector.size(); ++i) {
				if(!selector[i].isInTbMap(tbMap)) {
					char buf[128];
					snprintf(buf, 128, "selector (%s.%s) doesn't exist in table list", selector[i].tbName.c_str(), selector[i].colName.c_str());
					throw string(buf);
				}
			}

			SelectFieldList tFieldList;
			Table* tTable = nullptr;
			for(int i = 0;i < selector.size(); ++i) {
				tTable = Database::GetTable(selector[i].tbName.c_str());
				tFieldList.AddSelectField(selector[i], tTable->fieldList.GetColumn(tTable->fieldList.GetColumnIndex(selector[i].colName.c_str())));
			}

			int depth = 0;
			unsigned int bitmap = 0;
			unsigned int bitmapPos = 0;
			function<void(Record&, BufType)> it = [&tFieldList, &depth, &selector, &tbMap, &bitmap, &bitmapPos, &it](Record& record, BufType b) {
				// selector 对应的列（目前已经在表中）
				const int cIndex = record.fieldList.GetColumnIndex(selector[depth].colName.c_str());
				const Field& field = record.fieldList.GetColumn(cIndex);
				if(((record.bitMap & (1u << cIndex)) ? (1u << bitmapPos) : 0u) == 0) {
					// 如果没有数据则应该赋予一个 UNDEFINED 的数据
					tFieldList.fields[bitmapPos].data = Data();
				} else {
					tFieldList.fields[bitmapPos].data = field.data;
				}
				bitmap |= (record.bitMap & (1u << cIndex)) ? (1u << bitmapPos) : 0u;
				bitmapPos++;
				// 进入下一次递归
				depth++;
				if(depth == selector.size()) {
					tFieldList.PrintDatas(bitmap);
				} else {
					tbMap.find(selector[depth].tbName)->second->IterTable(it);
				}
				// 回退
				depth--;
				bitmapPos--;
				bitmap &= (0xffffffffu ^ (1 << bitmapPos));
			};
			
			// 进行递归打印笛卡尔积
			tFieldList.PrintFields();
			tbMap.find(tbList[0])->second->IterTable(it);
		}
	}
	cout << "SELECT finished!" << endl;
}
void Database::Select(vector<ColObj>& selector, const vector<string>& tbList, WhereCondition& where) {
	if (tbList.size() == 1) {
		// 查询单个 db
		Table* table = Database::GetTable(tbList[0].c_str());
		if(!where.validate(table)){
			throw "whereClause Error";
		}
		if(selector.size() == 0) {
			// 如果是 * ，则类似于 UPDATE
			// 根据 whereClause 中的条件进行搜索，如果满足条件则打印
			WhereCondition& whereClause = where;
			table->fieldList.PrintFields();
			function<void(Record&, BufType)> it = [&whereClause](Record& record, BufType b) {
				if((whereClause).check(record)) {
					record.fieldList.PrintDatas(record.bitMap);
				}
			};
			table->IterTable(it);
		} else {
			// 验证每一个 col 是否合法
			for(int i = 0;i < selector.size(); ++i) {
				// 检查 selector
				if(!selector[i].isInTable(*table)) {
					char buf[256];
					snprintf(buf, 256, "Error: Selector (%s.%s) doesn't exist in table %s", selector[i].tbName.c_str(), selector[i].colName.c_str(), table->tableName.c_str());
					throw string(buf);
				}
			}

			FieldList tFieldList;
			for(int i = 0;i < selector.size(); ++i) {
				tFieldList.AddField(table->fieldList.GetColumn(table->fieldList.GetColumnIndex(selector[i].colName.c_str())));
			}
			tFieldList.PrintFields();
			WhereCondition& whereClause = where;
			function<void(Record&, BufType)> it = [&tFieldList, &whereClause](Record& record, BufType b) {
				if(whereClause.check(record)) {
					unsigned int bitmap = 0;
					int index = -1;
					for(int i = 0;i < tFieldList.fields.size(); ++i) {
						index = record.fieldList.GetColumnIndex(tFieldList.fields[i].columnName);
						tFieldList.fields[i].data = record.fieldList.GetColumn(index).data;
						bitmap |= (record.bitMap & (1u << index)) ? 1u << i : 0u;
					}
					tFieldList.PrintDatas(bitmap);
				}
			};
			table->IterTable(it);
		}
	} else {
		// 笛卡尔积
		// 检查 tbList 中的内容是否存在且唯一
		map<string, Table*> tbMap;
		Table* tTable = nullptr;
		for(int i = 0;i < tbList.size(); ++i) {
			tTable = Database::GetTable(tbList[i].c_str());
			if(tbMap.find(tbList[i]) != tbMap.end()) {
				// 如果存在相同的表名则报错
				throw "Error: table name in selector should be unique";
			} else {
				tbMap[tbList[i]] = tTable;
			}
		}

		// 检查 selector 是否都在其中
		for(int i = 0;i < selector.size(); ++i) {
			if(!selector[i].isInTbMap(tbMap)) {
				char buf[128];
				snprintf(buf, 128, "selector (%s.%s) doesn't exist in table list", selector[i].tbName.c_str(), selector[i].colName.c_str());
				throw string(buf);
			}
		}

		// 检查 whereClause 是否正确
		where.validate(tbMap);

		if(selector.size() > 0) {
			// 如同 * 一样，将表格完全连接起来
			SelectFieldList tFieldList;
			SelectFieldList sFieldList;
			// 如同 * 一样完全连接表格的 fieldList
			for(int i = 0;i < tbList.size(); ++i) {
				FieldList& fieldList = tbMap.find(tbList[i])->second->fieldList;
				for(int j = 0; j < fieldList.fields.size(); ++j) {
					tFieldList.AddSelectField(ColObj(tbList[i], fieldList.fields[j].columnName), fieldList.fields[j]);
				}
			}
			// 只包含 selector 对应部分的 fieldList
			for(int i = 0;i < selector.size(); ++i) {
				sFieldList.AddSelectField(selector[i], tFieldList.GetColumn(tFieldList.GetColumnIndex(selector[i])));
			}
			int depth = 0;
			unsigned int bitmap = 0;
			unsigned int bitmapPos = 0;
			function<void(Record&, BufType)> it = [&tFieldList, &sFieldList, &depth, &tbList, &tbMap, &bitmap, &bitmapPos, &selector, &where, &it](Record& record, BufType b) {
				depth++;
				// 递归获得如同 * 的全部表格列的数据
				for(int i = 0;i < record.fieldList.fields.size(); ++i) {
					if(((record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u) == 0) {
						// 如果这一位的数据不存在，则要拷贝一个 UNDEFINED data
						// 否则不存在的数据实际上具有数据类型
						tFieldList.fields[bitmapPos].data = Data();
					} else {
						tFieldList.fields[bitmapPos].data = record.fieldList.fields[i].data;
					}
					bitmap |= (record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u;
					bitmapPos++;
				}
				if(depth == tbList.size()) {
					if(where.check(tFieldList)) {
						// 要打印的时候，根据 selector 从全部的列中取出一部分来打印
						for(int i = 0;i < selector.size(); ++i) {
							sFieldList.GetColumn(sFieldList.GetColumnIndex(selector[i])) = tFieldList.GetColumn(tFieldList.GetColumnIndex(selector[i]));
						}
						sFieldList.PrintDatas(bitmap);
					}
				} else {
					tbMap.find(tbList[depth])->second->IterTable(it);
				}
				depth--;
				bitmap &= (0xffffffffu ^ ((1u << bitmapPos)-(1 << (bitmapPos - record.fieldList.fields.size()))));
				bitmapPos -= record.fieldList.fields.size();
			};
			
			// 进行递归打印笛卡尔积
			sFieldList.PrintFields();
			tbMap.find(tbList[0])->second->IterTable(it);
		} else {
			// 如果是 * ，则将所有的 FieldList 连接起来
			SelectFieldList tFieldList;
			for(int i = 0;i < tbList.size(); ++i) {
				FieldList& fieldList = tbMap.find(tbList[i])->second->fieldList;
				for(int j = 0; j < fieldList.fields.size(); ++j) {
					tFieldList.AddSelectField(ColObj(tbList[i], fieldList.fields[j].columnName), fieldList.fields[j]);
				}
			}
			int depth = 0;
			unsigned int bitmap = 0;
			unsigned int bitmapPos = 0;
			function<void(Record&, BufType)> it = [&tFieldList, &depth, &tbList, &tbMap, &bitmap, &bitmapPos, &where, &it](Record& record, BufType b) {
				depth++;
				for(int i = 0;i < record.fieldList.fields.size(); ++i) {
					if(((record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u) == 0) {
						// 如果这一位的数据不存在，则要拷贝一个 UNDEFINED data
						// 否则不存在的数据实际上具有数据类型
						tFieldList.fields[bitmapPos].data = Data();
					} else {
						tFieldList.fields[bitmapPos].data = record.fieldList.fields[i].data;
					}
					bitmap |= (record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u;
					bitmapPos++;
				}
				if(depth == tbList.size()) {
					if(where.check(tFieldList)) {
						tFieldList.PrintDatas(bitmap);
					}
				} else {
					tbMap.find(tbList[depth])->second->IterTable(it);
				}
				depth--;
				bitmap &= (0xffffffffu ^ ((1u << bitmapPos)-(1 << (bitmapPos - record.fieldList.fields.size()))));
				bitmapPos -= record.fieldList.fields.size();
			};
			// 进行递归打印笛卡尔积
			tFieldList.PrintFields();
			tbMap.find(tbList[0])->second->IterTable(it);
		}
	}
	cout << "SELECT finished!" << endl;
}
void Database::CreateIndex(string tableName, string indexName, const vector<string>& columnList) {
	Table* table = GetTable(tableName);
	if(indexName[0] != '-' && indexName.length() > MAX_IDENTIFIER_LEN)
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

void Database::addTableField(const string& tbName, const FieldDesc& fieldDesc) {
	switch(fieldDesc.type) {
		case FieldDesc::FieldType::NOTNULL:
			cerr << "Error: Try to ADD a not null column without giving default" << endl;
			return;
		case FieldDesc::FieldType::PRIMARY:
			cerr << "Error: Cannot ADD PRIMARY KEY into table: " << tbName << endl;
			return;
		case FieldDesc::FieldType::FOREIGN:
			cerr << "Error: Cannot ADD FOREIGN KEY into table: " << tbName << endl;
			return;
		case FieldDesc::FieldType::UNDEFINED:
			cerr << "Error: undefined fieldDesc in Database::addTableField" << endl;
			return;
		default:
			break;
	}
	try {
		// 找到 Table，可能抛出异常
		Table *table = currentDatabase->GetTable(tbName);
		// 复制一个 FieldList
		FieldList newFieldList = table->fieldList;
		// 添加新的 Field
		// 如果添加中产生错误则这句话会抛出异常
		newFieldList.AddFieldDesc(fieldDesc);
		// 创建一个新的叫作 ~ 的临时表，利用全新的 FieldList
		Table *newTable = new Table(currentDatabase->databaseName, "~", newFieldList);
		// 遍历 table ，将每一个条目插入到新的 FieldList 中
		function<void(Record&, BufType)> it = [&newTable, &fieldDesc](Record& record, BufType b) {
			Record newRecord = newTable->EmptyRecord();
			// 使用以前的 Record 进行赋值
			for(int i = 0;i < record.fieldList.fields.size(); ++i) {
				// 如果数据位为 0 则表示 data 为 NULL
				if(record.bitMap & (1u << i) == 0) {
					newRecord.FillData(i, Data());
				} else {
					newRecord.FillData(i, record.fieldList.fields[i].data);
				}
			}
			// 对新的一列进行赋值
			switch(fieldDesc.type) {
				case FieldDesc::FieldType::DEFAULT:
				case FieldDesc::FieldType::NOTNULLWITHDEFAULT:
					newRecord.FillData(record.fieldList.fields.size(), fieldDesc.field.data);
					break;
				case FieldDesc::FieldType::NORMAL:
					newRecord.FillData(record.fieldList.fields.size(), Data());
					break;
				default:
					cerr << "Error: default branch in addTableField" << endl;
					return;
			}
			// 添加 Record
			unsigned int recordPosition = 0;
			newTable->AddRecord(newRecord, recordPosition);
		};
		table->IterTable(it);
		// 将之前的表格的相关 index 转移到新的表格上
		for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
			if(iter->second->tableName == tbName) {
				iter->second->tableName = "~";
			}
		}
		// 将新的表插入到 map 中
		currentDatabase->tables["~"] = newTable;
		// 删除之前的表格与对应关系
		currentDatabase->quietDropTable(tbName);
		// 将之前修改过的 index tablename 改回来
		// 因为实际上 index 的对应关系没有改变，所以可以不重新保存表中的数据
		for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
			if(iter->second->tableName == "~") {
				iter->second->tableName = tbName;
			}
		}
		// 重命名 ~ 表为原来的名字
		currentDatabase->RenameTable("~", tbName);
	}
	catch(const char* err) {
		cerr << err << endl;
	}
	catch(string err) {
		cerr << err << endl;
	}
	catch(exception& e) {
		cerr << e.what() << endl;
	}
}
void Database::dropTableField(const string& tbName, const string& colName) {
	try {
		// 找到 Table，可能抛出异常
		Table *table = currentDatabase->GetTable(tbName);
		//* 如果其对应某个主键则禁止删除，直接返回
		const FieldList& tFieldList = table->fieldList;
		if(tFieldList.pkConstraints.size() > 0) {
			for(vector<string>::const_iterator it = tFieldList.pkConstraints[0].colNames.begin(); it != tFieldList.pkConstraints[0].colNames.end(); ++it) {
				if (*it == colName) {
					cerr << "Error: cannot remove primary key column" << endl;
					return;
				}
			}
		}
		//* 如果其对应某个 Index 则提示并禁止删除
		for(map<string, Index*>::const_iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); ++it) {
			for(vector<string>::const_iterator iter = it->second->colNames.begin(); iter != it->second->colNames.end(); ++iter) {
				if (*iter == colName) {
					cerr << "Error: cannot remove column in an index" << endl;
					return;
				}
			}
		}
		// 复制一个 FieldList
		FieldList newFieldList = tFieldList;
		// 找到 colName 对应的列
		const int cIndex = tFieldList.GetColumnIndex(colName);
		// 删除对应列的 Field
		//! 这里面可能缺少某些项的更新从而导致 bug
		newFieldList.fields.erase(newFieldList.fields.begin() + cIndex);
		// 创建一个新的叫作 ~ 的临时表，利用全新的 FieldList
		Table *newTable = new Table(currentDatabase->databaseName, "~", newFieldList);
		// 遍历 table ，将每一个条目插入到新的 FieldList 中
		function<void(Record&, BufType)> it = [&newTable, cIndex](Record& record, BufType b) {
			Record newRecord = newTable->EmptyRecord();
			// 使用以前的 Record 进行赋值
			for(int i = 0;i < record.fieldList.fields.size(); ++i) {
				if(i == cIndex) continue;
				// 如果数据位为 0 则表示 data 为 NULL
				if(record.bitMap & (1u << i) == 0) {
					newRecord.FillData(i > cIndex ? i - 1 : i, Data());
				} else {
					newRecord.FillData(i > cIndex ? i - 1 : i, record.fieldList.fields[i].data);
				}
			}
			// 添加 Record
			unsigned int recordPosition = 0;
			newTable->AddRecord(newRecord, recordPosition);
		};
		table->IterTable(it);
		// 将之前的表格的相关 index 转移到新的表格上
		for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
			if(iter->second->tableName == tbName) {
				iter->second->tableName = "~";
			}
		}
		// 将新的表插入到 map 中
		currentDatabase->tables["~"] = newTable;
		// 删除之前的表格与对应关系
		currentDatabase->quietDropTable(tbName);
		// 将之前修改过的 index tablename 改回来
		// 因为实际上 index 的对应关系没有改变，所以可以不重新保存表中的数据
		for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
			if(iter->second->tableName == "~") {
				iter->second->tableName = tbName;
			}
		}
		// 重命名 ~ 表为原来的名字
		currentDatabase->RenameTable("~", tbName);
	}
	catch(const char* err) {
		cerr << err << endl;
	}
	catch(string err) {
		cerr << err << endl;
	}
	catch(exception& e) {
		cerr << e.what() << endl;
	}
}
void Database::changeTableField(const string& tbName, const string& colName, const FieldDesc& fieldDesc) {
	switch(fieldDesc.type) {
		case FieldDesc::FieldType::NOTNULL:
			cerr << "Error: Try to CHANGE a not null column without giving default" << endl;
			return;
		case FieldDesc::FieldType::PRIMARY:
			cerr << "Error: Cannot CHANGE a column into PRIMARY KEY" << endl;
			return;
		case FieldDesc::FieldType::FOREIGN:
			cerr << "Error: Cannot CHANGE a column into FOREIGN KEY" << endl;
			return;
		case FieldDesc::FieldType::UNDEFINED:
			cerr << "Error: undefined fieldDesc in Database::changeTableField" << endl;
			return;
		default:
			break;
	}
	try {
		// 找到 Table，可能抛出异常
		Table *table = currentDatabase->GetTable(tbName);
		//* 如果其对应某个主键则禁止删除，直接返回
		const FieldList& tFieldList = table->fieldList;
		if(tFieldList.pkConstraints.size() > 0) {
			for(vector<string>::const_iterator it = tFieldList.pkConstraints[0].colNames.begin(); it != tFieldList.pkConstraints[0].colNames.end(); ++it) {
				if (*it == colName) {
					cerr << "Error: cannot remove primary key column" << endl;
					return;
				}
			}
		}
		//* 如果其对应某个 Index 则提示并禁止删除
		for(map<string, Index*>::const_iterator it = currentDatabase->indexes.begin(); it != currentDatabase->indexes.end(); ++it) {
			for(vector<string>::const_iterator iter = it->second->colNames.begin(); iter != it->second->colNames.end(); ++iter) {
				if (*iter == colName) {
					cerr << "Error: cannot remove column in an index" << endl;
					return;
				}
			}
		}
		// 复制一个 FieldList
		FieldList newFieldList = table->fieldList;
		// 找到 colName 对应的列
		const int cIndex = tFieldList.GetColumnIndex(colName);
		// 删除对应列的 Field
		//! 这里面可能缺少某些项的更新从而导致 bug
		newFieldList.fields.erase(newFieldList.fields.begin() + cIndex);
		// 添加新的 Field
		newFieldList.fields.insert(newFieldList.fields.begin() + cIndex, fieldDesc.field);
		// 创建一个新的叫作 ~ 的临时表，利用全新的 FieldList
		Table *newTable = new Table(currentDatabase->databaseName, "~", newFieldList);
		// 遍历 table ，将每一个条目插入到新的 FieldList 中
		function<void(Record&, BufType)> it = [&newTable, &fieldDesc, cIndex](Record& record, BufType b) {
			Record newRecord = newTable->EmptyRecord();
			// 使用以前的 Record 进行赋值
			for(int i = 0;i < record.fieldList.fields.size(); ++i) {
				if (i == cIndex) continue;
				// 如果数据位为 0 则表示 data 为 NULL
				if(record.bitMap & (1u << i) == 0) {
					newRecord.FillData(i, Data());
				} else {
					newRecord.FillData(i, record.fieldList.fields[i].data);
				}
			}
			// 对新的一列进行赋值
			switch(fieldDesc.type) {
				case FieldDesc::FieldType::DEFAULT:
				case FieldDesc::FieldType::NOTNULLWITHDEFAULT:
					newRecord.FillData(cIndex, fieldDesc.field.data);
					break;
				case FieldDesc::FieldType::NORMAL:
					newRecord.FillData(cIndex, Data());
					break;
				default:
					cerr << "Error: default branch in addTableField" << endl;
					return;
			}
			// 添加 Record
			unsigned int recordPosition = 0;
			newTable->AddRecord(newRecord, recordPosition);
		};
		table->IterTable(it);
		// 将之前的表格的相关 index 转移到新的表格上
		for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
			if(iter->second->tableName == tbName) {
				iter->second->tableName = "~";
			}
		}
		// 将新的表插入到 map 中
		currentDatabase->tables["~"] = newTable;
		// 删除之前的表格与对应关系
		currentDatabase->quietDropTable(tbName);
		// 将之前修改过的 index tablename 改回来
		// 因为实际上 index 的对应关系没有改变，所以可以不重新保存表中的数据
		for(map<string, Index*>::iterator iter = currentDatabase->indexes.begin(); iter != currentDatabase->indexes.end(); ++iter) {
			if(iter->second->tableName == "~") {
				iter->second->tableName = tbName;
			}
		}
		// 重命名 ~ 表为原来的名字
		currentDatabase->RenameTable("~", tbName);
	}
	catch(const char* err) {
		cerr << err << endl;
	}
	catch(string err) {
		cerr << err << endl;
	}
	catch(exception& e) {
		cerr << e.what() << endl;
	}
}
void Database::AddPrimaryKey(string tableName, string pkName, const vector<string>& columnList) {
	Table* table = GetTable(tableName);
	if(table->fieldList.pkConstraints.size() > 0)
		throw "Primary key already exists!";
	for(int i = 0; i < (signed)columnList.size(); i++) {
		string colName = columnList[i];
		int colIndex = table->fieldList.GetColumnIndex(colName);
		if(colIndex == -1)
			throw "Error: Column does not exist";
		Field& field = table->fieldList.GetColumn(colIndex);
		if((field.constraints & Field::PRIMARY_KEY))
			throw "Error: Multiple constraint is not supported!";
	}
	CreateIndex(tableName, "-" + tableName, columnList);
	table->fieldList.AddPrimaryKey(pkName, columnList);
}
void Database::DropPrimaryKey(string tableName, string pkName) {
	Table* table = GetTable(tableName);
	if(table->fieldList.pkConstraints.size() == 0)
		throw "Primary key does not exist!";
	if(pkName == "")
		pkName = table->fieldList.pkConstraints[0].pkName;
	if(pkName != table->fieldList.pkConstraints[0].pkName)
		throw "Primary key does not exist!";
	for(map<string, Table*>::iterator it = currentDatabase->tables.begin(); it != currentDatabase->tables.end(); it++)
		for(int i = 0; i < (signed)it->second->fieldList.fkConstraints.size(); i++)
			if(it->second->fieldList.fkConstraints[i].tbName == tableName)
				throw "Dropping failed!";
	DropIndex(tableName, "-" + tableName);
	table->fieldList.DropPrimaryKey();
}
void Database::AddForeignKey(string tableName, string fkName, const vector<string>& columnList, 
	string refTableName, const vector<string>& refColumnList) {
	Table* table = GetTable(tableName);
	for(int i = 0; i < (signed)columnList.size(); i++) {
		string colName = columnList[i];
		int colIndex = table->fieldList.GetColumnIndex(colName);
		if(colIndex == -1)
			throw "Error: Column does not exist";
		Field& field = table->fieldList.GetColumn(colIndex);
		if((field.constraints & Field::FOREIGN_KEY))
			throw "Error: Multiple constraint is not supported!";
	}
	table->fieldList.AddForeignKey(fkName, refTableName, columnList, refColumnList);
}
void Database::DropForeignKey(string tableName, string fkName) {
	Table* table = GetTable(tableName);
	int i;
	for(i = 0; i < (signed)table->fieldList.fkConstraints.size(); i++)
		if(table->fieldList.fkConstraints[i].fkName == fkName)
			break;
	if(i == (signed)table->fieldList.fkConstraints.size())
		throw "Foreign key does not exist!";
	table->fieldList.DropForeignKey(i);
}
void Database::quietDropTable(string tableName) {
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
}

void Database::quietDropIndex(string tableName,string indexName) {
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
}
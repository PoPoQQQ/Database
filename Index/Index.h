#pragma once
#include <string>
#include <vector>
#include "../Record/Data.h"
#include "../Pages/FileBase.h"
#include "../Pages/PageBase.h"
#include "../Utils/MyBitMap.h"
#include "../Utils/Constraints.h"
using namespace std;
/*
管理一个索引的信息
*/
class Index: public FileBase {
public:
	string databaseName;
	string tableName;
	string indexName;
	int rootPage;
	vector<Data> keys;

	Index(string databaseName, string tableName, string indexName);
	Index(string databaseName, string tableName, string indexName, vector<Data> keys);

	void Insert(vector<Data> keys, int value);
	void Remove(vector<Data> keys);
	void Print();
private:
	void LoadHeader();
	void SaveHeader() const;
};

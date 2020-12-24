#pragma once
#include <string>
#include <vector>
#include "../Record/Data.h"
#include "../Pages/FileBase.h"
#include "../Utils/MyBitMap.h"
#include "../Record/PageBase.h"
#include "../Utils/Constraints.h"
using namespace std;
/*
管理一个索引的信息
*/
class Index: FileBase {
public:
	string databaseName;
	string tableName;
	string indexName;
	int rootPage;
	vector<Data> keys;
	MyBitMap *bitMap;

	Index(string databaseName, string tableName, string indexName);
	Index(string databaseName, string tableName, string indexName, vector<Data> keys);
	~Index();

	void LoadHeader();
	void SaveHeader();

	PageBase* LoadPage(int pageNum);
	PageBase* CreatePage(int pageType);
	void FreePage(int pageNum);

	void Insert(vector<Data> keys, int value);
	void Remove(vector<Data> keys);
	void Print();
};

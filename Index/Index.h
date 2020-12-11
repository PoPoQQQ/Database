#pragma once
#include <vector>
#include "../Record/Data.h"
#include "../Utils/MyBitMap.h"
#include "../Record/PageBase.h"
#include "../Utils/Constraints.h"
using namespace std;
/*
管理一个索引的信息
*/
class Index {
public:
	int fileID;
	char databaseName[MAX_STRING_LEN + 1];
	char tableName[MAX_STRING_LEN + 1];
	char indexName[MAX_STRING_LEN + 1];
	int numberOfPage;
	int rootPage;
	vector<Data> keys;
	MyBitMap *bitMap;

	Index(const char *databaseName, const char *tableName, const char *indexName);
	Index(const char *databaseName, const char *tableName, const char *indexName, vector<Data> keys);
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

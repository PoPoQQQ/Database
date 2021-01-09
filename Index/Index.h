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
	vector<string> colNames;
	vector<Data> keyTypes;

	Index(string databaseName, string tableName, string indexName);
	Index(string databaseName, string tableName, string indexName, 
		const vector<string> colNames, const vector<Data>& keyTypes);

	void Insert(vector<Data> keys, unsigned int value);
	void Remove(vector<Data> keys, unsigned int value);
	void Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer);
	void Print();
	/** 
	 * 直接改变所指向的 Table 的名称
	 * 不进行任何的检测
	 * */
	void changeTableName(const string&);
private:
	void LoadHeader();
	void SaveHeader() const;
};

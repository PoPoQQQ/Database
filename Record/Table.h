#pragma once
#include <string>
#include <vector>
#include "Record.h"
#include "FieldList.h"
#include "../Pages/FileBase.h"
#include "../Utils/MyBitMap.h"
using namespace std;
/*
管理一个表的信息
*/
class Table: FileBase {
public:
	string databaseName;
	string tableName;
	unsigned long long ridTimestamp;
	int recordCount;
	FieldList fieldList;
	MyBitMap *bitMap;

	Table(string databaseName, string tableName);
	Table(string databaseName, string tableName, FieldList fieldList);
	~Table();

	void LoadHeader();
	void SaveHeader();
	Record EmptyRecord();
	void AddRecord(Record record);
	void PrintTable();
};

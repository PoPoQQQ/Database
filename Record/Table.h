#pragma once
#include <vector>
#include "Record.h"
#include "FieldList.h"
#include "../Utils/MyBitMap.h"
using namespace std;
/*
管理一个表的信息
*/
class Table {
public:
	int fileID;
	char databaseName[MAX_IDENTIFIER_LEN + 1];
	char tableName[MAX_IDENTIFIER_LEN + 1];
	int numberOfPage;
	unsigned long long ridTimestamp;
	int recordCount;
	FieldList fieldList;
	MyBitMap *bitMap;

	Table(const char *databaseName, const char *tableName);
	Table(const char *databaseName, const char *tableName, FieldList fieldList);
	~Table();

	void LoadHeader();
	void SaveHeader();
	Record *CreateEmptyRecord();
	void AddRecord(Record *record);
	void PrintTable();
};

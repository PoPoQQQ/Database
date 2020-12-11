#pragma once
#include <vector>
#include "Field.h"
#include "Record.h"
#include "../Utils/MyBitMap.h"
using namespace std;
/*
管理一个表的信息
*/
class Table {
public:
	int fileID;
	char databaseName[MAX_STRING_LEN + 1];
	char tableName[MAX_STRING_LEN + 1];
	int numberOfPage;
	unsigned long long ridTimestamp;
	int recordCount;
	vector<Field> fields;
	MyBitMap *bitMap;

	Table(const char *databaseName, const char *tableName);
	Table(const char *databaseName, const char *tableName, vector<Field> fields);
	~Table();

	void LoadHeader();
	void SaveHeader();
	Record *CreateEmptyRecord();
	void AddRecord(Record *record);
	void PrintTable();
};

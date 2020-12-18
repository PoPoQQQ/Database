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

	Table(const char *databaseName, const char *tableName); // 仅用于读取表格。从给定的数据库和表中读取信息到内存中
	Table(const char *databaseName, const char *tableName, FieldList fieldList); // 创建表格数据结构并进行创建对应的文件进行保存
	~Table();

	Record EmptyRecord();
	void AddRecord(Record record);
	void PrintTable();

private:
	void LoadHeader();
	void SaveHeader() const;
};

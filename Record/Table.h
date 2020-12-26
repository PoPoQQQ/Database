#pragma once
#include <string>
#include <vector>
#include "Record.h"
#include "FieldList.h"
#include "../Pages/FileBase.h"
using namespace std;
/*
管理一个表的信息
*/
class Table: public FileBase {
public:
	string databaseName;
	string tableName;
	unsigned long long ridTimestamp;
	int recordCount;
	FieldList fieldList;

	// 仅用于读取表格。从给定的数据库和表中读取信息到内存中
	Table(string databaseName, string tableName);
	// 创建表格数据结构并进行创建对应的文件进行保存
	Table(string databaseName, string tableName, FieldList fieldList);

	Record EmptyRecord();
	void AddRecord(Record record);
	void PrintTable();
	void IterTable();
	// 对应 mysql 的 desc table 命令
	// 将表头信息以及约束按照一定的规则进行打印
	void DescTable() const;
private:
	void LoadHeader();
	void SaveHeader() const;
};

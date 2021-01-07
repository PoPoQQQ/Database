#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Record.h"
#include "FieldList.h"
#include "../Index/Index.h"
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
	vector<int> GetColumnIndexes(const vector<string>& columnList);
	vector<Data> GetKeyTypes(const vector<string>& columnList);
	void InsertAllIntoIndex(Index* index);
	void InsertRecordIntoIndex(Index* index, const vector<int>& columnIndexes, 
		Record record, unsigned int recordPosition);
	void AddRecord(Record& record, unsigned int& recordPosition);
	void AddRecords(vector<Record>& records, const vector<Index*>& idxes);
	void PrintTable();
	// 根据给定的函数对表中的每一个 Record 进行迭代
	// 类似于 forEach 函数
	void IterTable(function<void(Record&, BufType)> iterFunc);
	// 对应 mysql 的 desc table 命令
	// 将表头信息以及约束按照一定的规则进行打印
	void DescTable() const;
private:
	void LoadHeader();
	void SaveHeader() const;
};

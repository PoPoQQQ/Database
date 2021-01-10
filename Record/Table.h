#pragma once
#include <string>
#include <vector>
#include <functional>
#include "Record.h"
#include "FieldList.h"
#include "../Index/Index.h"
#include "../Pages/FileBase.h"
#include "../Parser/SetClauseObj.h"
#include "../Index/WhereCondition.h"
using namespace std;
/*
管理一个表的信息
*/
class Table: public FileBase {
public:
	string databaseName;
	string tableName;
	unsigned long long ridTimestamp = 0;
	int recordCount = 0;
	FieldList fieldList;

	// 仅用于读取表格。从给定的数据库和表中读取信息到内存中
	Table(string databaseName, string tableName);
	// 创建表格数据结构并进行创建对应的文件进行保存
	Table(string databaseName, string tableName, const FieldList& fieldList);
	// 在 Table 析构的时候一般都说明这个 Table 相关的文件需要保存
	// 故进行 Header 缓存的写入，其余部分在 FileBase 中保存
	~Table(){
		SaveHeader(); 
	}

	Record EmptyRecord();
	vector<int> GetColumnIndexes(const vector<string>& columnList);
	vector<Data> GetKeyTypes(const vector<string>& columnList);
	void InsertAllIntoIndex(Index* index);
	void InsertRecordIntoIndex(Index* index, Record record, unsigned int recordPosition);
	void RemoveRecordFromIndex(Index* index, Record record, unsigned int recordPosition);
	void AddRecord(Record& record, unsigned int& recordPosition);
	void AddRecords(vector<Record>& records, const vector<Index*>& idxes);
	void DeleteRecord(Record& record, unsigned int recordPosition);
	void DeleteRecords(const vector<unsigned int>& records, const vector<Index*>& idxes);
	bool UpdateRecords(const vector<unsigned int>& records, const vector<Index*>& idxes, SetClauseObj& setClause);
	bool CheckPrimaryKey(Index* index);
	bool CheckForeignKey(const vector<string>& columnList, Index *index);
	vector<unsigned int> GetRecordList(WhereCondition& whereCondition);
	vector<unsigned int> CheckRecordList(WhereCondition& whereCondition, const vector<unsigned int>& gatherer);
	void PrintTable();
	// 根据给定的函数对表中的每一个 Record 进行迭代
	// 类似于 forEach 函数
	void IterTable(function<void(Record&, BufType)> iterFunc);
	// 对应 mysql 的 desc table 命令
	// 将表头信息以及约束按照一定的规则进行打印
	void DescTable() const;
	/** 
	 * 修改自身结构的 table name 并且将其写到文件中
	 * */
	void changeName(const string&);
private:
	void LoadHeader();
	void SaveHeader() const;
};

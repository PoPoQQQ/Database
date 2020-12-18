#pragma once
#include <vector>
#include "Field.h"
#include "../BufManager/BufPageManager.h"
// #include "FieldDesc.h"
using namespace std;
/*
实现了一个字段的定义
*/
class FieldList {
public:
	vector<Field> fields;
	// vector<string> pkList; // 用于储存所有主键列名的 vector（主键只有一个，长度代表是否有主键和主键是否是联合的）

	void LoadFields(BufType b);
	void SaveFields(BufType b) const;
	void LoadDatas(unsigned char* b);
	// 将 FieldList 中储存的所有数据保存到 buffer 中（按 Byte 进行储存）
	void SaveDatas(unsigned char* b) const;

	void AddField(const Field& field);
	void PrintFields();
	void PrintDatas(unsigned int bitMap);

	int FieldCount() const;
	int RoundedDataSize();
	Field& GetColumn(int index);
	int GetColumnIndex(const char* columnName);

};

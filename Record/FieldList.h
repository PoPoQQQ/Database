#pragma once
#include <vector>
#include "Field.h"
#include "../BufManager/BufPageManager.h"
#include "./PrimaryKeyCstrnt.h"
#include "FieldDesc.h"
using namespace std;
/*
实现了一个字段的定义
*/
class FieldList {
public:
	vector<Field> fields;
	PrimaryKeyCstrnt pkConstraint;

	void LoadFields(BufType b);
	void SaveFields(BufType b) const;
	void LoadDatas(unsigned char* b);
	// 将 FieldList 中储存的所有数据保存到 buffer 中（按 Byte 进行储存）
	void SaveDatas(unsigned char* b) const;

	void AddField(const Field& field);
	void AddFieldDescVec(const vector<FieldDesc>& field_desc_vec);
	void PrintFields();
	void PrintDatas(unsigned int bitMap);

	int FieldCount() const;
	int RoundedDataSize() const;
	Field& GetColumn(int index);
	int GetColumnIndex(const char* columnName);

	FieldList(){};
	FieldList(const FieldList&);
};

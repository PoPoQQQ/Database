#pragma once
#include <vector>
#include "Field.h"
#include "../BufManager/BufPageManager.h"
#include "./PrimaryKeyCstrnt.h"
#include "./ForeignKeyCstrnt.h"
#include "FieldDesc.h"
using namespace std;
/*
实现了一个字段的定义
*/
class FieldList {
public:
	vector<Field> fields;
	vector<PrimaryKeyCstrnt> pkConstraints;
	vector<ForeignKeyCstrnt> fkConstraints;

	void LoadFields(BufType b);
	void SaveFields(BufType b) const;
	void LoadDatas(unsigned char* b);
	// 将 FieldList 中储存的所有数据保存到 buffer 中（按 Byte 进行储存）
	void SaveDatas(unsigned char* b) const;

	void AddField(const Field& field);
	void AddFieldDescVec(const char* tbName, const vector<FieldDesc>& field_desc_vec);
	void PrintFields();
	void DescFields() const;
	void PrintDatas(unsigned int bitMap);

	int FieldCount() const;
	int RoundedDataSize() const;
	Field& GetColumn(int index);
	const Field& GetColumn(int index) const;
	int GetColumnIndex(const char* columnName) const;

	FieldList(){};
	FieldList(const FieldList&);
	~FieldList();
};

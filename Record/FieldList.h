#pragma once
#include <string>
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
	/** 
	 * 通过 FieldDesc 添加一个 Field
	 * 添加过程中忽略添加约束的操作
	 * 目前只能用于 ALTER ADD 命令
	 * */
	void AddFieldDesc(const FieldDesc& fieldDesc);
	/**
	 * 通过 FieldDesc vec 来构建 FieldList
	 * 仅能用于 parser 的读取操作，一般只能调用一次
	 * 在解析完毕所有的 field 后会将 pk 和 fk 约束加在 field 上
	 * */
	void AddFieldDescVec(string tbName, const vector<FieldDesc>& vec);
	void PrintFields();
	void DescFields() const;
	void PrintDatas(unsigned int bitMap);

	int FieldCount() const;
	int RoundedDataSize() const;
	Field& GetColumn(int index);
	const Field& GetColumn(int index) const;
	int GetColumnIndex(string columnName) const;

	FieldList(){};
	FieldList(const FieldList&);
	~FieldList();
};

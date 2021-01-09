#pragma once
#include <cstdio>
#include <cstring>
#include "Data.h"
#include "../Utils/Constraints.h"
#include "../BufManager/BufPageManager.h"
/*
实现了一个字段的定义
*/
class Field {
public:
	enum {
		NOT_NULL = 1,
		DEFAULT = 2,
		PRIMARY_KEY = 4,
		FOREIGN_KEY = 8
	};
	string columnName;
	Data data;
	int constraints = 0;

	Field();
	Field(const string& columnName);
	Field(const Field& other);

	Field& operator = (const Field& field);

	Data GetData();
	void SetData(const Data& data);
	Field SetDataType(Data data);
	Field SetNotNull();
	Field SetDefault(Data data);
	Field SetPrimaryKey();
	Field SetForeignKey();

	int DataSize() const;
	int RoundedDataSize() const;
	int FieldSize() const;
	void Load(BufType b);
	void Save(BufType b) const;
	void LoadData(unsigned char* b);
	void SaveData(unsigned char* b) const;

	//string toString() const;
	// 检查传入的 Data 结构是否符合当前 Field 的数据要求
	// 如果不满足要求则会直接抛出错误
	//* @throws string 错误原因
	bool validateData(const Data& data) const;
};

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
	char columnName[MAX_IDENTIFIER_LEN + 1];
	Data data;

	int constraints;
	char primaryKeyName[MAX_IDENTIFIER_LEN + 1];
	char foreignKeyTable[MAX_IDENTIFIER_LEN + 1];
	char foreignKeyColumn[MAX_IDENTIFIER_LEN + 1];
	char foreignKeyName[MAX_IDENTIFIER_LEN + 1];

	Field();
	Field(const char *columnName);

	Data GetData();
	void SetData(const Data& data);
	Field SetDataType(Data data);
	Field SetNotNull();
	Field SetDefault(Data data);
	Field SetPrimaryKey(const char* primaryKeyName = "");
	Field SetForeignKey(const char* foreignKeyTable, const char* foreignKeyColumn, const char* foreignKeyName = "");

	int DataSize() const;
	int RoundedDataSize() const;
	int FieldSize() const;
	void Load(BufType b);
	void Save(BufType b) const;
	void LoadData(unsigned char* b);
	void SaveData(unsigned char* b) const;

	string toString() const;
	// 检查传入的 Data 结构是否符合当前 Field 的数据要求
	// 如果不满足要求则会直接抛出错误
	//* @throws string 错误原因
	bool validateData(const Data& data) const;
	// Field(const Field& other): data(other.data) {
	// 	memset(this->fieldName, 0, sizeof this->fieldName);
	// 	strcpy(this->fieldName, other.fieldName);
	// }
	// Field& operator = (const Field& field) {
	// 	this->data = field.data;
	// 	memset(this->fieldName, 0, sizeof this->fieldName);
	// 	strcpy(this->fieldName, field.fieldName);
	// 	return *this;
 	// }
};

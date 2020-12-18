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
	void SetData(Data data);
	Field SetDataType(Data data);
	Field SetNotNull();
	Field SetDefault(Data data);
	Field SetPrimaryKey(const char* primaryKeyName = "");
	Field SetForeignKey(const char* foreignKeyTable, const char* foreignKeyColumn, const char* foreignKeyName = "");

	int DataSize();
	int RoundedDataSize();
	int FieldSize();
	void Load(BufType b);
	void Save(BufType b);
	void LoadData(unsigned char* b);
	void SaveData(unsigned char* b);

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

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
	char columnName[MAX_IDENTIFIER_LEN + 1];
	//1-NOT NULL 2-DEFAULT 4-PRIMARY KEY 8-FOREIGN KEY
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
};

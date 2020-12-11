#pragma once
#include <cstdio>
#include <cstring>
#include "Data.h"
#include "../Utils/Constraints.h"
#include "../Utils/StringValidator.h"
#include "../BufManager/BufPageManager.h"
#define FIELD_SIZE (MAX_STRING_LEN + 8)
using namespace std;
/*
实现了一个字段的定义
*/
class Field {
public:
	char fieldName[MAX_STRING_LEN + 1];
	Data data;
	Field() {
		fieldName[0] = '\0';
	}
	Field(const char *fieldName, Data data): data(data) {
		StringValidator::Check(fieldName);
		memset(this->fieldName, 0, sizeof this->fieldName);
		strcpy(this->fieldName, fieldName);
	}
	void SetDataType(Data data) {
		this->data = data;
	}
	void Load(BufType b) {
		memcpy(fieldName, b, MAX_STRING_LEN);
		fieldName[MAX_STRING_LEN] = 0;
		data.LoadType(b + (MAX_STRING_LEN >> 2));
	}
	void Save(BufType b) {
		memcpy(b, fieldName, MAX_STRING_LEN);
		data.SaveType(b + (MAX_STRING_LEN >> 2));
	}
};

#pragma once
#include <cstdio>
#include <cstring>
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
	int fieldType, fieldSize;
	Field() {
		fieldName[0] = '\0';
		fieldType = -1;
		fieldSize = -1;
	}
	Field(const char *fieldName, int fieldType, int fieldSize) {
		StringValidator::Check(fieldName);
		memset(this->fieldName, 0, sizeof this->fieldName);
		strcpy(this->fieldName, fieldName);
		this->fieldType = fieldType;
		this->fieldSize = fieldSize;
	}
	void Load(BufType b) {
		memset(fieldName, 0, sizeof fieldName);
		memcpy(fieldName, b, MAX_STRING_LEN);
		fieldType = b[MAX_STRING_LEN >> 2];
		fieldSize = b[(MAX_STRING_LEN >> 2) + 1];
	}
	void Save(BufType b) {
		memcpy(b, fieldName, MAX_STRING_LEN);
		b[MAX_STRING_LEN >> 2] = fieldType;
		b[(MAX_STRING_LEN >> 2) + 1] = fieldSize;
	}
};

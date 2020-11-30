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
	unsigned int fieldType;
	unsigned int fieldSize;
	Field() {
		fieldName[0] = '\0';
		fieldType = 0;
		fieldSize = 0;
	}
	Field(const char *fieldName, DataType fieldType, int para1 = 10, int para2 = 0) {
		StringValidator::Check(fieldName);
		memset(this->fieldName, 0, sizeof this->fieldName);
		strcpy(this->fieldName, fieldName);
		SetTypeAndSize(fieldType, para1, para2);
	}
	void SetTypeAndSize(DataType fieldType, int para1 = 10, int para2 = 0) {
		this->fieldType = fieldType;
		switch(fieldType) {
			case TINYINT:
				fieldSize = 1;
				break;
			case SMALLINT:
				fieldSize = 2;
				break;
			case MEDIUMINT:
				fieldSize = 3;
				break;
			case INTEGER:
				fieldSize = 4;
				break;
			case BIGINT:
				fieldSize = 8;
				break;
			case FLOAT:
				fieldSize = 4;
				break;
			case DOUBLE:
				fieldSize = 8;
				break;
			case DECIMAL:
				if(para1 < 1 || para1 > 65 || para2 < 0 || para2 > 30 || para2 > para1) {
					cerr << "Parameter invalid!" << endl;
					exit(-1);
				}
				this->fieldType = fieldType | (para1 << 8) | (para2 << 16);
				fieldSize = para1 + 2;
				break;
			case DATE:
				fieldSize = 3;
				break;
			case TIME:
				fieldSize = 3;
				break;
			case YEAR:
				fieldSize = 1;
				break;
			case DATETIME:
				fieldSize = 8;
				break;
			case TIMESTAMP:
				fieldSize = 4;
				break;
			case CHAR:
				if(para1 < 1 || para1 > 255) {
					cerr << "Parameter invalid!" << endl;
					exit(-1);
				}
				this->fieldType = fieldType | (para1 << 8);
				fieldSize = para1;
				break;
			case VARCHAR:
				if(para1 < 1 || para1 > 65535) {
					cerr << "Parameter invalid!" << endl;
					exit(-1);
				}
				this->fieldType = fieldType | (para1 << 8);
				fieldSize = para1;
				break;
			default:
				cerr << "Data type not supported!" << endl;
				exit(-1);
				break;
		}
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

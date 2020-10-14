#ifndef FIELD
#define FIELD
#include <cstdio>
#include <cstring>
#define MAX_NAME_LENGTH 24
#define FIELD_SIZE (MAX_NAME_LENGTH + 8)
using namespace std;
/*
实现了一个字段的定义
*/
class Field {
public:
	char fieldName[MAX_NAME_LENGTH + 1];
	int fieldType, fieldLength;
	Field() {
		fieldName[0] = '\0';
		fieldType = -1;
		fieldLength = -1;
	}
	Field(const char *fieldName, int fieldType, int fieldLength) {
		if(strlen(fieldName) > MAX_NAME_LENGTH)
		{
			cerr << "Field name should not be longer than 24 characters!" << endl;
			exit(-1);
		}
		strcpy(this.fieldName, fieldName);
		this.fieldType = fieldType;
		this.fieldLength = fieldLength;
	}
};
#endif

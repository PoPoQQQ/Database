#pragma once
#include <vector>
#include "Field.h"
#include "../BufManager/BufPageManager.h"
using namespace std;
/*
实现了一个字段的定义
*/
class FieldList {
public:
	vector<Field> fields;

	void LoadFields(BufType b);
	void SaveFields(BufType b);
	void LoadDatas(unsigned char* b);
	void SaveDatas(unsigned char* b);

	void AddField(Field field);
	void PrintFields();
	void PrintDatas(unsigned int bitMap);

	int FieldCount();
	int RoundedDataSize();
	Field& GetColumn(int index);
	int GetColumnIndex(const char* columnName);

};

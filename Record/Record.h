#pragma once
#include <vector>
#include <cstring>
#include "Data.h"
#include "FieldList.h"
using namespace std;

class Record {
public:
	unsigned int enabled;
	unsigned long long rid;
	unsigned int bitMap;
	FieldList fieldList;
	Record(FieldList fieldList): fieldList(fieldList) {
		if(fieldList.FieldCount() == 0) {
			cerr << "Invalid Fields!" << endl;
			exit(-1);
		}
		CleanData();
	}
	void CleanData() {
		bitMap = 0;
	}
	void FillData(const char* columnName, Data data) {
		int index = fieldList.GetColumnIndex(columnName);
		if(index == -1)
			throw "Field not found!";
		fieldList.GetColumn(index).SetData(data);
		bitMap |= (1 << index);
	}
	void Load(BufType b) {
		enabled = b[0];
		rid = (long long)b[2] << 32ll | b[1];
		bitMap = b[3];
		fieldList.LoadDatas((unsigned char*)(b + 4));
	}
	void Save(BufType b) {
		b[0] = enabled;
		b[1] = rid & 0xFFFFFFFFull;
		b[2] = rid >> 32ll;
		b[3] = bitMap;
		fieldList.SaveDatas((unsigned char*)(b + 4));
	}
	int RecordSize() {
		int recordSize = 16 + fieldList.RoundedDataSize();
		if(recordSize < MIN_RECORD_SIZE)
			recordSize = MIN_RECORD_SIZE;
		return recordSize;
	}
	void PrintRecord() {
		fieldList.PrintDatas(bitMap);
	}
};

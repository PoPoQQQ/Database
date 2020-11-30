#pragma once
#include <vector>
#include <cstring>
#include "Data.h"
#include "Field.h"
using namespace std;

class Record {
public:
	unsigned int enabled;
	unsigned long long rid;
	unsigned int bitMap;
	vector<Field> fields;
	Record(const vector<Field>& fields): fields(fields){
		if(fields.empty()) {
			cerr << "Invalid Fields!" << endl;
			exit(-1);
		}
  		CleanData();
	}
	void CleanData() {
		bitMap = 0;
	}
	void FillData(int index, Data data) {
		if(index >= (signed)fields.size()) {
			cerr << "Invalid index!" << endl;
			exit(-1);
		}
		if(data.dataType != fields[index].data.dataType) {
			cerr << "Data type does not match!" << endl;
			exit(-1);
		}
		fields[index].data = data;
		bitMap |= (1 << index);
	}
	void Save(BufType b) {
		b[0] = enabled;
		b[1] = rid & 0xFFFFFFFFull;
		b[2] = rid >> 32ll;
		b[3] = bitMap;
		int offset = 4 << 2;
		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		{
			it->data.SaveData(reinterpret_cast<unsigned char*>(b) + offset);
			offset += it->data.dataSize;
		}
	}
	void Load(BufType b) {
		enabled = b[0];
		rid = (long long)b[2] << 32ll | b[1];
		bitMap = b[3];
		int offset = 4 << 2;
		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		{
			it->data.LoadData(reinterpret_cast<unsigned char*>(b) + offset);
			offset += it->data.dataSize;
		}
	}
	int RecordSize() {
		int dataSize = 0;
		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
			dataSize += it->data.dataSize;
		int recordSize = 16 + dataSize;
		recordSize = ((recordSize | 3) ^ 3) + ((recordSize & 3) ? 4 : 0);
		if(recordSize < MIN_RECORD_SIZE)
			recordSize = MIN_RECORD_SIZE;
		return recordSize;
	}
	void PrintRecord() {
		for(int index = 0; index < (signed)fields.size(); index++)
		{
			if((bitMap & (1u << index)) == 0) {
				cout << " | NULL";
				continue;
			}
			cout << " | " << fields[index].data;
		}
		cout << " | " << endl;
	}
};

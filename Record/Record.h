#pragma once
#include <vector>
#include <cstring>
#include "Field.h"
using namespace std;

class Record {
public:
	unsigned int enabled;
	unsigned long long rid;
	unsigned int bitMap;
	const vector<Field> fields;
	vector<int> offsets;
	int dataSize;
	unsigned char* data;
	Record(const vector<Field> fields): fields(fields) {
		if(fields.empty()) {
			cerr << "Invalid Fields!" << endl;
			exit(-1);
		}
		offsets.resize(fields.size());
		dataSize = 0;
		for(int i = 0; i < (signed)fields.size(); i++)
		{
			offsets[i] = dataSize;
			dataSize += fields[i].fieldSize;
		}
		data = new unsigned char[dataSize];
		CleanData();
	}
	~Record() {
		delete[] data;
	}
	void CleanData() {
		bitMap = 0;
		memset(data, 0, dataSize);
	}
	void FillData(int index, const void *data) {
		if(index >= (signed)fields.size()) {
			cerr << "Invalid index!" << endl;
			exit(-1);
		}
		switch(fields[index].fieldType & 0xff) {
			case TINYINT:
			case SMALLINT:
			case MEDIUMINT:
			case INTEGER:
			case BIGINT:
			case FLOAT:
			case DOUBLE:
			case CHAR:
			case VARCHAR:
				memcpy(this->data + offsets[index], data, fields[index].fieldSize);
				bitMap |= (1 << index);
				break;
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}
	}
	void Save(BufType b) {
		b[0] = enabled;
		b[1] = rid & 0xFFFFFFFFull;
		b[2] = rid >> 32ll;
		b[3] = bitMap;
		memcpy(b + 4, data, dataSize);
	}
	void Load(BufType b) {
		enabled = b[0];
		rid = (long long)b[2] << 32ll | b[1];
		bitMap = b[3];
		memcpy(data, b + 4, dataSize);
	}
	int RecordSize() {
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
			switch(fields[index].fieldType & 0xff) {
			case INTEGER: {
				int buffer;
				memcpy(&buffer, this->data + offsets[index], 4);
				cout << " | " << buffer;
				break;
			}
			case CHAR: {
				char buffer[fields[index].fieldSize + 1];
				memset(buffer, 0, fields[index].fieldSize + 1);
				memcpy(buffer, this->data + offsets[index], fields[index].fieldSize);
				cout << " | " << buffer;
				break;
			}
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
			}
		}
		cout << " | " << endl;
	}
};

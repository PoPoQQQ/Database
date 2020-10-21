#pragma once
#include <vector>
#include <cstdio>
#include <cstring>
#include "Field.h"
#include "Record.h"
#include "../Utils/MyBitMap.h"
#include "../Utils/Constraints.h"
#include "../Utils/StringValidator.h"
#include "../BufManager/BufPageManager.h"
using namespace std;
/*
实现了一个表的表头
*/
class TableHeader {
public:
	char databaseName[MAX_STRING_LEN + 1];
	char tableName[MAX_STRING_LEN + 1];
	int numberOfPage;
	unsigned long long ridTimestamp;
	int recordCount;
	//int fieldCount;
	vector<Field> fields;
	int recordSize;

	TableHeader(): 
		numberOfPage(1), 
		ridTimestamp(0), 
		recordCount(0), 
		recordSize(0) {}

	void setDatabaseName(const char *name) {
		StringValidator::Check(name);
		strcpy(databaseName, name);
	}

	void setTableName(const char *name) {
		StringValidator::Check(name);
		strcpy(tableName, name);
	}

	void addField(Field field) {
		if(fields.size() >= MAX_COL_NUM)
		{
			cerr << "Too many fields!" << endl;
			exit(-1);
		}
		fields.push_back(field);
		recordSize += field.fieldSize;
	}

	void Load(BufType b) {
		int offset = 0;
		
		memset(databaseName, 0, sizeof databaseName);
		memcpy(databaseName, b + (offset >> 2), MAX_STRING_LEN);
		offset += MAX_STRING_LEN;
		
		memset(tableName, 0, sizeof databaseName);
		memcpy(tableName, b + (offset >> 2), MAX_STRING_LEN);
		offset += MAX_STRING_LEN;

		numberOfPage = b[offset >> 2];
		offset += 4;
		
		ridTimestamp = (long long)b[(offset >> 2) + 1] << 32ll | b[offset >> 2];
		offset += 8;
		
		recordCount = b[offset >> 2];
		offset += 4;
		
		unsigned int size = b[offset >> 2];
		offset += 4;

		fields.resize(size);
		recordSize = 0;
		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		{
			it->Load(b + (offset >> 2));
			recordSize += it->fieldSize;
			offset += FIELD_SIZE;
		}
	}

	void Save(BufType b) {
		int offset = 0;
		
		memcpy(b + (offset >> 2), databaseName, MAX_STRING_LEN);
		offset += MAX_STRING_LEN;
		
		memcpy(b + (offset >> 2), tableName, MAX_STRING_LEN);
		offset += MAX_STRING_LEN;

		b[offset >> 2] = numberOfPage;
		offset += 4;
		
		b[offset >> 2] = ridTimestamp & 0xFFFFFFFFull;
		b[(offset >> 2) + 1] = ridTimestamp >> 32ll;
		offset += 8;
		
		b[offset >> 2] = recordCount;
		offset += 4;
		
		b[offset >> 2] = fields.size();
		offset += 4;

		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		{
			it->Save(b + (offset >> 2));
			offset += FIELD_SIZE;
		}
	}
};

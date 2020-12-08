#pragma once
#include <map>
#include <string>
#include <cstring>
#include "PageFactory.h"
#include "../Utils/Global.h"
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"
using namespace std;
/*
管理一个索引的信息
*/
class Index {
public:
	int fileID;
	char databaseName[MAX_STRING_LEN + 1];
	char tableName[MAX_STRING_LEN + 1];
	char indexName[MAX_STRING_LEN + 1];
	int numberOfPage;
	int rootPage;
	vector<Data> keys;
	MyBitMap *bitMap;

	Index(const char *databaseName, const char *tableName, const char *indexName) {
		bitMap = NULL;

		static char dir[1000];
		sprintf(dir, "Database/%s/%s-%s", databaseName, tableName, indexName);
		Global::fm->openFile(dir, fileID);

		LoadHeader();
	}

	Index(const char *databaseName, const char *tableName, const char *indexName, vector<Data> keys): keys(keys) {
		StringValidator::Check(databaseName);
		StringValidator::Check(tableName);
		StringValidator::Check(indexName);
		if(keys.size() >= INDEX_MAX_KEYS) {
			cerr << "Too many keys!" << endl;
			exit(-1);
		}

		memset(this->databaseName, 0, MAX_STRING_LEN + 1);
		memset(this->tableName, 0, MAX_STRING_LEN + 1);
		memset(this->indexName, 0, MAX_STRING_LEN + 1);
		strcpy(this->databaseName, databaseName);
		strcpy(this->tableName, tableName);
		strcpy(this->indexName, indexName);
		numberOfPage = 1;
		rootPage = 1;

		bitMap = new MyBitMap(PAGE_SIZE << 2, 1);
		bitMap->setBit(0, 0);

		static char dir[1000];
		sprintf(dir, "Database/%s/%s-%s", databaseName, tableName, indexName);
		Global::fm->createFile(dir);
		Global::fm->openFile(dir, fileID);

		SaveHeader();
	}

	~Table() {
		Global::fm->closeFile(fileID);
		delete bitMap;
	}

	void LoadHeader() {
		int index;
		BufType b = Global::bpm->getPage(fileID, 0, index);

		int offset = 0;
		
		memcpy(databaseName, b + (offset >> 2), MAX_STRING_LEN);
		databaseName[MAX_STRING_LEN] = 0;
		offset += MAX_STRING_LEN;
		
		memcpy(tableName, b + (offset >> 2), MAX_STRING_LEN);
		tableName[MAX_STRING_LEN] = 0;
		offset += MAX_STRING_LEN;

		memcpy(indexName, b + (offset >> 2), MAX_STRING_LEN);
		indexName[MAX_STRING_LEN] = 0;
		offset += MAX_STRING_LEN;

		numberOfPage = b[offset >> 2];
		offset += 4;

		rootPage = b[offset >> 2]
		offset += 4;
		
		unsigned int size = b[offset >> 2];
		offset += 4;

		keys.resize(size);
		for(vector<Data>::iterator it = keys.begin(); it != keys.end(); it++) {
			it->LoadType(b + (offset >> 2));
			offset += 8;
		}

		if(bitMap != NULL)
			delete bitMap;
		bitMap = new MyBitMap(PAGE_SIZE << 2, b + (PAGE_INT_NUM >> 1));
	}

	void SaveHeader() {
		int index;
		BufType b = Global::bpm->getPage(fileID, 0, index);
		
		int offset = 0;
		
		memcpy(b + (offset >> 2), databaseName, MAX_STRING_LEN);
		offset += MAX_STRING_LEN;
		
		memcpy(b + (offset >> 2), tableName, MAX_STRING_LEN);
		offset += MAX_STRING_LEN;

		memcpy(b + (offset >> 2), indexName, MAX_STRING_LEN);
		offset += MAX_STRING_LEN;

		b[offset >> 2] = numberOfPage;
		offset += 4;
		
		b[offset >> 2] = rootPage;
		offset += 4;
		
		b[offset >> 2] = fields.size();
		offset += 4;

		for(vector<Data>::iterator it = keys.begin(); it != keys.end(); it++) {
			it->SaveType(b + (offset >> 2));
			offset += 8;
		}

		bitMap->save(b + (PAGE_INT_NUM >> 1));

		Global::bpm->markDirty(index);
	}
};

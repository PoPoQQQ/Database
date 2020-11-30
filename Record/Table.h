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
管理一个表的信息
*/
class Table {
public:
	int fileID;
	char databaseName[MAX_STRING_LEN + 1];
	char tableName[MAX_STRING_LEN + 1];
	int numberOfPage;
	unsigned long long ridTimestamp;
	int recordCount;
	vector<Field> fields;
	MyBitMap *bitMap;

	Table(const char *databaseName, const char *tableName) {
		bitMap = NULL;

		static char dir[1000];
		sprintf(dir, "Database/%s/%s", databaseName, tableName);
		Global::getInstance()->getInstance()->fm->openFile(dir, fileID);

		LoadHeader();
	}

	Table(const char *databaseName, const char *tableName, vector<Field> fields): fields(fields) {
		StringValidator::Check(databaseName);
		StringValidator::Check(tableName);
		if(fields.size() >= MAX_COL_NUM) {
			cerr << "Too many fields!" << endl;
			exit(-1);
		}

		memset(this->databaseName, 0, MAX_STRING_LEN + 1);
		memset(this->tableName, 0, MAX_STRING_LEN + 1);
		strcpy(this->databaseName, databaseName);
		strcpy(this->tableName, tableName);
		numberOfPage = 1;
		ridTimestamp = 0;
		recordCount = 0;

		bitMap = new MyBitMap(PAGE_SIZE << 2, 1);
		bitMap->setBit(0, 0);

		static char dir[1000];
		sprintf(dir, "Database/%s/%s", databaseName, tableName);
		Global::getInstance()->fm->createFile(dir);
		Global::getInstance()->fm->openFile(dir, fileID);

		SaveHeader();
	}

	~Table() {
		Global::getInstance()->fm->closeFile(fileID);
		delete bitMap;
	}

	void LoadHeader() {
		int index;
		BufType b = Global::getInstance()->bpm->getPage(fileID, 0, index);

		int offset = 0;
		
		memcpy(databaseName, b + (offset >> 2), MAX_STRING_LEN);
		databaseName[MAX_STRING_LEN] = 0;
		offset += MAX_STRING_LEN;
		
		memcpy(tableName, b + (offset >> 2), MAX_STRING_LEN);
		tableName[MAX_STRING_LEN] = 0;
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
		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++) {
			it->Load(b + (offset >> 2));
			offset += FIELD_SIZE;
		}

		if(bitMap != NULL)
			delete bitMap;
		bitMap = new MyBitMap(PAGE_SIZE << 2, b + (PAGE_INT_NUM >> 1));
	}

	void SaveHeader() {
		int index;
		BufType b = Global::getInstance()->bpm->getPage(fileID, 0, index);
		
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

		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++) {
			it->Save(b + (offset >> 2));
			offset += FIELD_SIZE;
		}

		bitMap->save(b + (PAGE_INT_NUM >> 1));

		Global::getInstance()->bpm->markDirty(index);
	}

	Record *CreateEmptyRecord() {
		return new Record(fields);
	}

	void AddRecord(Record *record) {
		int pageNumber = bitMap->findLeftOne();
		if(pageNumber == -1) {
			cerr << "File volume not enough!" << endl;
			exit(-1);
		}
		if(pageNumber == 0) {
			cerr << "Bit map error in class \"Table\"!" << endl;
			exit(-1);
		}
		if(pageNumber > numberOfPage) {
			cerr << "Bit map error in class \"Table\"!" << endl;
			exit(-1);
		}

		int index;
		BufType b;
		PageBase *page;

		if(pageNumber < numberOfPage) {
			b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
			page = PageFactory::LoadPageHeader(b);
		}
		else {
			numberOfPage++;
			b = Global::getInstance()->bpm->allocPage(fileID, pageNumber, index);
			page = new RecordPage;
			page->SavePageHeader(b);
		}

		if(page->type != PageBase::RECORD_PAGE)
		{
			cerr << "Illegal Page Type!" << endl;
			exit(-1);
		}

		record->enabled = 1;
		record->rid = ++ridTimestamp;
		++recordCount;

		bool full = dynamic_cast<RecordPage*>(page)->AddRecord(b, record);
		Global::getInstance()->bpm->markDirty(index);

		if(full)
			bitMap->setBit(pageNumber, 0);

		SaveHeader();
		delete page;
	}

	void PrintTable() {
		for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
			cout << " | " << it->fieldName;
		cout << " | " << endl;

		Record *record = CreateEmptyRecord();
		for(int pageNumber = 1; pageNumber < numberOfPage; pageNumber++) {
			int index;
			BufType b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
			PageBase *page = PageFactory::LoadPageHeader(b);
			if(page->type != PageBase::RECORD_PAGE)
				continue;
			dynamic_cast<RecordPage*>(page)->PrintPage(b, record);
		}
		delete record;
	}
};

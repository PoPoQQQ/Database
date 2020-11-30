#pragma once
#include <map>
#include <string>
#include <cstring>
#include "PageFactory.h"
#include "TableHeader.h"
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
	TableHeader *tableHeader;
	MyBitMap *bitMap;

	Table(const char *databaseName, const char *tableName) {
		//cout << dir << endl;
		tableHeader = new TableHeader;

		static char dir[1000];
		sprintf(dir, "Database/%s/%s", databaseName, tableName);
		Global::fm->openFile(dir, fileID);

		int index;
		BufType b = Global::bpm->getPage(fileID, 0, index);
		tableHeader->Load(b);

		bitMap = new MyBitMap(PAGE_SIZE << 2, b + (PAGE_INT_NUM >> 1));
	}

	Table(TableHeader *tableHeader): tableHeader(new TableHeader(*tableHeader)) {
		static char dir[1000];
		sprintf(dir, "Database/%s/%s", tableHeader->databaseName, tableHeader->tableName);
		Global::fm->createFile(dir);
		Global::fm->openFile(dir, fileID);

		int index;
		BufType b = Global::bpm->allocPage(fileID, 0, index);
		tableHeader->Save(b);
		
		bitMap = new MyBitMap(PAGE_SIZE << 2, 1);
		bitMap->setBit(0, 0);
		bitMap->save(b + (PAGE_INT_NUM >> 1));

		Global::bpm->markDirty(index);
	}

	~Table() {
		Global::fm->closeFile(fileID);
		delete tableHeader;
		delete bitMap;
	}

	Record *CreateEmptyRecord() {
		return new Record(tableHeader->fields);
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
		if(pageNumber > tableHeader->numberOfPage) {
			cerr << "Bit map error in class \"Table\"!" << endl;
			exit(-1);
		}

		int header_index;
		BufType header_b = Global::bpm->getPage(fileID, 0, header_index);

		int index;
		BufType b;
		PageBase *page;

		if(pageNumber < tableHeader->numberOfPage) {
			b = Global::bpm->getPage(fileID, pageNumber, index);
			page = PageFactory::LoadPageHeader(b);
		}
		else {
			tableHeader->numberOfPage++;
			b = Global::bpm->allocPage(fileID, pageNumber, index);
			page = new RecordPage;
			page->SavePageHeader(b);
		}

		if(page->type != PageBase::RECORD_PAGE)
		{
			cerr << "Illegal Page Type!" << endl;
			exit(-1);
		}

		record->enabled = 1;
		record->rid = ++tableHeader->ridTimestamp;
		++tableHeader->recordCount;
		tableHeader->Save(header_b);
		Global::bpm->markDirty(header_index);

		bool full = dynamic_cast<RecordPage*>(page)->AddRecord(b, record);
		Global::bpm->markDirty(index);

		if(full) {
			bitMap->setBit(pageNumber, 0);

			//temperate solution
			bitMap->save(header_b + (PAGE_INT_NUM >> 1));
			Global::bpm->markDirty(header_index);
		}

		delete page;
	}

	void PrintTable() {
		for(vector<Field>::iterator it = tableHeader->fields.begin(); it != tableHeader->fields.end(); it++)
			cout << " | " << it->fieldName;
		cout << " | " << endl;

		Record *record = CreateEmptyRecord();
		for(int pageNumber = 1; pageNumber < tableHeader->numberOfPage; pageNumber++) {
			int index;
			BufType b = Global::bpm->getPage(fileID, pageNumber, index);
			PageBase *page = PageFactory::LoadPageHeader(b);
			if(page->type != PageBase::RECORD_PAGE)
				continue;
			dynamic_cast<RecordPage*>(page)->PrintPage(b, record);
		}
		delete record;
	}
};

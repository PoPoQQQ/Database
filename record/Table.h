#pragma once
#include <map>
#include <string>
#include <cstring>
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

	Table(TableHeader *tableHeader): tableHeader(tableHeader) {
		static char dir[1000];
		sprintf(dir, "Database/%s/%s", tableHeader->databaseName, tableHeader->tableName);
		Global::fm->createFile(dir);
		Global::fm->openFile(dir, fileID);

		int index;
		BufType b = Global::bpm->allocPage(fileID, 0, index, false);
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
};

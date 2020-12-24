#include <cstring>
#include "Field.h"
#include "Table.h"
#include "Record.h"
#include "RecordPage.h"
#include "PageFactory.h"
#include "../Utils/Global.h"
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"

Table::Table(string databaseName, string tableName): 
	FileBase("Database/" + databaseName + "/" + tableName) {
	bitMap = NULL;

	static char dir[1000];
	sprintf(dir, "Database/%s/%s", databaseName, tableName);
	Global::getInstance()->getInstance()->fm->openFile(dir, fileID);

	LoadHeader();
}

Table::Table(string databaseName, string tableName, FieldList fieldList): fieldList(fieldList) {
	if(strlen(databaseName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(strlen(tableName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(fieldList.FieldCount() >= MAX_COL_NUM)
		throw "Too many fields!";

	memset(this->databaseName, 0, MAX_IDENTIFIER_LEN + 1);
	memset(this->tableName, 0, MAX_IDENTIFIER_LEN + 1);
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

Table::~Table() {
	Global::getInstance()->fm->closeFile(fileID);
	delete bitMap;
}

void Table::LoadHeader() {
	int pageIndex;
	BufType b = Global::getInstance()->bpm->getPage(fileID, 0, pageIndex);

	int offset = 0;
	
	memset(databaseName, 0, sizeof databaseName);
	memcpy(databaseName, b + (offset >> 2), MAX_IDENTIFIER_LEN);
	offset += MAX_IDENTIFIER_LEN;
	
	memset(tableName, 0, sizeof databaseName);
	memcpy(tableName, b + (offset >> 2), MAX_IDENTIFIER_LEN);
	offset += MAX_IDENTIFIER_LEN;

	numberOfPage = b[offset >> 2];
	offset += 4;
	
	ridTimestamp = (long long)b[(offset >> 2) + 1] << 32ll | b[offset >> 2];
	offset += 8;
	
	recordCount = b[offset >> 2];
	offset += 4;
	
	fieldList.LoadFields(b + (offset >> 2));

	if(bitMap != NULL)
		delete bitMap;
	bitMap = new MyBitMap(PAGE_SIZE << 2, b + (PAGE_INT_NUM >> 1));
}

void Table::SaveHeader() {
	int pageIndex;
	BufType b = Global::getInstance()->bpm->getPage(fileID, 0, pageIndex);
	
	int offset = 0;
	
	memcpy(b + (offset >> 2), databaseName, MAX_IDENTIFIER_LEN);
	offset += MAX_IDENTIFIER_LEN;
	
	memcpy(b + (offset >> 2), tableName, MAX_IDENTIFIER_LEN);
	offset += MAX_IDENTIFIER_LEN;

	b[offset >> 2] = numberOfPage;
	offset += 4;
	
	b[offset >> 2] = ridTimestamp & 0xFFFFFFFFull;
	b[(offset >> 2) + 1] = ridTimestamp >> 32ll;
	offset += 8;
	
	b[offset >> 2] = recordCount;
	offset += 4;

	fieldList.SaveFields(b + (offset >> 2));

	bitMap->save(b + (PAGE_INT_NUM >> 1));

	Global::getInstance()->bpm->markDirty(pageIndex);
}

Record Table::EmptyRecord() {
	return Record(fieldList);
}

void Table::AddRecord(Record record) {
	int pageNumber = bitMap->findLeftOne();
	if(pageNumber == -1) {
		cerr << "File volume not enough!" << endl;
		exit(-1);
	}
	if(pageNumber == 0 || pageNumber > numberOfPage) {
		cerr << "Bit map error in class \"Table\"!" << endl;
		exit(-1);
	}

	PageBase *page;

	if(pageNumber < numberOfPage)
		page = PageFactory::LoadPage(this, fileID, pageNumber);
	else {
		numberOfPage++;
		page = PageFactory::CreatePage(this, fileID, pageNumber, PageBase::RECORD_PAGE);
	}

	if(page == NULL || page->pageType != PageBase::RECORD_PAGE) {
		cerr << "Illegal Page Type!" << endl;
		exit(-1);
	}

	record.enabled = 1;
	record.rid = ++ridTimestamp;
	++recordCount;

	bool full = dynamic_cast<RecordPage*>(page)->AddRecord(record);
	if(full)
		bitMap->setBit(pageNumber, 0);

	SaveHeader();
	delete page;
}

void Table::PrintTable() {
	fieldList.PrintFields();

	for(int pageNumber = 1; pageNumber < numberOfPage; pageNumber++) {
		PageBase *page = PageFactory::LoadPage(this, fileID, pageNumber);
		if(page == NULL)
			continue;
		if(page->pageType == PageBase::RECORD_PAGE)
			dynamic_cast<RecordPage*>(page)->PrintPage();
		delete page;
	}
}

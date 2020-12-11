#include <cstring>
#include "Field.h"
#include "Table.h"
#include "Record.h"
#include "RecordPage.h"
#include "PageFactory.h"
#include "../Utils/Global.h"
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"

Table::Table(const char *databaseName, const char *tableName) {
	bitMap = NULL;

	static char dir[1000];
	sprintf(dir, "Database/%s/%s", databaseName, tableName);
	Global::getInstance()->getInstance()->fm->openFile(dir, fileID);

	LoadHeader();
}

Table::Table(const char *databaseName, const char *tableName, vector<Field> fields): fields(fields) {
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

Table::~Table() {
	Global::getInstance()->fm->closeFile(fileID);
	delete bitMap;
}

void Table::LoadHeader() {
	int pageIndex;
	BufType b = Global::getInstance()->bpm->getPage(fileID, 0, pageIndex);

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

void Table::SaveHeader() {
	int pageIndex;
	BufType b = Global::getInstance()->bpm->getPage(fileID, 0, pageIndex);
	
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

	Global::getInstance()->bpm->markDirty(pageIndex);
}

Record* Table::CreateEmptyRecord() {
	return new Record(fields);
}

void Table::AddRecord(Record *record) {
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

	record->enabled = 1;
	record->rid = ++ridTimestamp;
	++recordCount;

	bool full = dynamic_cast<RecordPage*>(page)->AddRecord(record);
	if(full)
		bitMap->setBit(pageNumber, 0);

	SaveHeader();
	delete page;
}

void Table::PrintTable() {
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		cout << " | " << it->fieldName;
	cout << " | " << endl;

	for(int pageNumber = 1; pageNumber < numberOfPage; pageNumber++) {
		PageBase *page = PageFactory::LoadPage(this, fileID, pageNumber);
		if(page == NULL)
			continue;
		if(page->pageType == PageBase::RECORD_PAGE)
			dynamic_cast<RecordPage*>(page)->PrintPage();
		delete page;
	}
}

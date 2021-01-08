#include <cstring>
#include <iostream>
#include <algorithm>
#include "Field.h"
#include "Table.h"
#include "Record.h"
#include "RecordPage.h"
#include "../Utils/Global.h"
#include "../Pages/PageFactory.h"
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"
using namespace std;

Table::Table(string databaseName, string tableName): 
	FileBase("Database/" + databaseName + "/" + tableName, false),
	databaseName(databaseName), tableName(tableName) {
	LoadHeader();
}

Table::Table(string databaseName, string tableName, FieldList fieldList):
	FileBase("Database/" + databaseName + "/" + tableName, true),
	databaseName(databaseName), tableName(tableName),
	ridTimestamp(0), recordCount(0), fieldList(fieldList) {
	if(databaseName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(tableName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(fieldList.FieldCount() >= MAX_COL_NUM)
		throw "Too many fields!";
	SaveHeader();
}

void Table::LoadHeader() {
	int pageIndex;
	BufType b = GetHeaderBufType();

	int offset = 0;
	
	char _databaseName[MAX_IDENTIFIER_LEN + 1];
	memcpy(_databaseName, b + (offset >> 2), MAX_IDENTIFIER_LEN);
	_databaseName[MAX_IDENTIFIER_LEN] = 0;
	databaseName = string(_databaseName);
	offset += MAX_IDENTIFIER_LEN;
	
	char _tableName[MAX_IDENTIFIER_LEN + 1];
	memcpy(_tableName, b + (offset >> 2), MAX_IDENTIFIER_LEN);
	_tableName[MAX_IDENTIFIER_LEN] = 0;
	tableName = string(_tableName);
	offset += MAX_IDENTIFIER_LEN;

	numberOfPage = b[offset >> 2];
	offset += 4;
	
	ridTimestamp = (long long)b[(offset >> 2) + 1] << 32ll | b[offset >> 2];
	offset += 8;
	
	recordCount = b[offset >> 2];
	offset += 4;
	
	fieldList.LoadFields(b + (offset >> 2));
}

void Table::SaveHeader() const {
	int pageIndex;
	BufType b = GetHeaderBufType();
	
	int offset = 0;
	
	memcpy(b + (offset >> 2), databaseName.c_str(), min((unsigned)MAX_IDENTIFIER_LEN, (unsigned)databaseName.length() + 1));
	offset += MAX_IDENTIFIER_LEN;
	
	memcpy(b + (offset >> 2), tableName.c_str(), min((unsigned)MAX_IDENTIFIER_LEN, (unsigned)tableName.length() + 1));
	offset += MAX_IDENTIFIER_LEN;

	b[offset >> 2] = numberOfPage;
	offset += 4;
	
	b[offset >> 2] = ridTimestamp & 0xFFFFFFFFull;
	b[(offset >> 2) + 1] = ridTimestamp >> 32ll;
	offset += 8;
	
	b[offset >> 2] = recordCount;
	offset += 4;

	fieldList.SaveFields(b + (offset >> 2));

	MarkDirty();
}

Record Table::EmptyRecord() {
	return Record(fieldList);
}

vector<int> Table::GetColumnIndexes(const vector<string>& columnList) {
	vector<int> columnIndexes;
	for(vector<string>::const_iterator it = columnList.begin(); it != columnList.end(); it++) {
		int columnIndex = fieldList.GetColumnIndex(*it);
		if(columnIndex == -1)
			throw "Column does not exist!";
		columnIndexes.push_back(columnIndex);
	}
	return columnIndexes;
}

vector<Data> Table::GetKeyTypes(const vector<string>& columnList) {
	vector<Data> keyTypes;
	for(vector<string>::const_iterator it = columnList.begin(); it != columnList.end(); it++) {
		int columnIndex = fieldList.GetColumnIndex(*it);
		if(columnIndex == -1)
			throw "Column does not exist!";
		Data data = fieldList.GetColumn(columnIndex).GetData();
		if((data.dataType & 0xff) == Data::VARCHAR)
			data = Data(Data::INT);
		keyTypes.push_back(data);
	}
	return keyTypes;
}

void Table::InsertAllIntoIndex(Index* index) {
	vector<int> columnIndexes = GetColumnIndexes(index->colNames);
	for(int pageNumber = 1; pageNumber < numberOfPage; pageNumber++) {
		PageBase *page = LoadPage(pageNumber);
		if(page == NULL)
			continue;
		if(page->pageType == PageBase::RECORD_PAGE)
			dynamic_cast<RecordPage*>(page)->InsertPageIntoIndex(index, columnIndexes);
		delete page;
	}
}

void Table::InsertRecordIntoIndex(Index* index, const vector<int>& columnIndexes, 
	Record record, unsigned int recordPosition) {
	vector<Data> datas;
	for(vector<int>::const_iterator it = columnIndexes.begin(); it != columnIndexes.end(); it++) {
		if((record.bitMap & (1 << *it)) == 0)
			return;
		Data data = record.fieldList.GetColumn(*it).GetData();
		if((data.dataType & 0xff) != Data::VARCHAR)
			datas.push_back(data);
		else
			datas.push_back(HashData(data));
	}
	index->Insert(datas, recordPosition);
}

void Table::AddRecord(Record &record, unsigned int& recordPosition) {
	PageBase *page = GetAvailablePage(PageBase::RECORD_PAGE);

	record.enabled = 1;
	record.rid = ++ridTimestamp;
	++recordCount;

	unsigned int recordIndex;
	bool full = dynamic_cast<RecordPage*>(page)->AddRecord(record, recordIndex);
	recordPosition = page->pageNumber << 8 | recordIndex;
	if(full)
		SetBit(page->pageNumber, 0);
	// 更新 Header 数据，比如说 recordCount
	SaveHeader();
	delete page;
}

void Table::AddRecords(vector<Record>& records, const vector<Index*>& idxes) {
	vector<unsigned int> recordPositions;
	for(vector<Record>::iterator it = records.begin(); it != records.end(); it++) {
		unsigned int recordPosition;
		AddRecord(*it, recordPosition);
		recordPositions.push_back(recordPosition);
	}
	for(vector<Index*>::const_iterator it = idxes.begin(); it != idxes.end(); it++) {
		vector<int> columnIndexes = GetColumnIndexes((*it)->colNames);
		for(int i = 0; i < (signed)records.size(); i++)
			InsertRecordIntoIndex(*it, columnIndexes, records[i], recordPositions[i]);
	}
}

void Table::PrintTable() {
	fieldList.PrintFields();

	for(int pageNumber = 1; pageNumber < numberOfPage; pageNumber++) {
		PageBase *page = LoadPage(pageNumber);
		if(page == NULL)
			continue;
		if(page->pageType == PageBase::RECORD_PAGE)
			dynamic_cast<RecordPage*>(page)->PrintPage();
		delete page;
	}
}

void Table::IterTable(function<void(Record&, BufType)> iterFunc) {
	for(int pageNumber = 1; pageNumber < numberOfPage; pageNumber++) {
		PageBase *page = PageFactory::LoadPage(this, fileID, pageNumber);
		if(page == NULL)
			continue;
		if(page->pageType == PageBase::RECORD_PAGE)
			dynamic_cast<RecordPage*>(page)->IterPage(iterFunc);
		delete page;
	}
}

void Table::DescTable() const {
	this->fieldList.DescFields();
}

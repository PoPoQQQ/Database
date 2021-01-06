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
	
	memcpy(b + (offset >> 2), databaseName.c_str(), min((unsigned long)MAX_IDENTIFIER_LEN, databaseName.length() + 1));
	offset += MAX_IDENTIFIER_LEN;
	
	memcpy(b + (offset >> 2), tableName.c_str(), min((unsigned long)MAX_IDENTIFIER_LEN, tableName.length() + 1));
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

void Table::AddRecord(Record record) {
	PageBase *page = GetAvailablePage(PageBase::RECORD_PAGE);

	record.enabled = 1;
	record.rid = ++ridTimestamp;
	++recordCount;

	bool full = dynamic_cast<RecordPage*>(page)->AddRecord(record);
	if(full)
		SetBit(page->pageNumber, 0);
	// 更新 Header 数据，比如说 recordCount
	SaveHeader();
	delete page;
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
	// get max length
	int max_field_length = 5;
	int max_type_length = 4;
	int max_null_length = 4;
	int max_key_length = 3;
	int max_default_length = 7;
	// max field length
	for(vector<Field>::const_iterator it = fieldList.fields.begin(); it != fieldList.fields.end(); it++) {
		max_field_length = max(max_field_length, (int) strlen(it->columnName));
		char buf[256];
		switch(it->data.dataType & 0xff) {
			case Data::INT:
				snprintf(buf, 256, "INT(%d)", it->data.dataType >> 8);
				max_type_length = max(max_type_length, (int) strlen(buf));
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%d", it->data.intData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::VARCHAR:
				snprintf(buf, 256, "VARCHAR(%d)", it->data.dataSize);
				max_type_length = max(max_type_length, (int) strlen(buf));
				if(it->constraints & Field::DEFAULT) {
					max_default_length = max(max_default_length, min((int) strlen(it->data.stringData), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::DATE:
				// data == type : 4 == 4
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%d", it->data.intData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::FLOAT:
				max_type_length = max(max_type_length, 5);
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%f", it->data.floatData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			default:
				cerr << it->data.dataType << endl;
				throw "Error: error type in DescTable";
		}
	}

	// print header
	cout << "+";
	for(int i = 0; i < max_field_length + 2; ++i)
		cout << "-";
	cout << "+";
	for(int i = 0;i < max_type_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_null_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_key_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_default_length + 2; ++i) {
		cout << "-";
	}
	cout << "+" << endl;
	
	cout << "| Field";
	for(int i = 0; i < max_field_length + 2 - 6; ++i)
		cout << " ";
	cout << "| Type";
	for(int i = 0; i < max_type_length + 2 - 5; ++i)
		cout << " ";
	cout << "| Null ";

	cout << "| Key ";

	cout << "| Default";
	for(int i = 0;i < max_default_length + 2 - 8; ++i) {
		cout << " ";
	}
	cout << "|" << endl;

	cout << "+";
	for(int i = 0; i < max_field_length + 2; ++i)
		cout << "-";
	cout << "+";
	for(int i = 0;i < max_type_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_null_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_key_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_default_length + 2; ++i) {
		cout << "-";
	}
	cout << "+" << endl;

	for(vector<Field>::const_iterator it = fieldList.fields.begin(); it != fieldList.fields.end(); it++) {
		cout << "| " << it->columnName;
		for(int i = 0; i < max_field_length + 2 - (int) strlen(it->columnName) - 1; i++)
			cout << " ";
		
		cout << "| ";
		char buf[256];
		switch(it->data.dataType & 0xff) {
			case Data::INT:
				snprintf(buf, 256, "INT(%d)", it->data.dataType >> 8);
				cout << buf;
				for(int i = 0; i < max_type_length + 2 - (int) strlen(buf) - 1; i++)
					cout << " ";
				break;
			case Data::VARCHAR:
				snprintf(buf, 256, "VARCHAR(%d)", it->data.dataSize);
				cout << buf;
				for(int i = 0; i < max_type_length + 2 - (int) strlen(buf) - 1; i++)
					cout << " ";
				break;
			case Data::DATE:
				cout << "DATE";
				for(int i = 0; i < max_type_length + 2 - 4 - 1; i++)
					cout << " ";
				break;
			case Data::FLOAT:
				cout << "FLOAT";
				for(int i = 0; i < max_type_length + 2 - 5 - 1; i++)
					cout << " ";
				break;
			default:
				cout << "TYPE";
				for(int i = 0; i < max_type_length + 2 - 4 - 1; i++)
					cout << " ";
				break;
		}

		cout << "| ";	
		if(it->constraints & Field::NOT_NULL) {
			cout << "NO   ";
		} else {
			cout << "YES  ";
		}

		cout << "| ";
		if(it->constraints & Field::PRIMARY_KEY) {
			cout << "PRI ";
		} else if (it->constraints & Field::FOREIGN_KEY) {
			cout << "MUL ";
		} else {
			cout << "    ";
		}

		cout << "| ";
		if(it->constraints & Field::DEFAULT) {
			switch(it->data.dataType){
				default:
					cout << "DEFAULT ";
					break;
			}
		} else {
			cout << "NULL";
			for(int i = 0;i < max_default_length + 2 - 4 - 1; ++i) {
				cout << " ";
			}
		}
		cout << "|" << endl;
	}
	// print end line
	cout << "+";
	for(int i = 0; i < max_field_length + 2; ++i)
		cout << "-";
	cout << "+";
	for(int i = 0;i < max_type_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_null_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_key_length + 2; ++i) {
		cout << "-";
	}
	cout << "+";
	for(int i = 0;i < max_default_length + 2; ++i) {
		cout << "-";
	}
	cout << "+" << endl;
}

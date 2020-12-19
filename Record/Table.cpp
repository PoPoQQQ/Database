#include <cstring>
#include <algorithm>
#include "Field.h"
#include "Table.h"
#include "Record.h"
#include "RecordPage.h"
#include "PageFactory.h"
#include "../Utils/Global.h"
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"
using namespace std;

Table::Table(const char *databaseName, const char *tableName) {
	bitMap = NULL;

	static char dir[1000];
	sprintf(dir, "Database/%s/%s", databaseName, tableName);
	Global::getInstance()->getInstance()->fm->openFile(dir, fileID);

	LoadHeader();
}

Table::Table(const char *databaseName, const char *tableName, const FieldList& fieldList): fieldList(fieldList) {
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

void Table::SaveHeader() const {
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
	// 查看是否还有表是否还有空间
	int pageNumber = bitMap->findLeftOne();
	if(pageNumber == -1) {
		cerr << "File volume not enough!" << endl;
		exit(-1);
	}
	if(pageNumber == 0 || pageNumber > numberOfPage) {
		cerr << "Bit map error in class \"Table\"!" << endl;
		exit(-1);
	}
	// 获得（或创建）用于储存 Record 的页面
	// 此时获得的页面必然有空间储存 Record
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
	// 更新 Header 数据，比如说 recordCount
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
			case Data::DataType::INT:
				snprintf(buf, 256, "INT(%d)", it->data.dataType >> 8);
				max_type_length = max(max_type_length, (int) strlen(buf));
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%d", it->data.intData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::DataType::VARCHAR:
				snprintf(buf, 256, "VARCHAR(%d)", it->data.dataSize);
				max_type_length = max(max_type_length, (int) strlen(buf));
				if(it->constraints & Field::DEFAULT) {
					max_default_length = max(max_default_length, min((int) strlen(it->data.stringData), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::DataType::DATE:
				// data == type : 4 == 4
				if(it->constraints & Field::DEFAULT) {
					snprintf(buf, sizeof(buf), "%d", it->data.intData);
					max_default_length = max(max_default_length, min((int) strlen(buf), MAX_IDENTIFIER_LEN));
				}
				break;
			case Data::DataType::FLOAT:
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
			case Data::DataType::INT:
				snprintf(buf, 256, "INT(%d)", it->data.dataType >> 8);
				cout << buf;
				for(int i = 0; i < max_type_length + 2 - (int) strlen(buf) - 1; i++)
					cout << " ";
				break;
			case Data::DataType::VARCHAR:
				snprintf(buf, 256, "VARCHAR(%d)", it->data.dataSize);
				cout << buf;
				for(int i = 0; i < max_type_length + 2 - (int) strlen(buf) - 1; i++)
					cout << " ";
				break;
			case Data::DataType::DATE:
				cout << "DATE";
				for(int i = 0; i < max_type_length + 2 - 4 - 1; i++)
					cout << " ";
				break;
			case Data::DataType::FLOAT:
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
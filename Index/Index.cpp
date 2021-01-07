#include <cstring>
#include "Index.h"
#include "../Utils/Global.h"
#include "BplusLeafNodePage.h"
#include "BplusInnerNodePage.h"
#include "../Pages/PageFactory.h"
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"
using namespace std;

Index::Index(string databaseName, string tableName, string indexName):
	FileBase("Database/" + databaseName + "/" + tableName + "-" + indexName, false), 
	databaseName(databaseName), tableName(tableName), indexName(indexName) {
	LoadHeader();
}

Index::Index(string databaseName, string tableName, string indexName, 
	const vector<string> colNames, const vector<Data>& keyTypes): 
	FileBase("Database/" + databaseName + "/" + tableName + "-" + indexName, true),
	databaseName(databaseName), tableName(tableName), indexName(indexName),
	rootPage(0), colNames(colNames), keyTypes(keyTypes) {
	if(databaseName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(tableName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(indexName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	
	if(colNames.empty())
		throw "No column!";
	if(colNames.size() >= INDEX_MAX_KEYS)
		throw "Too many columns!";

	SaveHeader();

	PageBase* page = GetAvailablePage(PageBase::BPLUS_LEAF_NODE_PAGE);
	SetBit(page->pageNumber, 0);
	rootPage = page->pageNumber;
	delete page;

	SaveHeader();
}

void Index::LoadHeader() {
	int index;
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

	char _indexName[MAX_IDENTIFIER_LEN + 1];
	memcpy(_indexName, b + (offset >> 2), MAX_IDENTIFIER_LEN);
	_indexName[MAX_IDENTIFIER_LEN] = 0;
	indexName = string(_indexName);
	offset += MAX_IDENTIFIER_LEN;

	numberOfPage = b[offset >> 2];
	offset += 4;

	rootPage = b[offset >> 2];
	offset += 4;
	
	unsigned int size = b[offset >> 2];
	offset += 4;

	colNames.resize(size);
	for(vector<string>::iterator it = colNames.begin(); it != colNames.end(); it++) {
		char buffer[MAX_IDENTIFIER_LEN + 1];
		memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
		buffer[MAX_IDENTIFIER_LEN] = 0;
		*it = string(buffer);
		offset += MAX_IDENTIFIER_LEN;
	}

	keyTypes.resize(size);
	for(vector<Data>::iterator it = keyTypes.begin(); it != keyTypes.end(); it++) {
		it->LoadType(b + (offset >> 2));
		offset += 8;
	}
}

void Index::SaveHeader() const {
	int index;
	BufType b = GetHeaderBufType();
	
	int offset = 0;
	
	memcpy(b + (offset >> 2), databaseName.c_str(), min((unsigned)MAX_IDENTIFIER_LEN, databaseName.length() + 1));
	offset += MAX_IDENTIFIER_LEN;
	
	memcpy(b + (offset >> 2), tableName.c_str(), min((unsigned)MAX_IDENTIFIER_LEN, tableName.length() + 1));
	offset += MAX_IDENTIFIER_LEN;

	memcpy(b + (offset >> 2), indexName.c_str(), min((unsigned)MAX_IDENTIFIER_LEN, indexName.length() + 1));
	offset += MAX_IDENTIFIER_LEN;

	b[offset >> 2] = numberOfPage;
	offset += 4;
	
	b[offset >> 2] = rootPage;
	offset += 4;
	
	b[offset >> 2] = colNames.size();
	offset += 4;

	for(vector<string>::const_iterator it = colNames.begin(); it != colNames.end(); it++) {
		char buffer[MAX_IDENTIFIER_LEN + 1];
		strcpy(buffer, it->c_str());
		memcpy(b + (offset >> 2), buffer, MAX_IDENTIFIER_LEN);
		offset += MAX_IDENTIFIER_LEN;
	}

	for(vector<Data>::const_iterator it = keyTypes.begin(); it != keyTypes.end(); it++) {
		it->SaveType(b + (offset >> 2));
		offset += 8;
	}
	
	MarkDirty();
}

void Index::Insert(vector<Data> keys, int value) {
	bool added = false;
	vector<Data> addedKey;
	int addedValue;

	BplusNodePage* page = dynamic_cast<BplusNodePage*>(LoadPage(rootPage));
	page->Insert(keys, value, added, addedKey, addedValue);
	delete page;

	if(!added)
		return;

	BplusInnerNodePage* _page = dynamic_cast<BplusInnerNodePage*>(GetAvailablePage(PageBase::BPLUS_INNER_NODE_PAGE));
	_page->SetValue(0, rootPage << 8);
	_page->InsertKeyAndValue(0, addedKey, addedValue);
	SetBit(_page->pageNumber, 0);

	rootPage = _page->pageNumber;
	SaveHeader();
	delete _page;
}

void Index::Remove(vector<Data> keys) {
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(LoadPage(rootPage));
	page->Remove(keys);
	if(page->pageType == PageBase::BPLUS_LEAF_NODE_PAGE || page->keyCount > 0) {
		delete page;
		return;
	}
	int pageNumber = page->GetValue(0) >> 8;
	delete page;
	FreePage(rootPage);
	rootPage = pageNumber;
}

void Index::Print() {
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(LoadPage(rootPage));
	page->Print(keyTypes, 1);
	delete page;
}

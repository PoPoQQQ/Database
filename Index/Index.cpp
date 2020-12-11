#include "Index.h"
#include "../Utils/Global.h"
#include "BplusLeafNodePage.h"
#include "BplusInnerNodePage.h"
#include "../FileIO/FileManager.h"
#include "../Record/PageFactory.h"
#include "../Utils/StringValidator.h"
#include "../BufManager/BufPageManager.h"
using namespace std;

Index::Index(const char *databaseName, const char *tableName, const char *indexName) {
	bitMap = NULL;

	static char dir[1000];
	sprintf(dir, "Database/%s/%s-%s", databaseName, tableName, indexName);
	Global::getInstance()->fm->openFile(dir, fileID);

	LoadHeader();
}

Index::Index(const char *databaseName, const char *tableName, const char *indexName, vector<Data> keys): keys(keys) {
	StringValidator::Check(databaseName);
	StringValidator::Check(tableName);
	StringValidator::Check(indexName);
	if(keys.empty()) {
		cerr << "No keys!" << endl;
		exit(-1);
	}
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
	rootPage = 0;

	bitMap = new MyBitMap(PAGE_SIZE << 2, 1);
	bitMap->setBit(0, 0);

	static char dir[1000];
	sprintf(dir, "Database/%s/%s-%s", databaseName, tableName, indexName);
	Global::getInstance()->fm->createFile(dir);
	Global::getInstance()->fm->openFile(dir, fileID);

	SaveHeader();

	PageBase* page = CreatePage(PageBase::BPLUS_LEAF_NODE_PAGE);
	rootPage = page->pageNumber;
	delete page;

	SaveHeader();
}

Index::~Index() {
	Global::getInstance()->fm->closeFile(fileID);
	delete bitMap;
}

void Index::LoadHeader() {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, 0, index);

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

	rootPage = b[offset >> 2];
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

void Index::SaveHeader() {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, 0, index);
	
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
	
	b[offset >> 2] = keys.size();
	offset += 4;

	for(vector<Data>::iterator it = keys.begin(); it != keys.end(); it++) {
		it->SaveType(b + (offset >> 2));
		offset += 8;
	}

	bitMap->save(b + (PAGE_INT_NUM >> 1));

	Global::getInstance()->bpm->markDirty(index);
}

PageBase* Index::LoadPage(int pageNumber) {
	if(pageNumber < 0 || pageNumber >= numberOfPage) {
		cerr << "Invalid page number!" << endl;
		exit(-1);
	}
	if(bitMap->getBit(pageNumber) == 1) {
		cerr << "Page does not exist!" << endl;
		exit(-1);
	}
	return PageFactory::LoadPage(this, fileID, pageNumber);
}

PageBase* Index::CreatePage(int pageType) {
	int pageNumber = bitMap->findLeftOne();
	if(pageNumber == -1) {
		cerr << "File volume not enough!" << endl;
		exit(-1);
	}
	if(pageNumber == 0 || pageNumber > numberOfPage) {
		cerr << "Bit map error in class \"Index\"!" << endl;
		exit(-1);
	}
	if(pageNumber == numberOfPage)
		++numberOfPage;
	bitMap->setBit(pageNumber, 0);
	SaveHeader();
	return PageFactory::CreatePage(this, fileID, pageNumber, pageType);
}

void Index::FreePage(int pageNumber) {
	if(pageNumber == 0 || pageNumber >= numberOfPage || bitMap->getBit(pageNumber) == 1) {
		cerr << "Unable to free this page!" << endl;
		exit(-1);
	}
	bitMap->setBit(pageNumber, 1);
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

	BplusInnerNodePage* _page = dynamic_cast<BplusInnerNodePage*>(CreatePage(PageBase::BPLUS_INNER_NODE_PAGE));
	_page->SetValue(0, rootPage << 8);
	_page->InsertKeyAndValue(0, addedKey, addedValue);

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
	page->Print(keys, 1);
	delete page;
}

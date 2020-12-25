#include "BplusLeafNodePage.h"

BplusLeafNodePage::BplusLeafNodePage(FileBase* context, int pageNumber, int pageIndex, BufType b):
	BplusNodePage(context, pageNumber, pageIndex, b) {
		pageType = PageBase::BPLUS_LEAF_NODE_PAGE;
		nextPage = 0;
	}

void BplusLeafNodePage::LoadPageHeader() {
	BplusNodePage::LoadPageHeader();
	nextPage = b[2];
}

void BplusLeafNodePage::SavePageHeader() {
	BplusNodePage::SavePageHeader();
	b[2] = nextPage;
	MarkDirty();
}

void BplusLeafNodePage::Insert(vector<Data> keys, int value,
	bool& added, vector<Data>& addedKey, int& addedValue) {
	int index;
	for(index = 0; index < keyCount; index++) {
		vector<Data> keys_index = GetKey(index);
		if(keys == keys_index) {
			cerr << "Duplicate key is not supported!" << endl;
			exit(-1);
		}
		if(keys < keys_index)
			break;
	}
	InsertKeyAndValue(index, keys, value);
	if(keyCount <= BPLUS_TREE_RANK)
		return;
	int leftCount = keyCount >> 1;
	int rightCount = keyCount - leftCount;
	BplusLeafNodePage* _page = dynamic_cast<BplusLeafNodePage*>(context->GetAvailablePage(PageBase::BPLUS_LEAF_NODE_PAGE));
	context->bitMap->setBit(_page->pageNumber, 0);
	added = true;
	addedKey = GetKey(leftCount);
	addedValue = _page->pageNumber << 8;
	Split(_page, leftCount);
}

void BplusLeafNodePage::Remove(vector<Data> keys) {
	for(int index = 0; index < keyCount; index++) {
		vector<Data> keys_index = GetKey(index);
		if(keys < keys_index) {
			cerr << "Keys not found!" << endl;
			exit(-1);
		}
		if(keys == keys_index) {
			RemoveKeyAndValue(index);
			return;
		}
	}
	cerr << "Keys not found!" << endl;
	exit(-1); 
}

void BplusLeafNodePage::Print(vector<Data> keys, int indent) {
	for(int index = 0; index < keyCount; index++) {
		for(int i = 0; i < indent; i++)
			cout << "----";
		cout << " " << GetKey(index) << ": " << GetValue(index + 1) << endl;
	}
	for(int i = 0; i < indent; i++)
		cout << "----";
	cout << " Next = " << nextPage << endl;
}

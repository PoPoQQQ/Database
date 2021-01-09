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

void BplusLeafNodePage::Insert(vector<Data> keys, unsigned int value,
	bool& added, vector<Data>& addedKey, unsigned int& addedValue) {
	int index;
	for(index = 0; index < keyCount; index++) {
		vector<Data> keys_index = GetKey(index);
		if(keys < keys_index)
			break;
	}
	InsertKeyAndValue(index, keys, value);
	if(keyCount <= BPLUS_TREE_RANK)
		return;
	int leftCount = keyCount >> 1;
	int rightCount = keyCount - leftCount;
	BplusLeafNodePage* _page = dynamic_cast<BplusLeafNodePage*>(context->GetAvailablePage(PageBase::BPLUS_LEAF_NODE_PAGE));
	context->SetBit(_page->pageNumber, 0);
	added = true;
	addedKey = GetKey(leftCount);
	addedValue = _page->pageNumber << 8;
	Split(_page, leftCount);
}

void BplusLeafNodePage::Remove(vector<Data> keys, unsigned int value) {
	for(int index = 0; index < keyCount; index++) {
		vector<Data> keys_index = GetKey(index);
		if(keys < keys_index)
			throw "Keys not found!";
		if(keys == keys_index && value == GetValue(index + 1)) {
			RemoveKeyAndValue(index);
			return;
		}
	}
	if(nextPage <= 0)
		throw "Keys not found!";
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(context->LoadPage(nextPage));
	page->Remove(keys, value);
	delete page;
}

void BplusLeafNodePage::Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer) {
	int index;
	for(index = 0; index < keyCount; index++) {
		vector<Data> keys_index = GetKey(index);
		if(upperBound < keys_index)
			return;
		if(!(keys_index < lowerBound))
			gatherer.push_back(GetValue(index + 1));
	}
	if(nextPage <= 0)
		return;
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(context->LoadPage(nextPage));
	page->Search(lowerBound, upperBound, gatherer);
	delete page;
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

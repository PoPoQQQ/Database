#include "BplusLeafNodePage.h"
#include "BplusInnerNodePage.h"

BplusInnerNodePage::BplusInnerNodePage(FileBase* context, int pageNumber, int pageIndex, BufType b):
	BplusNodePage(context, pageNumber, pageIndex, b) {
	pageType = PageBase::BPLUS_INNER_NODE_PAGE;
}

void BplusInnerNodePage::Zig(int index, BplusNodePage* leftSon, BplusNodePage *rightSon) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	if(leftSon->keyCount < 2) {
		cerr << "Keys in left son is not enough!" << endl;
		exit(-1);
	}
	if(leftSon->pageType == PageBase::BPLUS_LEAF_NODE_PAGE) {
		rightSon->InsertKeyAndValue(0, leftSon->GetKey(leftSon->keyCount - 1), leftSon->GetValue(leftSon->keyCount));
		SetKey(index, rightSon->GetKey(0));
		leftSon->RemoveKeyAndValue(leftSon->keyCount - 1);
	}
	else {
		rightSon->InsertValueAndKey(0, leftSon->GetValue(leftSon->keyCount), GetKey(index));
		SetKey(index, leftSon->GetKey(leftSon->keyCount - 1));
		leftSon->RemoveKeyAndValue(leftSon->keyCount - 1);
	}
}

void BplusInnerNodePage::Zag(int index, BplusNodePage* leftSon, BplusNodePage* rightSon) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	if(rightSon->keyCount < 2) {
		cerr << "Keys in right son is not enough!" << endl;
		exit(-1);
	}
	if(rightSon->pageType == PageBase::BPLUS_LEAF_NODE_PAGE) {
		leftSon->InsertKeyAndValue(leftSon->keyCount, rightSon->GetKey(0), rightSon->GetValue(1));
		SetKey(index, rightSon->GetKey(1));
		rightSon->RemoveKeyAndValue(0);
	}
	else {
		leftSon->InsertKeyAndValue(leftSon->keyCount, GetKey(index), rightSon->GetValue(0));
		SetKey(index, rightSon->GetKey(0));
		rightSon->RemoveValueAndKey(0);
	}
}

void BplusInnerNodePage::Merge(int index, BplusNodePage* leftSon, BplusNodePage* rightSon) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	if(leftSon->pageType == PageBase::BPLUS_LEAF_NODE_PAGE) {
		int offset = PAGE_OFFSET + INDEX_SIZE * leftSon->keyCount + 4;
		for(int i = (offset >> 2); i <= PAGE_INT_NUM - 1; i++)
			leftSon->b[i] = rightSon->b[i - (INDEX_SIZE * leftSon->keyCount >> 2)];
		leftSon->MarkDirty();
		leftSon->keyCount += rightSon->keyCount;
		dynamic_cast<BplusLeafNodePage*>(leftSon)->nextPage = dynamic_cast<BplusLeafNodePage*>(rightSon)->nextPage;
		leftSon->SavePageHeader();
		RemoveKeyAndValue(index);
	}
	else {
		int offset = PAGE_OFFSET + INDEX_SIZE * (leftSon->keyCount + 1);
		for(int i = (offset >> 2); i <= PAGE_INT_NUM - 1; i++)
			leftSon->b[i] = rightSon->b[i - (INDEX_SIZE * (leftSon->keyCount + 1) >> 2)];
		leftSon->MarkDirty();
		int _index = leftSon->keyCount;
		leftSon->keyCount += rightSon->keyCount + 1;
		leftSon->SavePageHeader();
		leftSon->SetKey(_index, GetKey(index));
		RemoveKeyAndValue(index);
	}
}

void BplusInnerNodePage::Insert(vector<Data> keys, unsigned int value,
	bool& added, vector<Data>& addedKey, unsigned int& addedValue) {
	int index;
	for(index = 0; index < keyCount; index++)
		if(keys < GetKey(index))
			break;
	int pageNumber = GetValue(index) >> 8;
	bool _added = false;
	vector<Data> _addedKey;
	unsigned int _addedValue = 0;
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(context->LoadPage(pageNumber));
	page->Insert(keys, value, _added, _addedKey, _addedValue);
	delete page;
	if(!_added)
		return;
	InsertKeyAndValue(index, _addedKey, _addedValue);
	if(keyCount <= BPLUS_TREE_RANK)
		return;
	int leftCount = BPLUS_TREE_RANK >> 1;
	int rightCount = keyCount - leftCount - 1;
	BplusInnerNodePage* _page = dynamic_cast<BplusInnerNodePage*>(context->GetAvailablePage(PageBase::BPLUS_INNER_NODE_PAGE));
	context->SetBit(_page->pageNumber, 0);
	added = true;
	addedKey = GetKey(leftCount);
	addedValue = _page->pageNumber << 8;
	Split(_page, leftCount + 1);
	keyCount--;
	SavePageHeader();
	MarkDirty();
}

void BplusInnerNodePage::Remove(vector<Data> keys, unsigned int value) {
	int index;
	for(index = 0; index < keyCount; index++)
		if(keys <= GetKey(index))
			break;
	int pageNumber = GetValue(index) >> 8;
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(context->LoadPage(pageNumber));
	page->Remove(keys, value);
	delete page;

	if(index == keyCount)
		index--;
	int leftSonPageNumber = GetValue(index) >> 8;
	int rightSonPageNumber = GetValue(index + 1) >> 8;
	BplusNodePage* leftSon = dynamic_cast<BplusNodePage*>(context->LoadPage(leftSonPageNumber));
	BplusNodePage* rightSon = dynamic_cast<BplusNodePage*>(context->LoadPage(rightSonPageNumber));
	int minKeyCount = BPLUS_TREE_RANK >> 1;
	if(leftSon->keyCount >= minKeyCount && rightSon->keyCount >= minKeyCount) {
		delete leftSon;
		delete rightSon;
		return;
	}
	if(leftSon->keyCount > minKeyCount) {
		Zig(index, leftSon, rightSon);
		delete leftSon;
		delete rightSon;
		return;
	}
	if(rightSon->keyCount > minKeyCount) {
		Zag(index, leftSon, rightSon);
		delete leftSon;
		delete rightSon;
		return;
	}
	Merge(index, leftSon, rightSon);
	delete leftSon;
	delete rightSon;
	context->FreePage(rightSonPageNumber);
}

void BplusInnerNodePage::Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer) {
	int index;
	for(index = 0; index < keyCount; index++)
		if(lowerBound <= GetKey(index))
			break;
	int pageNumber = GetValue(index) >> 8;
	BplusNodePage* page = dynamic_cast<BplusNodePage*>(context->LoadPage(pageNumber));
	page->Search(lowerBound, upperBound, gatherer);
	delete page;
}

void BplusInnerNodePage::Print(vector<Data> keys, int indent) {
	for(int index = 0; index <= keyCount; index++) {
		int pageNumber = GetValue(index) >> 8;
		BplusNodePage* page = dynamic_cast<BplusNodePage*>(context->LoadPage(pageNumber));
		page->Print(keys, indent + 1);
		delete page;
		if(index < keyCount) {
			for(int i = 0; i < indent; i++)
				cout << "----";
			cout << " " << GetKey(index) << endl;
		}
	}
}

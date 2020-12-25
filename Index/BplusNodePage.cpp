#include "BplusNodePage.h"
#include "BplusLeafNodePage.h"

BplusNodePage::BplusNodePage(FileBase* context, int pageNumber, int pageIndex, BufType b):
PageBase(pageNumber, pageIndex, b) {
	this->context = dynamic_cast<Index*>(context);
	keyCount = 0;
}
void BplusNodePage::LoadPageHeader() {
	pageType = b[0];
	keyCount = b[1];
}
void BplusNodePage::SavePageHeader() {
	b[0] = pageType;
	b[1] = keyCount;
	MarkDirty();
}
vector<Data> BplusNodePage::GetKey(int index) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	vector<Data> ret = context->keys;
	BufType _b = b + (PAGE_OFFSET + INDEX_SIZE * index >> 2);
	for(int i = 0; i < (signed)ret.size(); i++)
		ret[i].LoadData((unsigned char*)(_b + 1 + i * 2));
	return ret;
}
int BplusNodePage::GetValue(int index) {
	if(index < 0 || index > keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	return b[PAGE_OFFSET + INDEX_SIZE * index >> 2];
}
void BplusNodePage::SetKey(int index, vector<Data> key) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	if(key.size() > INDEX_MAX_KEYS) {
		cerr << "Too many keys" << endl;
		exit(-1);
	}
	BufType _b = b + (PAGE_OFFSET + INDEX_SIZE * index >> 2);
	for(int i = 0; i < (signed)key.size(); i++)
		key[i].SaveData((unsigned char*)(_b + 1 + i * 2));
	MarkDirty();
}
void BplusNodePage::SetValue(int index, int value) {
	if(index < 0 || index > keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	b[PAGE_OFFSET + INDEX_SIZE * index >> 2] = value;
	MarkDirty();
}
void BplusNodePage::InsertKeyAndValue(int index, vector<Data> key, int value) {
	if(index < 0 || index > keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	int offset = PAGE_OFFSET + INDEX_SIZE * index + 4;
	for(int i = PAGE_INT_NUM - 1 - (INDEX_SIZE >> 2); i >= (offset >> 2); i--)
		b[i + (INDEX_SIZE >> 2)] = b[i];
	keyCount++;
	SavePageHeader();
	SetKey(index, key);
	SetValue(index + 1, value);
	MarkDirty();
}
void BplusNodePage::InsertValueAndKey(int index, int value, vector<Data> key) {
	if(index < 0 || index > keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	int offset = PAGE_OFFSET + INDEX_SIZE * index;
	for(int i = PAGE_INT_NUM - 1 - (INDEX_SIZE >> 2); i >= (offset >> 2); i--)
		b[i + (INDEX_SIZE >> 2)] = b[i];
	keyCount++;
	SavePageHeader();
	SetValue(index, value);
	SetKey(index, key);
	MarkDirty();
}
void BplusNodePage::RemoveKeyAndValue(int index) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	int offset = PAGE_OFFSET + INDEX_SIZE * index + 4;
	for(int i = (offset >> 2); i <= PAGE_INT_NUM - 1 - (INDEX_SIZE >> 2); i++)
		b[i] = b[i + (INDEX_SIZE >> 2)];
	keyCount--;
	SavePageHeader();
	MarkDirty();
}
void BplusNodePage::RemoveValueAndKey(int index) {
	if(index < 0 || index >= keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	int offset = PAGE_OFFSET + INDEX_SIZE * index;
	for(int i = (offset >> 2); i <= PAGE_INT_NUM - 1 - (INDEX_SIZE >> 2); i++)
		b[i] = b[i + (INDEX_SIZE >> 2)];
	keyCount--;
	SavePageHeader();
	MarkDirty();
}
void BplusNodePage::Split(BplusNodePage *other, int index) {
	if(index < 0 || index > keyCount) {
		cerr << "Invalid index!" << endl;
		exit(-1);
	}
	int offset = PAGE_OFFSET + INDEX_SIZE * index;
	for(int i = (offset >> 2); i < PAGE_INT_NUM - 1; i++)
		other->b[i - (INDEX_SIZE * index >> 2)] = b[i];
	other->keyCount = keyCount - index;
	keyCount = index;
	if(pageType == PageBase::BPLUS_LEAF_NODE_PAGE) {
		dynamic_cast<BplusLeafNodePage*>(other)->nextPage = dynamic_cast<BplusLeafNodePage*>(this)->nextPage;
		dynamic_cast<BplusLeafNodePage*>(this)->nextPage = dynamic_cast<BplusLeafNodePage*>(other)->pageNumber;
	}
	SavePageHeader();
	other->SavePageHeader();
	MarkDirty();
	other->MarkDirty();
}

bool operator < (const vector<Data>& data1, const vector<Data>& data2) {
	if(data1.size() != data2.size()) {
		cerr << "Key size distinct!" << endl;
		exit(-1);
	}
	for(int i = 0; i < (signed)data1.size(); i++) {
		if(!(data1[i] == data2[i]))
			return data1[i] < data2[i];
	}
	return false;
}

bool operator == (const vector<Data>& data1, const vector<Data>& data2) {
	if(data1.size() != data2.size()) {
		cerr << "Key size distinct!" << endl;
		exit(-1);
	}
	for(int i = 0; i < (signed)data1.size(); i++) {
		if(!(data1[i] == data2[i]))
			return false;
	}
	return true;
}

ostream& operator << (ostream& os, const vector<Data>& data) {
	if(data.size() == 0) {
		cerr << "Nothing to output!" << endl;
		exit(-1);
	}
	os << data[0];
	for(int i = 1; i < (signed)data.size(); i++) {
		os << ", " << data[i];
	}
	return os;
}

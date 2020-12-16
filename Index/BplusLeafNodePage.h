#pragma once
#include "BplusNodePage.h"
#define PAGE_OFFSET 64
class BplusLeafNodePage: public BplusNodePage {
public:
	int nextPage;
	BplusLeafNodePage(void* context, int pageNumber, int pageIndex, BufType b);
	
	void LoadPageHeader();
	void SavePageHeader();

	void Insert(vector<Data> keys, int value,
		bool& added, vector<Data>& addedKey, int& addedValue);
	void Remove(vector<Data> keys);
	void Print(vector<Data> keys, int indent);
};

#pragma once
#include "BplusNodePage.h"
#define PAGE_OFFSET 64
class BplusLeafNodePage: public BplusNodePage {
public:
	int nextPage;
	BplusLeafNodePage(FileBase* context, int pageNumber, int pageIndex, BufType b);
	
	void LoadPageHeader();
	void SavePageHeader();

	void Insert(vector<Data> keys, unsigned int value,
		bool& added, vector<Data>& addedKey, unsigned int& addedValue);
	void Remove(vector<Data> keys, unsigned int value);
	void Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer);
	void Print(vector<Data> keys, int indent);
};

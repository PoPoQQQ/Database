#pragma once
#include "BplusNodePage.h"
#define PAGE_OFFSET 64
class BplusInnerNodePage: public BplusNodePage {
public:
	BplusInnerNodePage(FileBase* context, int pageNumber, int pageIndex, BufType b);
	
	void Zig(int index, BplusNodePage* leftSon, BplusNodePage* rightSon);
	void Zag(int index, BplusNodePage* leftSon, BplusNodePage* rightSon);
	void Merge(int index, BplusNodePage* leftSon, BplusNodePage* rightSon);

	void Insert(vector<Data> keys, int value,
		bool& added, vector<Data>& addedKey, int& addedValue);
	void Remove(vector<Data> keys);
	void Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer);
	void Print(vector<Data> keys, int indent);
};

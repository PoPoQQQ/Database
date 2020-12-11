#pragma once
#include "BPlusNodePage.h"
#define PAGE_OFFSET 64
class BplusInnerNodePage: public BplusNodePage {
public:
	BplusInnerNodePage(void* context, int pageNumber, int pageIndex, BufType b);
	
	void Zig(int index, BplusNodePage* leftSon, BplusNodePage* rightSon);
	void Zag(int index, BplusNodePage* leftSon, BplusNodePage* rightSon);
	void Merge(int index, BplusNodePage* leftSon, BplusNodePage* rightSon);

	void Insert(vector<Data> keys, int value,
		bool& added, vector<Data>& addedKey, int& addedValue);
	virtual void Remove(vector<Data> keys);
	void Print(vector<Data> keys, int indent);
};

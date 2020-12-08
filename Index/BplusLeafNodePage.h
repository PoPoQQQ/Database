#pragma once
#include "BPlusNodePage.h"
#define PAGE_OFFSET 64
class BplusLeafNodePage: public BplusNodePage {
public:
	BplusLeafNodePage(int pageIndex, BufType b): BplusNodePage(pageIndex, b) {}
	void Insert(BplusNodePage *parent, vector<Data> keys, int value) {
		
	}
};

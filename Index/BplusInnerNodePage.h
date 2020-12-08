#pragma once
#include "BPlusNodePage.h"
#define PAGE_OFFSET 64
class BplusInnerNodePage: public BplusNodePage {
public:
	BplusInnerNodePage(int pageIndex, BufType b): BplusNodePage(pageIndex, b) {}
	void Insert(BplusNodePage *parent, vector<Data> keys, int value) {

	}
};

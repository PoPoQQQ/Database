#pragma once
#include <vector>
#include "../Record/Data.h"
#include "../Record/PageBase.h"
#define PAGE_OFFSET 64
class BplusNodePage: public PageBase {
public:
	int keyCount;
	BplusNodePage(int pageIndex, BufType b): PageBase(pageIndex, b) {
		keyCount = 0;
	}
	virtual void Insert(BplusNodePage *parent, vector<Data> keys, int value) = 0;
	void LoadPageHeader() {
		pageType = b[0];
		keyCount = b[1];
	}
	void SavePageHeader() {
		b[0] = pageType;
		b[1] = keyCount;
	}
};

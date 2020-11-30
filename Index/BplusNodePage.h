#pragma once
#include "../Record/PageBase.h"
#define PAGE_OFFSET 64
class BplusNodePage: public PageBase {
public:
	virtual void Insert() = 0;
};

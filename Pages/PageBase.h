#pragma once
#include "../Utils/Global.h"
#include "../BufManager/BufPageManager.h"
class PageBase {
public:
	enum RecordType{
		UNDEFINED = 0,
		RECORD_PAGE = 1,
		BPLUS_LEAF_NODE_PAGE = 2,
		BPLUS_INNER_NODE_PAGE = 3,
		BITMAP_PAGE = 4
	};
	
	int pageType;
	int pageNumber;
	int pageIndex;
	BufType b;
	
	PageBase(int pageNumber, int pageIndex, BufType b): 
		pageNumber(pageNumber), pageIndex(pageIndex), b(b) {}
	~PageBase() { 
	}
	virtual void LoadPageHeader() = 0;
	virtual void SavePageHeader() = 0;
	void MarkDirty() {
		Global::getInstance()->bpm->markDirty(pageIndex);
	}
};

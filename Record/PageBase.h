#pragma once
#include "../BufManager/BufPageManager.h"
class PageBase {
public:
	enum RecordType{
		UNDEFINED = 0,
		RECORD_PAGE = 1,
		BPLUS_LEAF_NODE_PAGE = 2,
		BPLUS_INNER_NODE_PAGE = 3

	};
	int pageIndex;
	BufType b;

	int pageType;
	
	PageBase(int pageIndex, BufType b): pageIndex(pageIndex), b(b) {}
	virtual void LoadPageHeader() = 0;
	virtual void SavePageHeader() = 0;
};

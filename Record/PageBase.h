#pragma once
#include "../BufManager/BufPageManager.h"
class PageBase {
public:
	enum RecordType{
		UNDEFINED = 0,
		RECORD_PAGE = 1
	};
	int type;
	static PageBase *CreatePageHeader(BufType b);

	virtual void LoadPageHeader(BufType b) = 0;
	virtual void SavePageHeader(BufType b) = 0;
};

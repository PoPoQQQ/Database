#pragma once
#include "PageBase.h"
#include "RecordPage.h"

class PageFactory {
public:
	static PageBase *LoadPageHeader(BufType b) {
		int type = b[0];
		PageBase *ret = NULL;
		if(type == PageBase::RECORD_PAGE)
			ret = new RecordPage;
		ret->LoadPageHeader(b);
		return ret;
	}
};




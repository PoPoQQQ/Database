#pragma once
#include "PageBase.h"

class PageFactory {
private:
	static PageBase *AllocPage(void* context, int pageNumber, int index, BufType b, int pageType);
public:
	static PageBase *LoadPage(void* context, int fileID, int pageNumber);
	static PageBase *CreatePage(void* context, int fileID, int pageNumber, int pageType);
};




#pragma once
#include "PageBase.h"

class PageFactory {
private:
	static PageBase *AllocPage(int index, BufType b, int pageType);
public:
	static PageBase *LoadPage(int fileID, int pageNumber);
	static PageBase *CreatePage(int fileID, int pageNumber, int pageType);
};




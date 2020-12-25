#pragma once
#include "FileBase.h"
#include "PageBase.h"

class PageFactory {
private:
	static PageBase* AllocPage(FileBase* context, int pageNumber, int index, BufType b, int pageType);
public:
	static PageBase* LoadPage(FileBase* context, int fileID, int pageNumber);
	static PageBase* CreatePage(FileBase* context, int fileID, int pageNumber, int pageType);
	static void FreePage(int fileID, int pageNumber);
};




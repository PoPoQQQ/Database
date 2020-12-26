#include "BitMapPage.h"
#include "PageFactory.h"
#include "../Record/RecordPage.h"
#include "../Index/BplusLeafNodePage.h"
#include "../Index/BplusInnerNodePage.h"

PageBase* PageFactory::AllocPage(FileBase* context, int pageNumber, int index, BufType b, int pageType) {
	switch(pageType)
	{
		case PageBase::UNDEFINED:
			return NULL;
		case PageBase::RECORD_PAGE:
			return new RecordPage(context, pageNumber, index, b);
		case PageBase::BPLUS_LEAF_NODE_PAGE:
			return new BplusLeafNodePage(context, pageNumber, index, b);
		case PageBase::BPLUS_INNER_NODE_PAGE:
			return new BplusInnerNodePage(context, pageNumber, index, b);
		case PageBase::BITMAP_PAGE:
			return new BitMapPage(context, pageNumber, index, b);
		default:
			throw "Invalid page type!";
	}
	return NULL;
}

PageBase* PageFactory::LoadPage(FileBase* context, int fileID, int pageNumber) {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
	int pageType = b[0];
	PageBase* page = AllocPage(context, pageNumber, index, b, pageType);
	if(page != NULL)
		page->LoadPageHeader();
	return page;
}

PageBase* PageFactory::CreatePage(FileBase* context, int fileID, int pageNumber, int pageType) {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
	PageBase* page = AllocPage(context, pageNumber, index, b, pageType);
	if(page == NULL)
		throw "What r u doing?";
	page->SavePageHeader();
	return page;
}

void PageFactory::FreePage(int fileID, int pageNumber) {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
	b[0] = PageBase::UNDEFINED;
}

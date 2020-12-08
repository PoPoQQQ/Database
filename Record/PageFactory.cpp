#include "RecordPage.h"
#include "PageFactory.h"
#include "../Index/BPlusLeafNodePage.h"
#include "../Index/BplusInnerNodePage.h"

PageBase* PageFactory::AllocPage(int index, BufType b, int pageType) {
	switch(pageType)
	{
		case PageBase::UNDEFINED:
			return NULL;
		case PageBase::RECORD_PAGE:
			return new RecordPage(index, b);
		case PageBase::BPLUS_LEAF_NODE_PAGE:
			return new BplusLeafNodePage(index, b);
		case PageBase::BPLUS_INNER_NODE_PAGE:
			return new BplusInnerNodePage(index, b);
		default:
			cerr << "Invalid page type!" << endl;
			exit(-1);
			break;
	}
	return NULL;
}

PageBase* PageFactory::LoadPage(int fileID, int pageNumber) {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
	int pageType = b[0];
	PageBase* page = AllocPage(index, b, pageType);
	if(page != NULL)
		page->LoadPageHeader();
	return page;
}

PageBase* PageFactory::CreatePage(int fileID, int pageNumber, int pageType) {
	int index;
	BufType b = Global::getInstance()->bpm->getPage(fileID, pageNumber, index);
	PageBase* page = AllocPage(index, b, pageType);
	if(page == NULL) {
		cerr << "What r u doing?" << endl;
		exit(-1);
	}
	page->SavePageHeader();
	return page;
}

#include "FileBase.h"
#include "BitMapPage.h"
#include "PageFactory.h"
#include "../Utils/Global.h"

// 所有的文件至少有两页，第一页是 Header ，第二页是 BitMap
FileBase::FileBase(string fileDirectory, bool createFile): 
	numberOfPage(2), fileDirectory(fileDirectory), bitMapPage(-1) {
	if(createFile) {
		Global::getInstance()->fm->createFile(fileDirectory.c_str());
		Global::getInstance()->fm->openFile(fileDirectory.c_str(), fileID);
		delete PageFactory::CreatePage(this, fileID, 0, PageBase::RECORD_PAGE);
		PageBase* page = PageFactory::CreatePage(this, fileID, 1, PageBase::BITMAP_PAGE);
		bitMapPage = page->pageNumber;
		dynamic_cast<BitMapPage*>(page)->SetBit(0, 0);
		dynamic_cast<BitMapPage*>(page)->SetBit(1, 0);
		delete page;
	}
	else {
		Global::getInstance()->fm->openFile(fileDirectory.c_str(), fileID);
	}
}

FileBase::~FileBase() {
	int index = 0;
	for(int i = 0;i < numberOfPage; ++i) {
		index = Global::getInstance()->bpm->getIndex(fileID, i);
		if (index >= 0) {
			Global::getInstance()->bpm->writeBack(index);
		}
	}
	Global::getInstance()->fm->closeFile(fileID);
}

BufType FileBase::GetHeaderBufType() const {
	return Global::getInstance()->bpm->getPage(fileID, 0, const_cast<int&>(pageIndex));
}

void FileBase::MarkDirty() const {
	Global::getInstance()->bpm->markDirty(const_cast<int&>(pageIndex));
}

PageBase* FileBase::GetAvailablePage(int pageType) {
	// 查看是否还有表是否还有空间
	int pageNumber = FindLeftOne();
	if(pageNumber <= 0 || pageNumber > numberOfPage)
		throw "Bit map error in class \"Table\"!";
	// 获得（或创建）用于储存 Record 的页面
	// 此时获得的页面必然有空间储存 Record
	PageBase *page;
	if(pageNumber < numberOfPage) {
		page = PageFactory::LoadPage(this, fileID, pageNumber);
		if(page == NULL)
			page = PageFactory::CreatePage(this, fileID, pageNumber, pageType);
	}
	else {
		++numberOfPage;
		page = PageFactory::CreatePage(this, fileID, pageNumber, pageType);
		SaveHeader();
	}
	if(page->pageType != pageType)
		throw "Illegal page type!";
	return page;
}

PageBase* FileBase::LoadPage(int pageNumber) {
	if(pageNumber <= 0 || pageNumber >= numberOfPage)
		throw "Invalid page number!";
	return PageFactory::LoadPage(this, fileID, pageNumber);
}

PageBase* FileBase::CreatePage(int pageType) {
	PageBase* page = PageFactory::CreatePage(this, fileID, numberOfPage, pageType);
	++numberOfPage;
	SaveHeader();
	return page;
}

void FileBase::FreePage(int pageNumber) {
	if(pageNumber <= 0 || pageNumber >= numberOfPage)
		throw "Unable to free this page!";
	PageFactory::FreePage(fileID, pageNumber);
	SetBit(pageNumber, 1);
}

void FileBase::SetBit(int index, int bit) {
	PageBase* page = LoadPage(bitMapPage);
	if(page == NULL || page->pageType != PageBase::BITMAP_PAGE)
		throw "Invalid page type!";
	dynamic_cast<BitMapPage*>(page)->SetBit(index, bit);
	delete page;
}

int FileBase::FindLeftOne() {
	PageBase* page = LoadPage(bitMapPage);
	if(page == NULL || page->pageType != PageBase::BITMAP_PAGE)
		throw "Invalid page type!";
	int ret = dynamic_cast<BitMapPage*>(page)->FindLeftOne();
	delete page;
	return ret;
}

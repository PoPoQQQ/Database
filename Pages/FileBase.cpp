#include "FileBase.h"
#include "PageFactory.h"
#include "../Utils/Global.h"

FileBase::FileBase(string fileDirectory, bool createFile): 
	numberOfPage(1), fileDirectory(fileDirectory) {
	if(createFile) {
		Global::getInstance()->fm->createFile(fileDirectory.c_str());
		Global::getInstance()->fm->openFile(fileDirectory.c_str(), fileID);
		bitMap = new MyBitMap(PAGE_SIZE << 2, 1);
		bitMap->setBit(0, 0);
	}
	else {
		Global::getInstance()->fm->openFile(fileDirectory.c_str(), fileID);
		bitMap = NULL;
	}
}

FileBase::~FileBase() {
	Global::getInstance()->fm->closeFile(fileID);
	delete bitMap;
}

BufType FileBase::GetHeaderBufType() const {
	return Global::getInstance()->bpm->getPage(fileID, 0, const_cast<int&>(pageIndex));
}

void FileBase::MarkDirty() const {
	Global::getInstance()->bpm->markDirty(const_cast<int&>(pageIndex));
}

PageBase* FileBase::GetAvailablePage(int pageType) {
	// 查看是否还有表是否还有空间
	int pageNumber = bitMap->findLeftOne();
	if(pageNumber == -1)
		throw "File volume not enough!";
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
	if(pageNumber == 0 || pageNumber >= numberOfPage)
		throw "Unable to free this page!";
	PageFactory::FreePage(fileID, pageNumber);
	bitMap->setBit(pageNumber, 1);
}


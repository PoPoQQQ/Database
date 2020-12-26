#pragma once
#include "FileBase.h"
#include "PageBase.h"
#include "../Utils/MyBitMap.h"
#include "../Utils/Constraints.h"
#define PAGE_OFFSET 64
#define BITMAP_SIZE ((PAGE_SIZE - PAGE_OFFSET) >> 2)
class BitMapPage: public PageBase {
public:
	FileBase* context;
	int nextPage;
	BitMapPage(FileBase* context, int pageNumber, int pageIndex, BufType b);
	void LoadPageHeader();
	void SavePageHeader();
	void SetBit(int index, int bit);
	int FindLeftOne();
};

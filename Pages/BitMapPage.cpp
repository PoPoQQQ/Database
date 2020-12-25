#include "BitMapPage.h"

int LowBit(unsigned int x) {
	int ret = 0;
	while((x & 1) == 0) {
		++ret;
		x >>= 1;
	}
	return ret;
}

BitMapPage::BitMapPage(FileBase* context, int pageNumber, int pageIndex, BufType b): 
	PageBase(pageNumber, pageIndex, b), context(context), nextPage(-1) {
	pageType = BITMAP_PAGE;
	if(b[0] != PageBase::BITMAP_PAGE)
		memset(b + (PAGE_OFFSET >> 2), 0xff, BITMAP_SIZE << 2);
}

void BitMapPage::LoadPageHeader() {
	pageType = b[0];
	nextPage = b[1];
}

void BitMapPage::SavePageHeader() {
	b[0] = pageType;
	b[1] = nextPage;
	MarkDirty();
}

void BitMapPage::SetBit(int index, int bit) {
	if(index < 0)
		throw "Invalid bitmap index!";
	if(index < (BITMAP_SIZE << 5)) {
		unsigned int& bitmap = b[(PAGE_OFFSET >> 2) + (index >> 5)];
		bitmap |= 1u << (index & 31);
		if(bit == 0)
			bitmap ^= 1u << (index & 31);
	}
	else {
		if(nextPage == -1)
			throw "Invalid bitmap index!";
		PageBase* page = context->LoadPage(nextPage);
		if(page == NULL || page->pageType != PageBase::BITMAP_PAGE)
			throw "Invalid page type!";
		dynamic_cast<BitMapPage*>(page)->SetBit(index - (BITMAP_SIZE << 5), bit);
		delete page;
	}
}

int BitMapPage::FindLeftOne() {
	for(int i = 0; i < BITMAP_SIZE; i++)
		if(b[(PAGE_OFFSET >> 2) + i] != 0u)
			return (i << 5) | LowBit(b[(PAGE_OFFSET >> 2) + i]);
	if(nextPage == -1) {
		PageBase* page = context->CreatePage(PageBase::BITMAP_PAGE);
		nextPage = page->pageNumber;
		SavePageHeader();
		dynamic_cast<BitMapPage*>(page)->SetBit(0, 0);
		delete page;
	}
	PageBase* page = context->LoadPage(nextPage);
	if(page == NULL || page->pageType != PageBase::BITMAP_PAGE)
		throw "Invalid page type!";
	int ret = (BITMAP_SIZE << 5) + dynamic_cast<BitMapPage*>(page)->FindLeftOne();
	delete page;
	return ret;
}

#pragma once
#include "Record.h"
#include "PageBase.h"
#include "../Utils/Global.h"
#define PAGE_OFFSET 64
class RecordPage: public PageBase {
public:
	unsigned int bitMaps[4];
	RecordPage(int pageIndex, BufType b): PageBase(pageIndex, b) {
		pageType = RECORD_PAGE;
		for(int i = 0; i < 4; i++)
			bitMaps[i] = 0xffffffffu;
	}
	void LoadPageHeader() {
		pageType = b[0];
		for(int i = 0; i < 4; i++)
			bitMaps[i] = b[i + 1];
	}
	void SavePageHeader() {
		b[0] = pageType;
		for(int i = 0; i < 4; i++)
			b[i + 1] = bitMaps[i];
	}
	int FindLeftOne() {
		int index = -1;
		for(int i = 0; i < 4; i++)
			if(bitMaps[i] != 0)
			{
				index = i << 5 | MyBitMap::getIndex(bitMaps[i] & -bitMaps[i]);
				break;
			}
		if(index == -1) {
			cerr << "Bit map error in class \"Page\"!" << endl;
			exit(-1);
		}
		return index;
	}
	bool AddRecord(Record *record) {
		int recordSize = record->RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		
		int index = FindLeftOne();
		if(index >= recordVolume) {
			cerr << "Page volume not enough!" << endl;
			exit(-1);
		}
		bitMaps[index >> 5] ^= 1u << (index & 31);
		SavePageHeader();
		record->Save(b + (PAGE_OFFSET + index * recordSize) / 4);
		Global::getInstance()->bpm->markDirty(pageIndex);

		index = FindLeftOne();
		return index == recordVolume;
	}
	void PrintPage(Record *emptyRecord) {
		int recordSize = emptyRecord->RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;

		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0) {
				emptyRecord->Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				emptyRecord->PrintRecord();
			}
	}
};

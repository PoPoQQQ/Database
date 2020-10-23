#pragma once
#include "Record.h"
#define PAGE_OFFSET 64
class Page {
public:
	int type;
	unsigned int bitMaps[4];
	Page(): type(1) {
		for(int i = 0; i < 4; i++)
			bitMaps[i] = 0xffffffffu;
	}
	void LoadPageHeader(BufType b) {
		type = b[0];
		for(int i = 0; i < 4; i++)
			bitMaps[i] = b[i + 1];
	}
	void SavePageHeader(BufType b) {
		b[0] = type;
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
	bool AddRecord(BufType b, Record *record) {
		int recordSize = record->RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		
		int index = FindLeftOne();
		if(index >= recordVolume) {
			cerr << "Page volume not enough!" << endl;
			exit(-1);
		}
		bitMaps[index >> 5] ^= 1u << (index & 31);
		SavePageHeader(b);
		record->Save(b + (PAGE_OFFSET + index * recordSize) / 4);

		index = FindLeftOne();
		return index == recordVolume;
	}
	void PrintPage(BufType b, Record *emptyRecord) {
		int recordSize = emptyRecord->RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;

		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0) {
				emptyRecord->Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				emptyRecord->PrintRecord();
			}
	}
};

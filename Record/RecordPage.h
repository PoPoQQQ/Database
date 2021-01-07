#pragma once
#include "Table.h"
#include "Record.h"
#include "../Index/Index.h"
#include "../Pages/PageBase.h"
#define PAGE_OFFSET 64
class RecordPage: public PageBase {
public:
	Table* context;
	unsigned int bitMaps[4];
	RecordPage(FileBase* context, int pageNumber, int pageIndex, BufType b): 
		PageBase(pageNumber, pageIndex, b) {
		this->context = dynamic_cast<Table*>(context);
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
		MarkDirty();
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
			cerr << "Bit map error in class \"RecordPage\"!" << endl;
			exit(-1);
		}
		return index;
	}
	bool AddRecord(const Record& record, unsigned int& recordIndex) {
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		
		int index = FindLeftOne();
		if(index >= recordVolume) {
			cerr << "Page volume not enough!" << endl;
			exit(-1);
		}
		recordIndex = index;
		bitMaps[index >> 5] ^= 1u << (index & 31);
		SavePageHeader();
		record.Save(b + (PAGE_OFFSET + index * recordSize) / 4);
		MarkDirty();

		index = FindLeftOne();
		return index == recordVolume;
	}
	void InsertPageIntoIndex(Index* index, const vector<int>& columnIndexes) {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int _index = 0; _index < recordVolume; _index++)
			if((bitMaps[_index >> 5] & (1u << (_index & 31))) == 0) {
				record.Load(b + (PAGE_OFFSET + _index * recordSize) / 4);
				context->InsertRecordIntoIndex(index, columnIndexes, record, pageNumber << 8 | _index);
			}
	}
	void PrintPage() {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0) {
				record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				record.PrintRecord();
			}
	}
	void IterPage() {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0)
				record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				// 逐个对 record 进行处理
	}
};

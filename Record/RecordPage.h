#pragma once
#include <functional>
#include "Table.h"
#include "Record.h"
#include "../Index/Index.h"
#include "../Pages/PageBase.h"
#include "../Index/WhereCondition.h"
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
	bool AddRecord(Record& record, unsigned int& index) {
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		
		index = FindLeftOne();
		if(index >= recordVolume)
			throw "Page volume not enough!";
		bitMaps[index >> 5] ^= (1u << (index & 31));
		SavePageHeader();
		record.enabled = 1;
		record.Save(b + (PAGE_OFFSET + index * recordSize) / 4);
		MarkDirty();

		return FindLeftOne() == recordVolume;
	}
	Record GetRecord(unsigned int index) {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;

		if(index >= recordVolume)
			throw "Page volume not enough!";
		if(bitMaps[index >> 5] & (1u << (index & 31)))
			throw "Record does not exist!";
		record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
		return record;
	}
	bool DeleteRecord(Record& record, unsigned int index) {
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;

		if(index >= recordVolume)
			throw "Page volume not enough!";
		if(bitMaps[index >> 5] & (1u << (index & 31)))
			throw "Record does not exist!";
		record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
		record.enabled = 0;
		record.Save(b + (PAGE_OFFSET + index * recordSize) / 4);
		MarkDirty();

		bool ret = (FindLeftOne() == recordVolume);
		bitMaps[index >> 5] |= (1u << (index & 31));
		SavePageHeader();
		return ret;
	}
	vector<unsigned int> GetRecordList(WhereCondition& whereCondition) {
		vector<unsigned int> recordList;
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0) {
				record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				if(whereCondition.check(record))
					recordList.push_back(pageNumber << 8 | index);
			}
		return recordList;
	}
	void InsertPageIntoIndex(Index* index) {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int _index = 0; _index < recordVolume; _index++)
			if((bitMaps[_index >> 5] & (1u << (_index & 31))) == 0) {
				record.Load(b + (PAGE_OFFSET + _index * recordSize) / 4);
				context->InsertRecordIntoIndex(index, record, pageNumber << 8 | _index);
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
	void IterPage(function<void(Record&, BufType)> iterFunc) {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0) {
				record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				// 逐个对 record 进行处理
				iterFunc(record, b + (PAGE_OFFSET + index * recordSize) / 4);
			}
	}
	bool CheckPrimaryKey(Record& record, Index* index) {
		vector<Data> datas;
		for(int i = 0; i < (signed)index->colNames.size(); i++) {
			Data data = record.fieldList.GetColumn(record.fieldList.GetColumnIndex(index->colNames[i])).GetData();
			if((data.dataType & 0xff) != Data::VARCHAR)
				datas.push_back(data);
			else
				datas.push_back(HashData(data));
		}
		vector<unsigned int> gatherer;
		index->Search(datas, datas, gatherer);
		if(gatherer.size() > 1)
			return true;
		return false;
	}
	bool CheckPrimaryKey(Index *_index) {
		Record record = context->EmptyRecord();
		int recordSize = record.RecordSize();
		int recordVolume = (PAGE_SIZE - PAGE_OFFSET) / recordSize;
		for(int index = 0; index < recordVolume; index++)
			if((bitMaps[index >> 5] & (1u << (index & 31))) == 0) {
				record.Load(b + (PAGE_OFFSET + index * recordSize) / 4);
				// 逐个对 record 进行处理
				if(CheckPrimaryKey(record, _index))
					return true;
			}
		return false;
	}
};

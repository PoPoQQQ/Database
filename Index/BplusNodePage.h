#pragma once
#include <vector>
#include <iostream>
#include "Index.h"
#include "../Record/Data.h"
#include "../Pages/PageBase.h"
#define PAGE_OFFSET 64
#define INDEX_SIZE 36
using namespace std;
class BplusNodePage: public PageBase {
public:
	Index* context;
	int keyCount;
	BplusNodePage(FileBase* context, int pageNumber, int pageIndex, BufType b);

	void LoadPageHeader();
	void SavePageHeader();
	
	vector<Data> GetKey(int index);
	unsigned int GetValue(int index);
	void SetKey(int index, vector<Data> key);
	void SetValue(int index, unsigned int value);
	void InsertKeyAndValue(int index, vector<Data> key, unsigned int value);
	void InsertValueAndKey(int index, unsigned int value, vector<Data> key);
	void RemoveKeyAndValue(int index);
	void RemoveValueAndKey(int index);
	void Split(BplusNodePage* other, int index);

	virtual void Insert(vector<Data> keys, unsigned int value,
		bool& added, vector<Data>& addedKey, unsigned int& addedValue) = 0;
	virtual void Remove(vector<Data> keys, unsigned int value) = 0;
	virtual void Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer) = 0;
	virtual void Print(vector<Data> keys, int indent) = 0;
};

bool operator < (const vector<Data>& data1, const vector<Data>& data2);
bool operator == (const vector<Data>& data1, const vector<Data>& data2);
ostream& operator << (ostream& os, const vector<Data>& data);

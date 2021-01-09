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
	int GetValue(int index);
	void SetKey(int index, vector<Data> key);
	void SetValue(int index, int value);
	void InsertKeyAndValue(int index, vector<Data> key, int value);
	void InsertValueAndKey(int index, int value, vector<Data> key);
	void RemoveKeyAndValue(int index);
	void RemoveValueAndKey(int index);
	void Split(BplusNodePage* other, int index);

	virtual void Insert(vector<Data> keys, int value,
		bool& added, vector<Data>& addedKey, int& addedValue) = 0;
	virtual void Remove(vector<Data> keys) = 0;
	virtual void Search(vector<Data> lowerBound, vector<Data> upperBound, vector<unsigned int>& gatherer) = 0;
	virtual void Print(vector<Data> keys, int indent) = 0;
};

bool operator < (const vector<Data>& data1, const vector<Data>& data2);
bool operator == (const vector<Data>& data1, const vector<Data>& data2);
ostream& operator << (ostream& os, const vector<Data>& data);

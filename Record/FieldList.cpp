#include <cstring>
#include "FieldList.h"
#include "../Utils/Constraints.h"

void FieldList::LoadFields(BufType b) {
	unsigned int size = b[0];
	b += 1;

	fields.resize(size);
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++) {
		it->Load(b);
		b += it->FieldSize() >> 2;
	}
}
void FieldList::SaveFields(BufType b) const {
	b[0] = fields.size();
	b += 1;

	for(vector<Field>::const_iterator it = fields.begin(); it != fields.end(); it++) {
		it->Save(b);
		b += it->FieldSize() >> 2;
	}
}
void FieldList::LoadDatas(unsigned char* b) {
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++) {
		it->LoadData(b);
		b += it->DataSize();
	}
}
void FieldList::SaveDatas(unsigned char* b) const{
	for(vector<Field>::const_iterator it = fields.begin(); it != fields.end(); it++) {
		it->SaveData(b);
		b += it->DataSize();
	}
}

void FieldList::AddField(const Field& field) {
	if(FieldCount() >= MAX_COL_NUM){
		printf("MAX_COL_NUM = %d, FieldCount = %d", MAX_COL_NUM, FieldCount());
		throw "Too many fields!";
	}
	if(GetColumnIndex(field.columnName) >= 0) {
		cerr << field.columnName << endl;
		throw "Field already exists!";
	}
	fields.push_back(field);
}
void FieldList::PrintFields() {
	for(vector<Field>::iterator it = fields.begin(); it != fields.end(); it++)
		cout << " | " << it->columnName;
	cout << " | " << endl;
}
void FieldList::PrintDatas(unsigned int bitMap) {
	for(int index = 0; index < (signed)fields.size(); index++) {
		if((bitMap & (1u << index)) == 0)
			cout << " | NULL";
		else
			cout << " | " << fields[index].data;
	}
	cout << " | " << endl;
}

int FieldList::FieldCount() const {
	return fields.size();
}
int FieldList::RoundedDataSize() const {
	int dataSize = 0;
	for(vector<Field>::const_iterator it = fields.begin(); it != fields.end(); it++)
		dataSize += it->DataSize();
	return ((dataSize | 3) ^ 3) + ((dataSize & 3) ? 4 : 0);
}
Field& FieldList::GetColumn(int index) {
	if(index < 0 || index >= FieldCount())
		throw "Invalid index!";
	return fields[index];
}
int FieldList::GetColumnIndex(const char* columnName) {
	for(int i = 0; i < (signed)fields.size(); i++)
		if(strcmp(fields[i].columnName, columnName) == 0)
			return i;
	return -1;
}

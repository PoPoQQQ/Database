#include "Field.h"
using namespace std;

Field::Field() {
	memset(columnName, 0, sizeof columnName);
}
Field::Field(const char *columnName) {
	if(strlen(columnName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	strcpy(this->columnName, columnName);
}

Data Field::GetData() {
	return data;
}
void Field::SetData(Data data) {
	if(this->data.dataType != data.dataType)
		throw "Invalid data type!";
	this->data = data;
}
Field Field::SetDataType(Data data) {
	this->data = data;
	return *this;
}
Field Field::SetNotNull() {
	constraints |= 1;
	return *this;
}
Field Field::SetDefault(Data data) {
	constraints |= 2;
	if(this->data.dataType != data.dataType)
		throw "Invalid data type!";
	this->data = data;
	return *this;
}
Field Field::SetPrimaryKey(const char* primaryKeyName) {
	if(strlen(primaryKeyName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	constraints |= 4;
	strcpy(this->primaryKeyName, primaryKeyName);
	return *this;
}
Field Field::SetForeignKey(const char *foreignKeyTable, const char *foreignKeyColumn, const char *foreignKeyName) {
	if(strlen(foreignKeyTable) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(strlen(foreignKeyColumn) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	strcpy(this->foreignKeyTable, foreignKeyTable);
	strcpy(this->foreignKeyColumn, foreignKeyColumn);
	strcpy(this->foreignKeyName, foreignKeyName);
	return *this;
}

int Field::DataSize() {
	return data.dataSize;
}
int Field::RoundedDataSize() {
	return ((DataSize() | 3) ^ 3) + ((DataSize() & 3) ? 4 : 0);
}
int Field::FieldSize() {
	int size = MAX_IDENTIFIER_LEN + 12;
	if(constraints & 2)
		size += RoundedDataSize();
	if(constraints & 8)
		size += MAX_IDENTIFIER_LEN * 2;
	return size;
}
void Field::Load(BufType b) {
	memcpy(columnName, b, MAX_IDENTIFIER_LEN);
	b += MAX_IDENTIFIER_LEN >> 2;

	data.LoadType(b);
	b += 2;

	constraints = b[0];
	b += 1;

	if(constraints & 2) {
		data.LoadData((unsigned char*)b);
		b += RoundedDataSize() >> 2;
	}

	if(constraints & 3) {
		memcpy(primaryKeyName, b, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;
	}

	if(constraints & 4) {
		memcpy(foreignKeyTable, b, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;

		memcpy(foreignKeyColumn, b, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;

		memcpy(foreignKeyName, b, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;
	}
}
void Field::Save(BufType b) {
	memcpy(b, columnName, MAX_IDENTIFIER_LEN);
	b += MAX_IDENTIFIER_LEN >> 2;

	data.SaveType(b);
	b += 2;

	b[0] = constraints;
	b += 1;

	if(constraints & 2) {
		data.SaveData((unsigned char*)b);
		b += RoundedDataSize() >> 2;
	}

	if(constraints & 3) {
		memcpy(b, primaryKeyName, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;
	}

	if(constraints & 4) {
		memcpy(b, foreignKeyTable, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;

		memcpy(b, foreignKeyColumn, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;

		memcpy(b, foreignKeyName, MAX_IDENTIFIER_LEN);
		b += MAX_IDENTIFIER_LEN >> 2;
	}
}
void Field::LoadData(unsigned char* b) {
	data.LoadData(b);
}
void Field::SaveData(unsigned char* b) {
	data.SaveData(b);
}

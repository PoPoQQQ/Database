#include "Field.h"
using namespace std;

Field::Field() {}
Field::Field(string columnName): columnName(columnName) {
	if(columnName.length() > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
}

Data Field::GetData() {
	return data;
}
void Field::SetData(const Data& data) {
	this->data.SetData(data);
}
Field Field::SetDataType(Data data) {
	this->data = data;
	return *this;
}
Field Field::SetNotNull() {
	constraints |= NOT_NULL;
	return *this;
}
Field Field::SetDefault(Data data) {
	constraints |= DEFAULT;
	SetData(data);
	return *this;
}
Field Field::SetPrimaryKey() {
	constraints |= PRIMARY_KEY;
	constraints |= NOT_NULL;
	return *this;
}
Field Field::SetForeignKey() {
	constraints |= FOREIGN_KEY;
	return *this;
}

int Field::DataSize() const {
	return data.dataSize;
}
int Field::RoundedDataSize() const {
	return ((DataSize() | 3) ^ 3) + ((DataSize() & 3) ? 4 : 0);
}
int Field::FieldSize() const {
	int size = MAX_IDENTIFIER_LEN + 12;
	if(constraints & 2)
		size += RoundedDataSize();
	return size;
}
void Field::Load(BufType b) {
	char buffer[MAX_IDENTIFIER_LEN + 1];
    memcpy(buffer, b, MAX_IDENTIFIER_LEN);
    buffer[MAX_IDENTIFIER_LEN] = 0;
    columnName = string(buffer);
    b += MAX_IDENTIFIER_LEN >> 2;

	data.LoadType(b);
	b += 2;

	constraints = b[0];
	b += 1;

	if(constraints & 2) {
		data.LoadData((unsigned char*)b);
		b += RoundedDataSize() >> 2;
	}
}
void Field::Save(BufType b) const{
	memcpy(b, columnName.c_str(), min((signed)columnName.length() + 1, MAX_IDENTIFIER_LEN));
	b += MAX_IDENTIFIER_LEN >> 2;

	data.SaveType(b);
	b += 2;

	b[0] = constraints;
	b += 1;

	if(constraints & 2) {
		data.SaveData((unsigned char*)b);
		b += RoundedDataSize() >> 2;
	}
}
void Field::LoadData(unsigned char* b) {
	data.LoadData(b);
}
void Field::SaveData(unsigned char* b) const {
	data.SaveData(b);
}
/*
string Field::toString() const {
	char buf[256];
	snprintf(buf, sizeof(buf), "Field{colName: %s, type: %d, constraints: %d }", columnName, data.dataType, constraints);
	return string(buf);
}

bool Field::validateData(const Data& data) const {
	if(data.dataType == Data::DataType::UNDEFINED) {
		// 如果该空非空，但是给了空值，则是不正确的
		if(this->constraints & NOT_NULL)
			return false;
		else
			return true;
	} else {
		// 如果 data 类型不相同则肯定不相同
		if((data.dataType & 0xff) != ((this->data).dataType & 0xff)) {
			return false;
		}
		// 如果 data 是 varchar，但是 data 的长度超过了也不正确
		if(data.dataType & 0xff == Data::DataType::VARCHAR &&
			strlen(data.stringData) > this->data.dataSize) {
			return false;
		}
		//TODO: 如果该 field 存在某种 index ，需要调用对应的 index 来确认
		// this->index.check(data);
		return true;
	}
}*/

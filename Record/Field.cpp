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
Field Field::SetPrimaryKey(const char* primaryKeyName) {
	if(strlen(primaryKeyName) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	constraints |= NOT_NULL | PRIMARY_KEY;
	strcpy(this->primaryKeyName, primaryKeyName);
	return *this;
}
Field Field::SetForeignKey(const char *foreignKeyTable, const char *foreignKeyColumn, const char *foreignKeyName) {
	if(strlen(foreignKeyTable) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	if(strlen(foreignKeyColumn) > MAX_IDENTIFIER_LEN)
		throw "Identifier is too long!";
	constraints |= FOREIGN_KEY;
	strcpy(this->foreignKeyTable, foreignKeyTable);
	strcpy(this->foreignKeyColumn, foreignKeyColumn);
	strcpy(this->foreignKeyName, foreignKeyName);
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
void Field::Save(BufType b) const{
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
void Field::SaveData(unsigned char* b) const {
	data.SaveData(b);
}

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
}
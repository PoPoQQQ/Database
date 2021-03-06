#include <cstring>
#include <algorithm>
#include "Data.h"

void CheckDate(int year, int month, int day) {
	if(year < 1 || year > 20000)
		throw "Invalid year!";
	if(month < 1 || month > 12)
		throw "Invalid month!";
	if(day < 1 || day > 31)
		throw "Invalid day!";
	bool leap;
	if(year % 400 == 0)
		leap = true;
	else if(year % 100 == 0)
		leap = false;
	else if(year % 4 == 0)
		leap = true;
	else
		leap = false;
	int dayOfMonth;
	if(month == 1 || month == 3 || month == 5 || month == 7 || month == 8 || month == 10 || month == 12)
		dayOfMonth = 31;
	else if(month == 4 || month == 6 || month == 9 || month == 11)
		dayOfMonth = 30;
	else
		dayOfMonth = 28 + (leap ? 1 : 0);
	if(day > dayOfMonth)
		throw "Invalid day!";
}

int GetDigit(unsigned int number) {
	int ret = 0;
	while(number > 0) {
		++ret;
		number /= 10;
	}
	return ret;
}

Data::Data(): 
	dataType(UNDEFINED), dataSize(0), intData(0), floatData(0), stringData("") {}

Data::Data(DataType dataType, int para) {
	this->dataType = dataType;
	intData = 0;
	floatData = 0;
	stringData = string("");
	switch(dataType) {
		case UNDEFINED:
			dataSize = 0;
			break;
		case INT:
			if(para < 1 || para > 255) {
				cerr << "Parameter invalid" << endl;
				exit(-1);
			}
			this->dataType = dataType | (para << 8);
			dataSize = 4;
			break;
		case VARCHAR:
			if(para < 1 || para > 255) {
				cerr << "Parameter invalid!" << endl;
				exit(-1);
			}
			this->dataType = dataType | (para << 8);
			dataSize = para;
			break;
		case DATE:
			dataSize = 3;
			break;
		case FLOAT:
			dataSize = 4;
			break;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
}

Data::~Data() {}

Data::Data(const Data& data) {
	dataType = data.dataType;
	dataSize = data.dataSize;
	intData = data.intData;
	floatData = data.floatData;
	stringData = data.stringData;
}

Data& Data::operator = (const Data& data) {
	dataType = data.dataType;
	dataSize = data.dataSize;
	intData = data.intData;
	floatData = data.floatData;
	stringData = data.stringData;
	return *this;
}

Data& Data::SetData(unsigned int data) {
	if((dataType & 0xff) == INT) {
		if(GetDigit(data) > (dataType >> 8))
			throw "Integer is too big!";
		intData = data;
	}
	else if((dataType & 0xff) == FLOAT)
		floatData = data;
	else
		throw "Invalid data type!";
	return *this;
}

Data& Data::SetData(float data) {
	if((dataType & 0xff) == FLOAT)
		floatData = data;
	else
		throw "Invalid data type!";
	return *this;
}

Data& Data::SetData(string data) {
	if((dataType & 0xff) == VARCHAR) {
		if(data.length() > dataSize) {
			cerr << data << endl;
			throw "String is too long!";
		}
		stringData = data;
	}
	else if((dataType & 0xff) == DATE) {
		int year, month, day;
		int res = sscanf(data.c_str(), "%u/%u/%u", &year, &month, &day);
		if(res < 3)
			res = sscanf(data.c_str(), "%u-%u-%u", &year, &month, &day);
		if(res < 3)
			throw "Invalid date format!";
		CheckDate(year, month, day);
		intData = (year * 13 + month) * 32 + day;
	}
	else
		throw "Invalid data type!";
	return *this;
}

Data& Data::SetData(const Data &data) {
	if((dataType & 0xff) == INT) {
		if((data.dataType & 0xff) == INT)
			SetData(data.intData);
		else if((data.dataType & 0xff) == FLOAT)
			SetData((unsigned int)data.floatData);
		else
			throw "Invalid data type!";
	}
	else if((dataType & 0xff) == VARCHAR) {
		if((data.dataType & 0xff) != VARCHAR)
			throw "Invalid data type!";
		SetData(data.stringData);
	}
	else if((dataType & 0xff) == DATE) {
		if((data.dataType & 0xff) == DATE)
			*this = data;
		else if((data.dataType & 0xff) == VARCHAR)
			SetData(data.stringData);
		else
			throw "Invalid data type!";
		
	}
	else if((dataType & 0xff) == FLOAT) {
		if((data.dataType & 0xff) == FLOAT)
			SetData(data.floatData);
		else if((data.dataType & 0xff) == INT)
			SetData((float)data.intData);
		else
			throw "Invalid data type!";
	}
	else
		throw "Invalid data type!";
	return *this;
}

Data& Data::SetNull() {
	return SetPosInf();
}

Data& Data::SetNegInf() {
	switch(dataType & 0xff) {
		case INT:
			intData = 0u;
			break;
		case VARCHAR:
			throw "Invalid data type!";
		case DATE:
			intData = 0u;
			break;
		case FLOAT:
			floatData = -1e20f;
			break;
		default:
			throw "Invalid data type!";
	}
	return *this;
}

Data& Data::SetPosInf() {
	switch(dataType & 0xff) {
		case INT:
			intData = 0xffffffffu;
			break;
		case VARCHAR:
			throw "Invalid data type!";
		case DATE:
			intData = 0x7fffffu;
			break;
		case FLOAT:
			floatData = 1e20f;
			break;
		default:
			throw "Invalid data type!";
	}
	return *this;
}

void Data::LoadType(unsigned int* b) {
	dataType = b[0];
	dataSize = b[1];
}

void Data::SaveType(unsigned int* b) const {
	b[0] = dataType;
	b[1] = dataSize;
}

void Data::LoadData(unsigned char*  b) {
	switch(dataType & 0xff) {
		case INT:
			memcpy(&intData, b, dataSize);
			break;
		case VARCHAR:{
			char buffer[dataSize + 1];
			buffer[dataSize] = 0;
			memcpy(buffer, b, dataSize);
			stringData = string(buffer);
			break;
		}
		case DATE:
			intData = 0;
			memcpy(&intData, b, dataSize);
			break;
		case FLOAT:
			memcpy(&floatData, b, dataSize);
			break;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
}

void Data::SaveData(unsigned char* b) const {
	switch(dataType & 0xff) {
		case INT:
			memcpy(b, &intData, dataSize);
			break;
		case VARCHAR:
			memcpy(b, stringData.c_str(), min((signed)stringData.length() + 1, dataSize));
			break;
		case DATE:
			memcpy(b, &intData, dataSize);
			break;
		case FLOAT:
			memcpy(b, &floatData, dataSize);
			break;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
}

ostream& operator << (ostream& os, const Data &data) {
	switch(data.dataType & 0xff) {
		case Data::INT:
			return os << data.intData;
		case Data::VARCHAR:
			return os << data.stringData;
		case Data::DATE:
			return os << data.intData / 32 / 13 << '/' << data.intData / 32 % 13 << '/' << data.intData % 32;
		case Data::FLOAT:
			return os << data.floatData;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
}

bool operator < (const Data &data1, const Data &data2) {
	if((data1.dataType & 0xff) != (data2.dataType & 0xff)) {
		cout << data1.dataType << "---" << data2.dataType << endl;
		throw "Data type distincts!";
	}
	switch(data1.dataType & 0xff) {
		case Data::INT:
			return data1.intData < data2.intData;
		case Data::VARCHAR:
			return data1.stringData < data2.stringData;
		case Data::DATE:
			return data1.intData < data2.intData;
		case Data::FLOAT:
			return data1.floatData < data2.floatData;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
	return false;
}

bool operator == (const Data &data1, const Data &data2) {
	if((data1.dataType & 0xff) != (data2.dataType & 0xff)) {
		cout << data1.dataType << "---" << data2.dataType << endl;
		throw "Data type distincts!";
	}
	switch(data1.dataType & 0xff) {
		case Data::INT:
			return data1.intData == data2.intData;
		case Data::VARCHAR:
			return data1.stringData == data2.stringData;
		case Data::DATE:
			return data1.intData == data2.intData;
		case Data::FLOAT:
			return data1.floatData == data2.floatData;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
	return false;
}

Data HashData(const Data &data) {
	if(data.dataType & 0xff != Data::VARCHAR)
		throw "Hashed data must be string!";
	unsigned int hash = 0;
	const unsigned int base = 131;
	int len = data.stringData.length();
	for(int i = 0; i < len; i++) {
		unsigned int v = (unsigned char)data.stringData[i];
		hash = hash * base + len;
	}
	Data ret(Data::INT);
	ret.SetData(hash);
	return ret;
}

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
	dataType(UNDEFINED), dataSize(0), stringData(NULL) {}

Data::Data(DataType dataType, int para) {
	this->dataType = dataType;
	stringData = NULL;
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
			stringData = new char[dataSize + 1];
			memset(stringData, 0, dataSize + 1);
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

Data::~Data() {
	if((dataType & 0xff) == VARCHAR)
		delete stringData;
}

Data::Data(const Data& data) {
	*this = data;
}

Data& Data::operator = (const Data& data) {
	if(dataType == VARCHAR)
		delete stringData;
	dataType = data.dataType;
	dataSize = data.dataSize;

	switch(data.dataType & 0xff) {
		case UNDEFINED:
			stringData = NULL;
			break;
		case INT:
			intData = data.intData;
			break;
		case VARCHAR: {
			if(data.stringData == NULL) {
				cerr << "Data class error!" << endl;
				exit(-1);
			}
			stringData = new char[dataSize + 1];
			memcpy(stringData, data.stringData, dataSize + 1);
			break;
		}
		case DATE:
			intData = data.intData;
			break;
		case FLOAT:
			floatData = data.floatData;
			break;
		default:
			cerr << "Data type error!" << endl;
			exit(-1);
			break;
	}
	return *this;
}

Data Data::SetData(unsigned int data) {
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

Data Data::SetData(float data) {
	if((dataType & 0xff) == FLOAT)
		floatData = data;
	else
		throw "Invalid data type!";
	return *this;
}

Data Data::SetData(const char *data) {
	if((dataType & 0xff) == VARCHAR) {
		if(strlen(data) > dataSize)
			throw "String is too long!";
		strcpy(stringData, data);
	}
	else if((dataType & 0xff) == DATE) {
		int year, month, day;
		int res = sscanf(data, "%u/%u/%u", &year, &month, &day);
		if(res < 3)
			throw "Invalid date format!";
		CheckDate(year, month, day);
		intData = (year * 13 + month) * 32 + day;
	}
	else
		throw "Invalid data type!";
	return *this;
}

void Data::LoadType(unsigned int* b) {
	if((dataType & 0xff) == VARCHAR) {
		delete stringData;
		stringData = NULL;
	}
	dataType = b[0];
	dataSize = b[1];
	if((dataType & 0xff) == VARCHAR) {
		stringData = new char[dataSize + 1];
		memset(stringData, 0, dataSize + 1);
	}
}

void Data::SaveType(unsigned int* b) {
	b[0] = dataType;
	b[1] = dataSize;
}

void Data::LoadData(unsigned char*  b) {
	switch(dataType & 0xff) {
		case INT:
			memcpy(&intData, b, dataSize);
			break;
		case VARCHAR:
			memset(stringData, 0, dataSize + 1);
			memcpy(stringData, b, dataSize);
			break;
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

void Data::SaveData(unsigned char*  b) {
	switch(dataType & 0xff) {
		case INT:
			memcpy(b, &intData, dataSize);
			break;
		case VARCHAR:
			memcpy(b, stringData, dataSize);
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
	if(data1.dataType != data2.dataType) {
		cerr << "Data type distincts!" << endl;
		exit(-1);
	}
	switch(data1.dataType & 0xff) {
		case Data::INT:
			return data1.intData < data2.intData;
		case Data::VARCHAR:
			return strcmp(data1.stringData, data2.stringData) < 0;
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
	if(data1.dataType != data2.dataType) {
		cerr << "Data type distincts!" << endl;
		exit(-1);
	}
	switch(data1.dataType & 0xff) {
		case Data::INT:
			return data1.intData == data2.intData;
		case Data::VARCHAR:
			return strcmp(data1.stringData, data2.stringData) == 0;
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

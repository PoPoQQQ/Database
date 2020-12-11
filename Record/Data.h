#pragma once
#include <cstring>
#include <iostream>
#include <algorithm>
using namespace std;
class Data {
public:
	enum DataType {
		UNDEFINED = 0,

		TINYINT = 1,
		SMALLINT = 2,
		MEDIUMINT = 3,
		INTEGER = 4,
		BIGINT = 5,
		FLOAT = 6,
		DOUBLE = 7,
		DECIMAL = 8,
		
		DATE = 9,
		TIME = 10,
		YEAR = 11,
		DATETIME = 12,
		TIMESTAMP = 13,

		CHAR = 14,
		VARCHAR = 15,
		TINYBLOB = 16,
		TINYTEXT = 17,
		BLOB = 18,
		TEXT = 19,
		MEDIUMBLOB = 20,
		MEDIUMTEXT = 21,
		LONGBLOB = 22,
		LONGTEXT = 23
	};

	int dataType;
	int dataSize;
	union {
		int intData;
		long long longlongData;
		float floatData;
		double doubleData;
		char* charData;
	};
	
	Data(): dataType(UNDEFINED), dataSize(0), longlongData(0) {}

	Data(DataType dataType, int para1 = 10, int para2 = 0) {
		this->dataType = dataType;
		longlongData = 0;
		switch(dataType) {
			case TINYINT:
				dataSize = 1;
				break;
			case SMALLINT:
				dataSize = 2;
				break;
			case MEDIUMINT:
				dataSize = 3;
				break;
			case INTEGER:
				dataSize = 4;
				break;
			case BIGINT:
				dataSize = 8;
				break;
			case FLOAT:
				dataSize = 4;
				break;
			case DOUBLE:
				dataSize = 8;
				break;
			case DECIMAL:
				if(para1 < 1 || para1 > 65 || para2 < 0 || para2 > 30 || para2 > para1) {
					cerr << "Parameter invalid!" << endl;
					exit(-1);
				}
				this->dataType = dataType | (para1 << 8) | (para2 << 16);
				dataSize = para1 + 2;
				break;
			case DATE:
				dataSize = 3;
				break;
			case TIME:
				dataSize = 3;
				break;
			case YEAR:
				dataSize = 1;
				break;
			case DATETIME:
				dataSize = 8;
				break;
			case TIMESTAMP:
				dataSize = 4;
				break;
			case CHAR:
				if(para1 < 1 || para1 > 255) {
					cerr << "Parameter invalid!" << endl;
					exit(-1);
				}
				this->dataType = dataType | (para1 << 8);
				dataSize = para1;
				charData = new char[dataSize];
				break;
			case VARCHAR:
				if(para1 < 1 || para1 > 65535) {
					cerr << "Parameter invalid!" << endl;
					exit(-1);
				}
				this->dataType = dataType | (para1 << 8);
				dataSize = para1;
				charData = new char[dataSize];
				break;
			default:
				cerr << "Data type not supported!" << endl;
				exit(-1);
				break;
		}
	}

	Data(const Data& data) {
		*this = data;
	}

	Data& operator = (const Data& data) {
		this->dataType = data.dataType;
		this->dataSize = data.dataSize;

		switch(data.dataType & 0xff) {
			case INTEGER:
				this->intData = data.intData;
				break;
			case BIGINT:
				this->longlongData = data.longlongData;
				break;
			case FLOAT:
				this->floatData = data.floatData;
				break;
			case DOUBLE:
				this->doubleData = data.doubleData;
				break;
			case CHAR:
			case VARCHAR: {
				if(this->charData != NULL) {
					delete this->charData;
				}
				this->charData = new char[this->dataSize];
				memcpy(this->charData, data.charData, this->dataSize);
				break;
			}
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}

		return *this;
	}

	Data SetData(int data) {
		intData = data;
		return *this;
	}

	Data SetData(long long data) {
		longlongData = data;
		return *this;
	}

	Data SetData(float data) {
		floatData = data;
		return *this;
	}

	Data SetData(double data) {
		doubleData = data;
		return *this;
	}

	Data SetData(const char *data) {
		charData = new char[dataSize];
		memset(charData, 0, dataSize);
		memcpy(charData, data, min((size_t)dataSize, strlen(data)));
		return *this;
	}

	void LoadType(unsigned int* b) {
		dataType = b[0];
		dataSize = b[1];
	}

	void SaveType(unsigned int* b) {
		b[0] = dataType;
		b[1] = dataSize;
	}

	void LoadData(unsigned char*  b) {
		switch(dataType & 0xff) {
			case INTEGER:
				memcpy(&intData, b, dataSize);
				break;
			case BIGINT:
				memcpy(&longlongData, b, dataSize);
				break;
			case FLOAT:
				memcpy(&floatData, b, dataSize);
				break;
			case DOUBLE:
				memcpy(&doubleData, b, dataSize);
				break;
			case CHAR:
			case VARCHAR:
				if(charData == NULL)
					charData = new char[dataSize];
				memcpy(charData, b, dataSize);
				break;
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}
	}

	void SaveData(unsigned char*  b) {
		switch(dataType & 0xff) {
			case INTEGER:
				memcpy(b, &intData, dataSize);
				break;
			case BIGINT:
				memcpy(b, &longlongData, dataSize);
				break;
			case FLOAT:
				memcpy(b, &floatData, dataSize);
				break;
			case DOUBLE:
				memcpy(b, &doubleData, dataSize);
				break;
			case CHAR:
			case VARCHAR:
				memcpy(b, charData, dataSize);
				break;
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}
	}

	friend ostream& operator << (ostream& os, const Data &data) {
		switch(data.dataType & 0xff) {
			case INTEGER:
				return os << data.intData;
				break;
			case BIGINT:
				return os << data.longlongData;
				break;
			case FLOAT:
				return os << data.floatData;
				break;
			case DOUBLE:
				return os << data.doubleData;
				break;
			case CHAR: {
				char buffer[data.dataSize + 1];
				memcpy(buffer, data.charData, data.dataSize);
				buffer[data.dataSize] = 0;
				return os << buffer;
				break;
			}
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}
	}

	friend bool operator < (const Data &data1, const Data &data2) {
		if(data1.dataType != data2.dataType) {
			cerr << "Data type distincts!" << endl;
			exit(-1);
		}
		switch(data1.dataType) {
			case INTEGER:
				return data1.intData < data2.intData;
			case BIGINT:
				return data1.longlongData < data2.longlongData;
			case FLOAT:
				return data1.floatData < data2.floatData;
			case DOUBLE:
				return data1.doubleData < data2.doubleData;
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}
		return false;
	}

	friend bool operator == (const Data &data1, const Data &data2) {
		if(data1.dataType != data2.dataType) {
			cerr << "Data type distincts!" << endl;
			exit(-1);
		}
		switch(data1.dataType) {
			case INTEGER:
				return data1.intData == data2.intData;
			case BIGINT:
				return data1.longlongData == data2.longlongData;
			case FLOAT:
				return data1.floatData == data2.floatData;
			case DOUBLE:
				return data1.doubleData == data2.doubleData;
			default:
				cerr << "Data type not supported yet!" << endl;
				exit(-1);
				break;
		}
		return false;
	}

	~Data() {
		if((dataType & 0xff) == CHAR || (dataType & 0xff) == VARCHAR)
			if(charData != NULL)
				delete charData;
	}
};

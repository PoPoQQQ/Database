#pragma once
#include <string>
#include <iostream>
using namespace std;

void CheckDate(int year, int month, int day);
int GetDigit(unsigned int number);

class Data {
public:
	enum DataType {
		UNDEFINED = 0,

		INT = 1,
		VARCHAR = 2,
		DATE = 3,
		FLOAT = 4
	};

	int dataType;
	int dataSize;
	unsigned int intData;
	float floatData;
	string stringData;
	
	Data();
	Data(DataType dataType, int para = 10);
	~Data();
	Data(const Data& data);
	Data& operator = (const Data& data);

	Data& SetData(unsigned int data);
	Data& SetData(float data);
	Data& SetData(string data);
	Data& SetData(const Data &data);

	Data& SetNull();
	Data& SetNegInf();
	Data& SetPosInf();

	void LoadType(unsigned int* b);
	void SaveType(unsigned int* b) const;
	void LoadData(unsigned char* b);
	void SaveData(unsigned char* b) const;

	friend ostream& operator << (ostream& os, const Data &data);
	friend bool operator < (const Data &data1, const Data &data2);
	friend bool operator == (const Data &data1, const Data &data2);

	friend Data HashData(const Data &data);
};

#ifndef TABLE_HEADER
#define TABLE_HEADER
#include <cstdio>
#include <cstring>
#include "field.h"
using namespace std;
/*
实现了一个表的表头
*/
class TableHeader {
public:
	unsigned long long ridTimestamp;
	int recordCount;
	int fieldCount;
	Field *fields;
	TableHeader(): ridTimestamp(0) {}
};
#endif

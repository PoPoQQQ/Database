#include "SelectFieldList.h"

void SelectFieldList::AddSelectField(const Field& field) {
    if(FieldCount() >= MAX_COL_NUM){
		printf("MAX_COL_NUM = %d, FieldCount = %d", MAX_COL_NUM, FieldCount());
		throw "Too many fields!";
	}
    fields.push_back(field);
}
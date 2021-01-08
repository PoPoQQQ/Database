#include "SelectFieldList.h"
#include "../Parser/ColObj.h"
void SelectFieldList::AddSelectField(const ColObj& col, const Field& field) {
    if(FieldCount() >= MAX_COL_NUM){
		printf("MAX_COL_NUM = %d, FieldCount = %d", MAX_COL_NUM, FieldCount());
		throw "Too many fields!";
	}
    fields.push_back(field);
	for(int i = 0;i < cols.size(); ++i) {
		if(cols[i] == col) {
			this->isUnique = false;
			break;
		}
	}
	cols.push_back(col);
}

int SelectFieldList::GetColumnIndex(const ColObj& col) {
	for(int i = 0; i < cols.size(); ++i) {
		if(cols[i] == col) {
			return i;
		}
	}
	return -1;
}
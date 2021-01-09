#include "SetClauseObj.h"
#include "../Record/FieldList.h"
#include "../Record/Record.h"
#include <map>
using namespace std;
bool SetClauseObj::validate(const FieldList& fieldList) {
	// 1. 每一个 colName 必须存在于 fieldList 中，且不能重复
	for(map<string, Data>::const_iterator it = setClauseMap.begin(); it != setClauseMap.end(); ++it) {
		const int cIndex = fieldList.GetColumnIndex(it->first.c_str());
		if(cIndex >= 0) {
			const Field& cField = fieldList.GetColumn(cIndex);
			// 2. 如果查找到则检查 data 是否合乎标准
			if(/*!cField.validateData(it->second)*/false) {
				char buf[256];
				snprintf(buf, 256, "Error: set clause colName(%s) data error", it->first.c_str());
				throw string(buf);
			}
		}else {
			// 如果查找不到对应的列名则报错
			char buf[256];
			snprintf(buf, 256, "Error: set clause colName does't exist: %s", it->first.c_str());
			throw string(buf);
		}
	}
	return true;
}

void SetClauseObj::apply(Record& record) {
	for(map<string, Data>::const_iterator it = setClauseMap.begin(); it != setClauseMap.end(); ++it) {
		record.FillData(it->first.c_str(), it->second);
	}
}
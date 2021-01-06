#include "SetClauseObj.h"
#include "../Record/FieldList.h"
#include <map>
using namespace std;
bool SetClauseObj::validate(const FieldList& fieldList) {
    // 1. 每一个 colName 必须存在于 fieldList 中，且不能重复
    bool found = false;
    for(map<string, Data>::const_iterator it = setClauseMap.begin(); it != setClauseMap.end(); ++it) {
        found = false;
        for(vector<Field>::const_iterator iter = fieldList.fields.begin(); iter != fieldList.fields.end(); ++iter) {
            if(strcmp(it->first.c_str(), iter->columnName) == 0) {
                found = true;
                // 2. 如果查找到则检查 data 是否合乎标准
                if(!iter->validateData(it->second)) {
                    char buf[256];
                    snprintf(buf, 256, "Error: set clause colName(%s) data error", iter->columnName);
                    throw string(buf);
                }
                break;
            }
        }
        if(!found) {
            // 如果查找不到对应的列名则报错
            char buf[256];
            snprintf(buf, 256, "Error: set clause colName does't exist: %s", it->first.c_str());
            throw string(buf);
        }
    }
    return true;
}
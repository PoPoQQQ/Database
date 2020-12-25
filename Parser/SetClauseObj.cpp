#include "SetClauseObj.h"
#include "../Record/FieldList.h"
#include <map>
using namespace std;

bool SetClauseObj::validate(const FieldList& fieldList) {
    // 1. 每一个 colName 必须存在于 fieldList 中，且不能重复
    // for(const vector<Field>::const_iterator iter = fieldList.fields.begin(); iter != fieldList.fields.end(); ++iter) {
    //     set_map.insert(string(iter->columnName), *iter);
    // }
    // for(int i = 0;i < this->colNames.size(); ++i) {
    //     // 如果查找不到对应的列名则报错
    //     if(set_map.find(colNames[i]) == set_map.end()) {
    //         char buf[256];
    //         snprintf(buf, 256, "Error: set clause colName does't exist: ", colNames[i]);
    //         throw string(buf);
    //     }
    //     // 如果查找到的列名是重复的则报错
    // }
    // 2. 每一个 data 必须与对应的 colName 的类型匹配
    // 且如果标记不为空则不能为空
    // 如果有索引则必须依据索引
}
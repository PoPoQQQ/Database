//! 用于在 parser 中传递 SetClause 变量数据的结构体
#ifndef __SET_CLAUSE_OBJ_H__
#define __SET_CLAUSE_OBJ_H__
#include <string>
#include <vector>
#include <map>
#include "../Record/Data.h"
class FieldList;
struct SetClauseObj {
    map<string, Data> setClauseMap;
    void print() {
        cout << "SetClause {" << endl;
        for(map<string, Data>::const_iterator iter = setClauseMap.begin(); iter != setClauseMap.end(); ++iter) {
            cout << "  " << iter->first << ": " << iter->second << endl;
        }
        cout << "}" << endl;
    }
    bool validate(const FieldList& fieldList);
};
#endif // __SET_CLAUSE_OBJ_H__

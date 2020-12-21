//! 用于在 parser 中传递 SetClause 变量数据的结构体
#ifndef __SET_CLAUSE_OBJ_H__
#define __SET_CLAUSE_OBJ_H__
#include <string>
#include <vector>
#include "../Record/Data.h"
struct SetClauseObj {
    vector<string> colNames;
    vector<Data> values;
    void print() {
        cout << "SetClause {" << endl;
        for(int i = 0;i < colNames.size(); ++i) {
            cout << "  " << colNames[i] << ": " << values[i] << endl;
        }
        cout << "}" << endl;
    }
};
#endif // __SET_CLAUSE_OBJ_H__
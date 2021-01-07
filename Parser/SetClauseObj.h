//! 用于在 parser 中传递 SetClause 变量数据的结构体
#ifndef __SET_CLAUSE_OBJ_H__
#define __SET_CLAUSE_OBJ_H__
#include <string>
#include <vector>
#include <map>
#include "../Record/Data.h"
class FieldList;
class Record;
struct SetClauseObj {
    map<string, Data> setClauseMap;
    void print() {
        cout << "SetClause {" << endl;
        for(map<string, Data>::const_iterator iter = setClauseMap.begin(); iter != setClauseMap.end(); ++iter) {
            cout << "  " << iter->first << ": " << iter->second << endl;
        }
        cout << "}" << endl;
    }
    /** 
     * 检查确认每一个 SetClause 中的内容针对该 fieldList：\n
     * 1. 每一个列名存在且唯一\n
     * 2. 每一个列的数据与给出的数据合乎要求\n
     * @throws : const char * err
     */
    bool validate(const FieldList& fieldList);
    /**
     * 根据自身的内容对 Record 进行设置
     * 运行的时候不会调用 validate 确保正确性，请在调用之前确认正确性
     */
    void apply(Record& record);
};
#endif // __SET_CLAUSE_OBJ_H__

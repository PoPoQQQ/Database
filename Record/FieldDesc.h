//! 用于在 parser 中记录 Field 以及 Primary Key 和 Foreign Key 信息的结构体
//! 仅用于传递信息
#ifndef __FIELD_DESC_H__
#define __FIELD_DESC_H__ 
#include "Field.h"
#include <vector>
#include <string>
using namespace std;
struct FieldDesc {
    enum class FieldType {
        UNDEFINED,
        NORMAL,
        NOTNULL,
        DEFAULT,
        NOTNULLWITHDEFAULT,
        PRIMARY,
        FOREIGN
    };
    FieldType type;
    Field field;
    vector<string> columnList;
    string tbName;
    vector<string> ref_columnList;
    FieldDesc(){
        this->type = FieldType::UNDEFINED;
    }
    void print() {
        cout << "FieldType(" << (int)type << ")" << endl;
    }
};
#endif // __FIELD_DESC_H__

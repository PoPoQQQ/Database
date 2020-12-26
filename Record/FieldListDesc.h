//! Parser 中用于储存和生成 FieldList 对象的储存数据用结构体
//! 仅在解析的时候使用
#ifndef __FIELDLIST_DESC_LIST_H__
#define __FIELDLIST_DESC_LIST_H__
#include "FieldDesc.h"
#include "FieldList.h"
#include <vector>
using namespace std;
struct FieldListDesc {
    vector<Field> fields;
    vector<string> columnList;
    string tbName;
    vector<string> ref_columnList;
    void addFieldDesc(const FieldDesc& fielddesc);
    FieldList toFieldList();
};
#endif

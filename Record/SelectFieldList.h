#ifndef __SELECT_FIELDLIST_H__
#define __SELECT_FIELDLIST_H__
#include "FieldList.h"
class SelectFieldList: public FieldList {
public:
    /**
     * 在不考虑重复的 field name 的情况下添加 field
     * 只用于处理 select 中不同表相同列名的情况
     * */
    void AddSelectField(const Field& field);
};
#endif // __SELECT_FIELDLIST_H__
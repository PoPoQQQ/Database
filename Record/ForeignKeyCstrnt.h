//! 用于描述一个外键约束

#ifndef __FOREIGH_KEY_CSTRNT_H__
#define __FOREIGH_KEY_CSTRNT_H__
#include <vector>
#include <map>
#include "FieldConstraint.h"
#include "Field.h"
#include "Table.h"
using namespace std;

class ForeignKeyCstrnt: public FieldConstraint {
public:
    vector<string> colList; // 储存约束对应的列名（若有多个则是联合键名）
    char tbName[MAX_IDENTIFIER_LEN + 1]; // 外键关联的表的名称
    vector<string, Field*> ref_colList; // 外键关联的列名（或联合列名）

    ForeignKeyCstrnt(const char* name="", FieldConstraint::ConstraintType type=FieldConstraint::FOREIGN_KEY);
    ForeignKeyCstrnt(const ForeignKeyCstrnt& other);

    string toString() const;
    
    BufType LoadConstraint(BufType b); // TODO
    BufType SaveConstraint(BufType b) const; // TODO

    void apply(const Table& table);
};
#endif // __FOREIGH_KEY_CSTRNT_H__
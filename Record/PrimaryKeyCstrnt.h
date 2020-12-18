//! 
#ifndef __PRIMARY_KEY_CSTRNT_H__
#define __PRIMARY_KEY_CSTRNT_H__
#include "FieldConstraint.h"
#include <vector>
#include <string>
#include "./Field.h"
using namespace std;

class PrimaryKeyCstrnt: public FieldConstraint {
public:
    vector<string> pkList; // 用于储存所有主键列名的 vector（主键只有一个，长度代表是否有主键和主键是否是联合的）

    BufType LoadConstraint(BufType b); // TODO
    BufType SaveConstraint(BufType b) const; // TODO

    string toString() const;

    // void validate(const vector<Field>&);
    void apply(vector<Field>&);

    PrimaryKeyCstrnt(const char* name="", FieldConstraint::ConstraintType type=FieldConstraint::PRIMARY_KEY);
    PrimaryKeyCstrnt(const PrimaryKeyCstrnt& other);
};
#endif // __PRIMARY_KEY_CSTRNT_H__
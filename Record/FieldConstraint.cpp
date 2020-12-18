#include "FieldConstraint.h"
#include <cstdio>
#include <cstring>
#include <string>
using namespace std;

FieldConstraint::FieldConstraint(const char* name, ConstraintType type) {
    this->type = type;
    
    if(strlen(name) > MAX_IDENTIFIER_LEN) {
        throw "Error in FieldConstraint: name too long";
    }
    strcpy(this->name, name);
}

FieldConstraint::FieldConstraint(const FieldConstraint& other) {
    this->type = other.type;
    strcpy(this->name, other.name);
}

BufType FieldConstraint::SaveBasic(BufType b) const {
    memcpy(b, this->name, MAX_IDENTIFIER_LEN);
	b += MAX_IDENTIFIER_LEN >> 2;

    b[0] = this->type;
    b += 1;

    return b;
}

BufType FieldConstraint::LoadBasic(BufType b) {
    memcpy(this->name, b, MAX_IDENTIFIER_LEN);
	b += MAX_IDENTIFIER_LEN >> 2;

    this->type = b[0];
    b += 1;

    return b;
}

BufType FieldConstraint::SaveConstraint(BufType b) const {
    return SaveBasic(b);
}

BufType FieldConstraint::LoadConstraint(BufType b) {
    return LoadBasic(b);
}

string FieldConstraint::toString() const {
    char buf[200];
    snprintf(buf, sizeof(buf),"FieldConstraint{ name: %s, type: %d }", name, type);
    return string(buf);
}
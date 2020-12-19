#include "ForeignKeyCstrnt.h"

ForeignKeyCstrnt::ForeignKeyCstrnt(const char* name, FieldConstraint::ConstraintType type)
    :FieldConstraint(name, type){

}

ForeignKeyCstrnt::ForeignKeyCstrnt(const ForeignKeyCstrnt& other) {
    this->colList = other.colList;
    strcpy(this->tbName, other.tbName);
    this->ref_colList = other.colList;
}

string ForeignKeyCstrnt::toString() const {
    char buf[256];
    snprintf(buf, 256, "ForeignKeyCstrnt {name: %s}", this->name);
    return string(buf);
}


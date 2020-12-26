#include "PrimaryKeyCstrnt.h"
#include "FieldList.h"
#include <iostream>
#include <sstream>

PrimaryKeyCstrnt::PrimaryKeyCstrnt(const char* name, FieldConstraint::ConstraintType type) 
    :FieldConstraint(name, type) {
        
}

PrimaryKeyCstrnt::PrimaryKeyCstrnt(const PrimaryKeyCstrnt& other)
    :FieldConstraint(other.name, (FieldConstraint::ConstraintType) other.type) {
    this->pkList = other.pkList;
}

BufType PrimaryKeyCstrnt::SaveConstraint(BufType b) const {
    return this->SaveBasic(b);
}

BufType PrimaryKeyCstrnt::LoadConstraint(BufType b) {
    return this->LoadBasic(b);
}

string PrimaryKeyCstrnt::toString() const {
    char buf[200];
    snprintf(buf, sizeof(buf),"PrimaryKeyCstrnt{ name: %s, type: %d, pksize: %lu}", name, type, pkList.size());
    return string(buf);
}

void PrimaryKeyCstrnt::apply(FieldList& fieldList) {
    bool found = false;
    for(vector<string>::const_iterator it = pkList.begin(); it != pkList.end(); ++it) {
        if(fieldList.GetColumnIndex(it->c_str()) == -1) {
            char buf[256];
            snprintf(buf, 256, "Error: Primary key `%s` doesn't exist", it->c_str());
            throw string(buf);
        }
    }
    for(vector<string>::const_iterator it = pkList.begin(); it != pkList.end(); ++it) {
        fieldList.GetColumn(fieldList.GetColumnIndex(it->c_str())).SetPrimaryKey(this->name);
    }
}

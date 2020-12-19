#include "PrimaryKeyCstrnt.h"
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

void PrimaryKeyCstrnt::apply(vector<Field>& fields) {
    bool found = false;
    for(vector<string>::const_iterator it = pkList.begin(); it != pkList.end(); ++it) {
        found = false;
        for(vector<Field>::iterator it2 = fields.begin(); it2 != fields.end(); ++it2) {
            if(strcmp(it2->columnName, it->c_str()) == 0) {
                found = true;
                it2->SetPrimaryKey(this->name);
            }
        }
        if(!found) { // 说明该列描述符不存在
            throw "Error: Primary key doesn't exist";
        }
    }
}
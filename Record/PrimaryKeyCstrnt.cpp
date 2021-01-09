#include <iostream>
#include <algorithm>
#include "PrimaryKeyCstrnt.h"

PrimaryKeyCstrnt::PrimaryKeyCstrnt() {}
PrimaryKeyCstrnt::PrimaryKeyCstrnt(string pkName, const vector<string>& colNames):
    pkName(pkName), colNames(colNames) {
    if(pkName.length() > MAX_IDENTIFIER_LEN)
        throw "Identifier is too long!";
    for(vector<string>::const_iterator it = colNames.begin(); it != colNames.end(); it++)
        if(it->length() > MAX_IDENTIFIER_LEN)
            throw "Identifier is too long!";
}

void PrimaryKeyCstrnt::LoadConstraint(BufType b) {
    int offset = 0;
    static char buffer[MAX_IDENTIFIER_LEN + 1];

    memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
    buffer[MAX_IDENTIFIER_LEN] = 0;
    pkName = string(buffer);
    offset += MAX_IDENTIFIER_LEN;

    for(vector<string>::iterator it = colNames.begin(); it != colNames.end(); it++) {
        memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
        buffer[MAX_IDENTIFIER_LEN] = 0;
        *it = string(buffer);
        offset += MAX_IDENTIFIER_LEN;
    }
}

void PrimaryKeyCstrnt::SaveConstraint(BufType b) const {
    int offset = 0;

    memcpy(b + (offset >> 2), pkName.c_str(), min((signed)pkName.length() + 1, MAX_IDENTIFIER_LEN));
    offset += MAX_IDENTIFIER_LEN;

    for(vector<string>::const_iterator it = colNames.begin(); it != colNames.end(); it++) {
        memcpy(b + (offset >> 2), it->c_str(), min((signed)it->length() + 1, MAX_IDENTIFIER_LEN));
        offset += MAX_IDENTIFIER_LEN;
    }
}

int PrimaryKeyCstrnt::GetConstraintSize() const {
    return MAX_IDENTIFIER_LEN * (colNames.size() + 1);
}

/*
string PrimaryKeyCstrnt::toString() const {
    char buf[200];
    snprintf(buf, sizeof(buf),"PrimaryKeyCstrnt{ name: %s, type: %d, pksize: %lu}", name, (int) type, pkList.size());
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
*/

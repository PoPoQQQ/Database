#include <iostream>
#include <algorithm>
#include "ForeignKeyCstrnt.h"

ForeignKeyCstrnt::ForeignKeyCstrnt() {}
ForeignKeyCstrnt::ForeignKeyCstrnt(string fkName, string tbName, 
    const vector<string>& colNames, const vector<string>& refColNames):
    fkName(fkName), tbName(tbName), colNames(colNames), refColNames(refColNames) {
    if(fkName.length() > MAX_IDENTIFIER_LEN)
        throw "Identifier is too long!";
    if(tbName.length() > MAX_IDENTIFIER_LEN)
        throw "Identifier is too long!";
    if(colNames.size() != refColNames.size())
        throw "reference number does not match!";
    for(vector<string>::const_iterator it = colNames.begin(); it != colNames.end(); it++)
        if(it->length() > MAX_IDENTIFIER_LEN)
            throw "Identifier is too long!";
    for(vector<string>::const_iterator it = refColNames.begin(); it != refColNames.end(); it++)
        if(it->length() > MAX_IDENTIFIER_LEN)
            throw "Identifier is too long!";
}

void ForeignKeyCstrnt::LoadConstraint(BufType b) {
    int offset = 0;
    char buffer[MAX_IDENTIFIER_LEN + 1];

    memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
    buffer[MAX_IDENTIFIER_LEN] = 0;
    fkName = string(buffer);
    offset += MAX_IDENTIFIER_LEN;

    memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
    buffer[MAX_IDENTIFIER_LEN] = 0;
    tbName = string(buffer);
    offset += MAX_IDENTIFIER_LEN;

    uint32_t size = (b + (offset >> 2))[0];
    offset += 4;

    for(uint32_t i = 0;i < size; ++i) {
        memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
        buffer[MAX_IDENTIFIER_LEN] = 0;
        colNames.push_back(string(buffer));
        offset += MAX_IDENTIFIER_LEN;
    }

    size = (b + (offset >> 2))[0];
    offset += 4;

    for(uint32_t i = 0;i < size; ++i) {
        memcpy(buffer, b + (offset >> 2), MAX_IDENTIFIER_LEN);
        buffer[MAX_IDENTIFIER_LEN] = 0;
        refColNames.push_back(string(buffer));
        offset += MAX_IDENTIFIER_LEN;
    }
}

void ForeignKeyCstrnt::SaveConstraint(BufType b) const {
    int offset = 0;

    memcpy(b + (offset >> 2), fkName.c_str(), min((signed)fkName.length() + 1, MAX_IDENTIFIER_LEN));
    offset += MAX_IDENTIFIER_LEN;

    memcpy(b + (offset >> 2), tbName.c_str(), min((signed)tbName.length() + 1, MAX_IDENTIFIER_LEN));
    offset += MAX_IDENTIFIER_LEN;

    (b + (offset >> 2))[0] = (uint32_t) colNames.size();
    offset += 4;

    for(vector<string>::const_iterator it = colNames.begin(); it != colNames.end(); it++) {
        memcpy(b + (offset >> 2), it->c_str(), min((signed)it->length() + 1, MAX_IDENTIFIER_LEN));
        offset += MAX_IDENTIFIER_LEN;
    }

    (b + (offset >> 2))[0] = (uint32_t) refColNames.size();
    offset += 4;

    for(vector<string>::const_iterator it = refColNames.begin(); it != refColNames.end(); it++) {
        memcpy(b + (offset >> 2), it->c_str(), min((signed)it->length() + 1, MAX_IDENTIFIER_LEN));
        offset += MAX_IDENTIFIER_LEN;
    }
}

int ForeignKeyCstrnt::GetConstraintSize() const {
    return MAX_IDENTIFIER_LEN * (colNames.size() * 2 + 2) + 8;
}

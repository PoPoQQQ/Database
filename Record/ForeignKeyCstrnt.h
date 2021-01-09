#pragma once
#include <string>
#include <vector>
#include "Field.h"
using namespace std;
class ForeignKeyCstrnt{
public:
    string fkName, tbName;
    vector<string> colNames, refColNames;

    ForeignKeyCstrnt();
    ForeignKeyCstrnt(string pkName, string tbName, 
        const vector<string>& colNames, const vector<string>& refColNames);

    void LoadConstraint(BufType b);
    void SaveConstraint(BufType b) const;
    int GetConstraintSize() const;
};

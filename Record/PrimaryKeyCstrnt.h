#pragma once
#include <string>
#include <vector>
#include "Field.h"
using namespace std;
class PrimaryKeyCstrnt{
public:
	string pkName;
    vector<string> colNames; // 用于储存所有主键列名的 vector（主键只有一个，长度代表是否有主键和主键是否是联合的）
    
    PrimaryKeyCstrnt();
    PrimaryKeyCstrnt(string pkName, const vector<string>& colNames);

    void LoadConstraint(BufType b);
    void SaveConstraint(BufType b) const;
    int GetConstraintSize() const;
};

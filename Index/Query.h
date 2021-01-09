#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "../Record/Data.h"
#include "WhereCondition.h"
using namespace std;
class Query {
public:
    vector<string> colNames;
    vector<pair<Data, Data>> intervals;
    bool invalidQuery;
    float coverRate;
    Query();
    Query(const vector<string>& colNames, const vector<Data>& keyTypes);
    void GetIntervals(WhereCondition& whereCondition);
    void SetUpQuery(WhereCondition& whereCondition);
    vector<Data> LowerBound();
    vector<Data> UpperBound();
};

/*
三类Constraint：单点，区间，无

满足查询的条件：N单点+0/1区间+N无限制

所有等号的情况外扩，即无视<>，<和>视为<=和>=

IS NULL 单点
NOT NULL 无视
= 单点
> < >= <= 区间
<> 无视
*/

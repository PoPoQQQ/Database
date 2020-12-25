//! 代表了一个 whereclause 所对应的搜索条件
//! 使用类的方式进行储存和操作，不需要进行保存（select的结果不会用来索引，除非被保存）
#ifndef __WHERE_CONDITION_H__
#define __WHERE_CONDITION_H__
#include "../Parser/ColObj.h"
#include "../Parser/OpEnum.h"
#include "../Parser/ExprObj.h"
class WhereCondition {
public:
    enum CondType {
        UNDEFINED = 0, // 初始类型，也是一个错误的类型
        EXPR, // col op expr
        IS_NULL, // col IS NULL
        IS_NOT_NULL, // col IS NOT NULL
        COMBINDED
    } type;

    ColObj col;
    int op;
    ExprObj expr;

    WhereCondition* condition1;
    WhereCondition* condition2;

    WhereCondition(CondType type=UNDEFINED);
    WhereCondition(const WhereCondition&);
    ~WhereCondition();

    void print();
    bool check(); //TODO
};
#endif // __WHERE_CONDITION_H__

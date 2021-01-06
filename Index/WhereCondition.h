//! 代表了一个 whereclause 所对应的搜索条件
//! 使用类的方式进行储存和操作，不需要进行保存（select的结果不会用来索引，除非被保存）
#ifndef __WHERE_CONDITION_H__
#define __WHERE_CONDITION_H__
#include "../Parser/ColObj.h"
#include "../Parser/OpEnum.h"
#include "../Parser/ExprObj.h"
class Table;
class WhereCondition {
public:
    enum class CondType {
        UNDEFINED = 0, // 初始类型，也是一个错误的类型
        EXPR, // col op expr
        IS_NULL, // col IS NULL
        IS_NOT_NULL, // col IS NOT NULL
        COMBINDED
    } type;

    ColObj col;
    OpEnum op;
    ExprObj expr;

    WhereCondition* condition1;
    WhereCondition* condition2;

    WhereCondition(CondType type = CondType::UNDEFINED);
    WhereCondition(const WhereCondition&);
    ~WhereCondition();

    void print();
    // 给出一个 table，检查该 whereClause 中的内容是否可以用于限制该表
    // 所有属于语法层面上的错误都会抛出错误终止执行
    //* @throws: const char * err
    bool validateUpdate(Table&);
    bool check(); //TODO
};
#endif // __WHERE_CONDITION_H__

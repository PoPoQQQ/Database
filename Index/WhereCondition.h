//! 代表了一个 whereclause 所对应的搜索条件
//! 使用类的方式进行储存和操作，不需要进行保存（select的结果不会用来索引，除非被保存）
#ifndef __WHERE_CONDITION_H__
#define __WHERE_CONDITION_H__
#include "../Parser/ColObj.h"
#include "../Parser/OpEnum.h"
#include "../Parser/ExprObj.h"
class Table;
class Record;
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
    /** 给出一个 table，检查该 whereClause 中的内容是否可以用于限制该表;
     * 所有属于语法层面上的错误都会抛出错误终止执行
     * @throws: const char * err/ std::string err
     */
    bool validateUpdate(Table&);
    /**
     * 根据给定的 tbName 判断 whereClause 中书写的内容是否合法
     * @throws const char* err or string err
     */
    bool validate(Table* table);
    /**
     * 根据给定的 tbMap 判断 whereClause 中的内容是否合法
     * 会判断 tbMap 的长度并且判断长度为一时的特殊情况
     * @throws const char* err or string err
     */
    bool validate(const map<string, Table*>& tbMap);
    /** 判断给定的数据条目是否满足给定的条件
     * 使用的时候不会检查 Record 是否存在于 col 对应的 Table 中，需要使用 validate 函数进行判断
     * 默认传入的 Record 就是 whereClause 对应的当前 Table 下的 Record 的数据
     * @throws: const char* err
     * */
    bool check(Record& record);
private:
    /**
     * 根据表达式和给定的 Data 对象计算 expr 的结果
     * 不会抛出异常，如果两者的类型不相同则永远返回 false
     * NULL 类型相关的所有比较默认返回 false
     * */
    bool evaluate(const Data& lData, const Data& rData);
    bool evaluateAlert = false;
};
#endif // __WHERE_CONDITION_H__

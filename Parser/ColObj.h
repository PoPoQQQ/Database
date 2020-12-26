//! 用于在 parser 中传递 col 变量数据的结构体
#ifndef __COL_OBJ_H__
#define __COL_OBJ_H__
#include <string>
#include <iostream>
class Table;
class Field;
struct ColObj {
    std::string tbName;
    std::string colName;
    ColObj(){}
    ColObj(const ColObj& other) {
        tbName = other.tbName;
        colName = other.colName;
    }
    ColObj& operator = (const ColObj& other) {
        tbName = other.tbName;
        colName = other.colName;
        return *this;
    }
    void print() {
        std::cout << "Col{tbName:" << tbName << ", colName: " << colName << " }" << std::endl;
    }
    // 不会抛出错误
    // 如果该列在给定的表内则返回真值
    bool isInTable(const Table&) const;
    // 从目标表中找到 col 所对应的 Field
    // 如果解析错误不会 throw 错误，而会返回 nullptr
    // 如果找到则会返回指针
    Field* getFieldOrNull(Table &);
    // 在当前数据库中搜索字段是否存在
    // 不会抛出错误
    bool isExistInDB(Table* table = nullptr) const;
    // 在当前数据库中搜索表与字段是否存在
    // 如果存在则会返回对应的 Field 的指针
    // 如果不存在则会返回 nullptr
    // 不会抛出异常
    Field* getFieldOrNullGlobal(Table* table = nullptr);
};
#endif // __COL_OBJ_H__

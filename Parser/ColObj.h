//! 用于在 parser 中传递 col 变量数据的结构体
#ifndef __COL_OBJ_H__
#define __COL_OBJ_H__
#include <string>
#include <iostream>
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
};
#endif // __COL_OBJ_H__
//! 用于传递 parser expr 数据的结构体
#ifndef __EXPR_OBJ_H__
#define __EXPR_OBJ_H__
#include "ColObj.h"
#include "../Record/Data.h"
struct ExprObj {
    bool isCol;
    Data value;
    ColObj col;
    ExprObj() {
        isCol = true;
    }
    ExprObj(const ExprObj& other) {
        isCol = other.isCol;
        if(isCol) {
            col = other.col;
        } else {
            value = other.value;
        }
    }
    ExprObj& operator = (const ExprObj& other) {
        isCol = other.isCol;
        if(isCol) {
            col = other.col;
        } else {
            value = other.value;
        }
        return *this;
    }
    void print() {
        if(isCol) {
            cout << "ExprObj(col){" << endl;
            col.print();
            cout << "}" << endl;
        } else {
            cout << "ExprObj(value) {" << endl;
            cout << value << endl;
            cout << "}" << endl;
        }
    }
};
#endif // __EXPR_OBJ_H__

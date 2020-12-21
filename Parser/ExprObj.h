//! 用于传递 parser expr 数据的结构体
#ifndef __EXPR_OBJ_H__
#define __EXPR_OBJ_H__
#include "ColObj.h"
#include "../Record/Data.h"
struct ExprObj {
    bool isCol;
    Data value;
    ColObj col;
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
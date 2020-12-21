#include "WhereCondition.h"
#include <iostream>
using namespace std;
WhereCondition::WhereCondition(CondType type) 
    : type(UNDEFINED), op(OpEnum::NONE){
    condition1 = nullptr;
    condition2 = nullptr;
}

WhereCondition::WhereCondition(const WhereCondition& other) {
    this->type = other.type;
    if(other.type == COMBINDED) {
        this->condition1 = new WhereCondition(*(other.condition1));
        this->condition2 = new WhereCondition(*(other.condition2));
    } else if(other.type == EXPR) {
        this->col = other.col;
        this->op = other.op;
        this->expr = other.expr;
    } else {
        this->col = other.col;
    }
}

WhereCondition::~WhereCondition() {
    if(this->type == COMBINDED) {
        if(this->condition1 == nullptr)
            delete this->condition1;
        if(this->condition2 == nullptr)
            delete this->condition2;
    }
}

void WhereCondition::print() {
    cout << "whereClause: ----" << endl;
    switch(type) {
        case COMBINDED:
            cout << "type: COMBINDED" << endl;
            condition1->print();
            condition2->print();
            break;
        case EXPR:
            cout << "type: EXPR" << endl;
            col.print();
            cout << "op = " << op << endl;
            expr.print();
            break;
        case IS_NULL:
            cout << "type: IS_NULL" << endl;
            col.print();
            break;
        case IS_NOT_NULL:
            cout << "type: IS_NOT_NULL" << endl;
            col.print();
            break;
        default:
            cout << "type: UNDEFINED" << endl;
            break;
    }
    cout << "~whereClause: ----" << endl;
}

bool WhereCondition::check() {
    return false;
}
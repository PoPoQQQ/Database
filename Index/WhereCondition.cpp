#include <iostream>
#include "WhereCondition.h"
using namespace std;
WhereCondition::WhereCondition(CondType type) 
    : type(CondType::UNDEFINED), op(OpEnum::NONE){
    condition1 = NULL;
    condition2 = NULL;
}

WhereCondition::WhereCondition(const WhereCondition& other) {
    this->type = other.type;
    if(other.type == CondType::COMBINDED) {
        this->condition1 = new WhereCondition(*(other.condition1));
        this->condition2 = new WhereCondition(*(other.condition2));
    } else if(other.type == CondType::EXPR) {
        this->col = other.col;
        this->op = other.op;
        this->expr = other.expr;
    } else {
        this->col = other.col;
    }
}

WhereCondition::~WhereCondition() {
    if(this->type == CondType::COMBINDED) {
        if(this->condition1 == NULL)
            delete this->condition1;
        if(this->condition2 == NULL)
            delete this->condition2;
    }
}

void WhereCondition::print() {
    cout << "whereClause: ----" << endl;
    switch(type) {
        case CondType::COMBINDED:
            cout << "type: COMBINDED" << endl;
            condition1->print();
            condition2->print();
            break;
        case CondType::EXPR:
            cout << "type: EXPR" << endl;
            col.print();
            cout << "op = " << (int)op << endl;
            expr.print();
            break;
        case CondType::IS_NULL:
            cout << "type: IS_NULL" << endl;
            col.print();
            break;
        case CondType::IS_NOT_NULL:
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

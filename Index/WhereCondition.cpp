#include "WhereCondition.h"
#include "../Record/Table.h"
#include "../Parser/OpEnum.h"
#include <iostream>
using namespace std;
WhereCondition::WhereCondition(CondType type) 
	: type(UNDEFINED), op(OpEnum::NONE){
	condition1 = NULL;
	condition2 = NULL;
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
		if(this->condition1 == NULL)
			delete this->condition1;
		if(this->condition2 == NULL)
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

bool WhereCondition::validateUpdate(Table& table) {
	bool result = false;
	const FieldList& fieldList = table.fieldList;
	const char* tableName = this->col.tbName.size() > 0 ? this->col.tbName.c_str() : table.tableName.c_str();
	switch(this->type) {
		case EXPR:
		{
			// 检查列名
			const Field* tgt_field = this->col.getFieldOrNullGlobal(&table);
			if(tgt_field == nullptr) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", tableName, this->col.colName.c_str());
				throw string(buf);
			}
			// 检查目标名
			// 如果没有初始化，说明有解析错误
			if(!(this->expr.isInited)) {
				throw "Error: expr is not initialized";
			}
			// 如果是 col op col 类型的 expr，则检查另一个域是否存在
			if(this->expr.isCol) {

				const Field* src_field = this->expr.col.getFieldOrNullGlobal(&table);
				
				if(src_field == nullptr) {
					char buf[256];
					snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", tableName, this->col.colName.c_str());
					throw string(buf);
				} else {
					// 检查两者类型是否相同
					if((tgt_field->data.dataType & 0xff) != (src_field->data.dataType & 0xff)) {
						// 如果类型不同则认为不能赋值
						char buf[256];
						snprintf(buf, 256, "Error: col (%s.%s) and col (%s.%s) have different data type.",
								tableName, this->col.colName.c_str(), this->expr.col.tbName.size() == 0 ? tableName : this->expr.col.tbName.c_str(), this->expr.col.colName.c_str());
						throw string(buf);
					} else {
						return true;
					}
				}
			} else {
				// 检查类型是否相同
				// 如果类型不同则表达式存在错误
				if(this->expr.value.dataType == Data::DataType::UNDEFINED) {
					// NULL 要进行一次特判（在不确定赋值语句上会不会写的情况下我们先进行特判）
					if(tgt_field->constraints & Field::NOT_NULL) {
						char buf[256];
						snprintf(buf, 256, "Error: assigning to NOT NULL col (%s.%s).",
								tableName, this->col.colName.c_str());
						throw string(buf); 
					}
				} else if((this->expr.value.dataType & 0xff) != (tgt_field->data.dataType & 0xff)) {
					char buf[256];
					snprintf(buf, 256, "Error: col (%s.%s) and value have different data type.",
								tableName, this->col.colName.c_str());
					throw string(buf);
				}
				// 除此之外的错误在更新的过程中检查
				return true;
			}

			break;
		}
		case IS_NULL:
			// 检查列名
			if(!this->col.isInTable(table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			// 即使是 IS_NOT_NULL 的限制也不会报错
			return true;
			break;
		case IS_NOT_NULL:
			// 检查列名
			if(!this->col.isInTable(table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			return true;
			break;
		case COMBINDED:
			result = this->condition1->validateUpdate(table) && 
					 this->condition2->validateUpdate(table);
			break;
		default:
			throw "Error: Validating undefined WhereCondition";
	}
	return result;
}

bool WhereCondition::check(FieldList& fieldList) {
	switch(this->type) {
		case WhereCondition::COMBINDED:
			return this->condition1->check(fieldList) && this->condition2->check(fieldList);
		case WhereCondition::IS_NULL: {
			const int cIndex = fieldList.GetColumnIndex(this->col.colName.c_str());
			if (cIndex >= 0) {
				const Field& cField = fieldList.GetColumn(cIndex);
				if(cField.data.dataType == Data::UNDEFINED) {
					return true;
				} else {
					return false;
				}
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		case WhereCondition::IS_NOT_NULL: {
			const int cIndex = fieldList.GetColumnIndex(this->col.colName.c_str());
			if (cIndex >= 0) {
				const Field& cField = fieldList.GetColumn(cIndex);
				if(cField.data.dataType == Data::UNDEFINED) {
					return false;
				} else {
					return true;
				}
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		case WhereCondition::EXPR: {
			const int cIndex = fieldList.GetColumnIndex(this->col.colName.c_str());
			if (cIndex >= 0) {
				const Field& cField = fieldList.GetColumn(cIndex);
				// 这里所有的比较都是和值进行的
				if(this->expr.isCol) {
					throw "WhereCondition::check(FieldList&) can only check value expr";
				} else if(this->expr.value.dataType == Data::UNDEFINED) {
					// TODO: 特殊化处理 NULL 的比较
					// 这里先默认所有的 NULL 按 true 返回
					return true;
				}
				switch(this->op) {
					case OpEnum::EQUAL:{
						return this->expr.value == cField.data;
					}
					case OpEnum::NOTEQUAL: {
						return !(this->expr.value == cField.data);
					}
					case OpEnum::LEQUAL: {
						return this->expr.value < cField.data || this->expr.value == cField.data;
					}
					case OpEnum::GEQUAL: {
						return !(this->expr.value < cField.data);
					}
					case OpEnum::LESS: {
						return this->expr.value < cField.data;
					}
					case OpEnum::GREATER: {
						return !(this->expr.value < cField.data) && !(this->expr.value == cField.data);
					}
					case OpEnum::NONE:
						throw "Error: WhereCondition has NONE op in check";
					default:
						throw "Error: undefined op type in WhereCondition::check";
				}
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		default:
			throw "Error: UNDEFINED condition is checking";
	}
}
#include "WhereCondition.h"
#include "../Record/Table.h"
#include "../Parser/OpEnum.h"
#include "../Record/Database.h"
#include "../Record/SelectFieldList.h"
#include <iostream>
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
			cout << "op = " << (int) op << endl;
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

bool WhereCondition::validateUpdate(Table& table) {
	bool result = false;
	const FieldList& fieldList = table.fieldList;
	const char* tableName = this->col.tbName.size() > 0 ? this->col.tbName.c_str() : table.tableName.c_str();
	switch(this->type) {
		case CondType::EXPR:
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
		case CondType::IS_NULL:
			// 检查列名
			if(!this->col.isInTable(table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			// 即使是 IS_NOT_NULL 的限制也不会报错
			return true;
			break;
		case CondType::IS_NOT_NULL:
			// 检查列名
			if(!this->col.isInTable(table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			return true;
			break;
		case CondType::COMBINDED:
			result = this->condition1->validateUpdate(table) && 
					 this->condition2->validateUpdate(table);
			break;
		default:
			throw "Error: Validating undefined WhereCondition";
	}
	return result;
}

bool WhereCondition::validate(Table* table) {
	// //1. tbName 对应 table 是否存在
	// Table* table = Database::GetTable(tbName.c_str()); // 如果获取失败会直接抛出异常，这里不进行捕捉
	//2. 根据限制条件类型来判断
	switch(this->type) {
		case CondType::IS_NULL:
			// 检查列名
			if(!this->col.isInTable(*table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			// 即使是 IS_NOT_NULL 的限制也不会报错
			// 该函数只会判断列名是否正确，但是不判断列的内容是否正确
			return true;
		case CondType::IS_NOT_NULL:
			// 检查列名
			if(!this->col.isInTable(*table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			return true;
		case CondType::COMBINDED:
			return  this->condition1->validate(table) && 
					this->condition2->validate(table);
		case CondType::EXPR:{
			// 检查列名是否在 table 中
			if(!this->col.isInTable(*table)) {
				char buf[256];
				snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
				throw string(buf);
			}
			// 如果没有初始化，说明有解析错误
			if(!(this->expr.isInited)) {
				throw "Error: expr is not initialized";
			}
			// 如果是两个列，则把另一个列名也进行判断
			if(this->expr.isCol) {
				if(!this->expr.col.isInTable(*table)) {
					char buf[256];
					snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->expr.col.tbName.c_str(), this->expr.col.colName.c_str());
					throw string(buf);
				}
			}
			// 除此之外不做判断，让比较程序来判断列之间的类型
			return true;
		}
		default:
			throw "Error: Validating undefined WhereCondition";
	}
	
}

bool WhereCondition::validate(const map<string, Table*>& tbMap) {
	if(tbMap.size() == 0) {
		throw "Error: No table to check";
	} else if (tbMap.size() == 1) {
		return this->validate(tbMap.begin()->second);
	} else {
		switch(this->type) {
			case CondType::IS_NULL:
				// 检查列名
				if(!this->col.isInTbMap(tbMap)) {
					char buf[256];
					snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
					throw string(buf);
				}
				// 即使是 IS_NOT_NULL 的限制也不会报错
				// 该函数只会判断列名是否正确，但是不判断列的内容是否正确
				return true;
			case CondType::IS_NOT_NULL:
				// 检查列名
				if(!this->col.isInTbMap(tbMap)) {
					char buf[256];
					snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
					throw string(buf);
				}
				return true;
			case CondType::COMBINDED:
				return  this->condition1->validate(tbMap) && 
						this->condition2->validate(tbMap);
			case CondType::EXPR:{
				// 检查列名是否在 table 中
				if(!this->col.isInTbMap(tbMap)) {
					char buf[256];
					snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->col.tbName.c_str(), this->col.colName.c_str());
					throw string(buf);
				}
				// 如果没有初始化，说明有解析错误
				if(!(this->expr.isInited)) {
					throw "Error: expr is not initialized";
				}
				// 如果是两个列，则把另一个列名也进行判断
				if(this->expr.isCol) {
					if(!this->expr.col.isInTbMap(tbMap)) {
						char buf[256];
						snprintf(buf, 256, "Error: col (%s.%s) doesn't exist", this->expr.col.tbName.c_str(), this->expr.col.colName.c_str());
						throw string(buf);
					}
				}
				// 除此之外不做判断，让比较程序来判断列之间的类型
				return true;
			}
			default:
				throw "Error: Validating undefined WhereCondition";
		}
	}
}

bool WhereCondition::check(Record& record) {
	FieldList& fieldList = record.fieldList;
	switch(this->type) {
		case CondType::COMBINDED:
			return this->condition1->check(record) && this->condition2->check(record);
		case CondType::IS_NULL: {
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
		case CondType::IS_NOT_NULL: {
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
		case CondType::EXPR: {
			const int cIndex = fieldList.GetColumnIndex(this->col.colName.c_str());
			if (cIndex >= 0) {
				const Field& cField = fieldList.GetColumn(cIndex);
				// 这里所有的比较都是和值进行的
				if(this->expr.isCol) {
					// 对于当前的函数，所有的列都在同一个表中，进行查找便可
					const Field* const rField = this->expr.col.getFieldFromList(fieldList);
					if(rField == nullptr) {
						throw "Error: col doesn't exist in fieldList in WhereCondition::check";
					} else {
						return this->evaluate(cField.data, rField->data);
					}
				}
				return this->evaluate(cField.data, this->expr.value);
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		default:
			throw "Error: UNDEFINED condition is checking";
	}
}

bool WhereCondition::check(SelectFieldList& sFieldList) {
	switch(this->type) {
		case CondType::COMBINDED:
			return this->condition1->check(sFieldList) && this->condition2->check(sFieldList);
		case CondType::IS_NULL: {
			const int cIndex = sFieldList.GetColumnIndex(this->col);
			if (cIndex >= 0) {
				const Field& cField = sFieldList.GetColumn(cIndex);
				// TODO: 测试一下这里，可能有问题
				if(cField.data.dataType == Data::UNDEFINED) {
					return true;
				} else {
					return false;
				}
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		case CondType::IS_NOT_NULL: {
			const int cIndex = sFieldList.GetColumnIndex(this->col);
			if (cIndex >= 0) {
				const Field& cField = sFieldList.GetColumn(cIndex);
				if(cField.data.dataType == Data::UNDEFINED) {
					return false;
				} else {
					return true;
				}
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		case CondType::EXPR: {
			const int cIndex = sFieldList.GetColumnIndex(this->col);
			if (cIndex >= 0) {
				const Field& cField = sFieldList.GetColumn(cIndex);
				// 这里所有的比较都是和值进行的
				if(this->expr.isCol) {
					// 我们默认所有需要的列已经在 FieldList 中
					const int rIndex = sFieldList.GetColumnIndex(this->expr.col);
					const Field& rField = sFieldList.GetColumn(rIndex);
					if(rIndex == -1) {
						throw "Error: col doesn't exist in fieldList in WhereCondition::check";
					} else {
						return this->evaluate(cField.data, rField.data);
					}
				}
				return this->evaluate(cField.data, this->expr.value);
			} else {
				throw "Error: col doesn't exist in fieldList in WhereCondition::check";
			}
		}
		default:
			throw "Error: UNDEFINED condition is checking";
	}
}

bool WhereCondition::evaluate(const Data& lData, const Data& rData) {
	// 不同类型默认返回 NULL
	// 另外，NULL 类型有关的所有操作默认返回 false
	Data prData = lData;
	try {
		// 通过 SetData 可以让右值变成和左值相同的 Data 类型
		// 如果不可以这么做则会抛出异常
		prData.SetData(rData);
		switch(this->op) {
			case OpEnum::EQUAL:{
				return lData == prData;
			}
			case OpEnum::NOTEQUAL: {
				return !(lData == prData);
			}
			case OpEnum::LEQUAL: {
				return !(prData < lData);
			}
			case OpEnum::GEQUAL: {
				return !(lData < prData);
			}
			case OpEnum::LESS: {
				return lData < prData;
			}
			case OpEnum::GREATER: {
				return prData < lData;
			}
			case OpEnum::NONE:
				throw "Error: WhereCondition has NONE op in check";
			default:
				throw "Error: undefined op type in WhereCondition::check";
		}	
	} catch (const char *err) {
		if(!evaluateAlert) {
			cerr << err << endl;
			evaluateAlert = true;
		}
		return false;
	}
}
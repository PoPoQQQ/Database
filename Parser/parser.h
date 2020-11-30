//lex.l和yacc.y共同使用的头文件
#ifndef __PARSER_H__
#define __PARSER_H__

#include <iostream>//使用C++库
#include <string>
#include <stdio.h>//printf和FILE要用的
#include "../Record/Database.h"
#include "../Record/Data.h"
#include "../Record/Field.h"

using namespace std;

/*当lex每识别出一个记号后，是通过变量yylval向yacc传递数据的。默认情况下yylval是int类型，也就是只能传递整型数据。
yylval是用YYSTYPE宏定义的，只要重定义YYSTYPE宏，就能重新指定yylval的类型(可参见yacc自动生成的头文件yacc.tab.h)。
在我们的例子里，当识别出标识符后要向yacc传递这个标识符串，yylval定义成整型不太方便(要先强制转换成整型，yacc里再转换回char*)。
这里把YYSTYPE重定义为struct Type，可存放多种信息*/
struct DataStruct {
	Data::DataType dataType;
	int param1;
	int param2;
	void print() {
		printf("DataStruct { dataType: %d, p1: %d, p2: %d }\n", dataType, param1, param2);
	}
	void CopyFrom(const DataStruct & other) {
		dataType = other.dataType;
		param1 = other.param1;
		param2 = other.param2;
	}
	void set(Data::DataType dataType, int para1 = 0, int para2 = 0) {
		this->dataType = dataType;
		this->param1 = para1;
		this->param2 = para2;
	}
};

struct FieldStruct {
	DataStruct data;
	enum FieldType {
		DATA,
		PRIMARY,
		FOREIGN
	} type;
	char fieldName[MAX_STRING_LEN + 1];
	void print() {
		printf("FieldStruct { type: %d, name: %s, datatype: %d }\n", type, fieldName, data.dataType);
	}
	void CopyFrom(const FieldStruct & other) {
		data.CopyFrom(other.data);
		type = other.type;
		memset(fieldName, 0, sizeof fieldName);
		strcpy(fieldName, other.fieldName);
	}
	void set(const char* fieldName, const DataStruct& data, FieldType type) {
		memset(this->fieldName, 0, sizeof this->fieldName);
		strcpy(this->fieldName, fieldName);
		this->data.CopyFrom(data);
		this->type = type;
	}
};
union ComplexType {
	DataStruct data;
	FieldStruct field;
};
struct Type//通常这里面每个成员，每次只会使用其中一个，一般是定义成union以节省空间(但这里用了string等复杂类型造成不可以)
{
	string m_sId;
	int m_nInt;
	string m_nString;
	char m_cOp;
	vector<ComplexType> m_vec;
	ComplexType m_cplx;
};

#define YYSTYPE Type//把YYSTYPE(即yylval变量)重定义为struct Type类型，这样lex就能向yacc返回更多的数据了

// enum Token {
//     DATABASE = 1,
//     DATABASES = 2,
//     TABLE = 3,
//     TABLES = 4,
//     SHOW = 5,
//     CREATE = 6,
//     DROP = 7,
//     USE	= 8,
//     PRIMARY = 9,
//     KEY = 10,
//     NOT = 11,
//     NULLTOKEN = 12,
//     INSERT = 13,
//     INTO = 14,
//     VALUES = 15,
//     DELETE = 16,
//     FROM = 17,
//     WHERE = 18,
//     UPDATE = 19,
//     SET = 20,
//     SELECT = 21,
//     IS = 22,
//     INT = 23,
//     VARCHAR = 24,
//     DEFAULT = 25,
//     CONSTRAINT = 26,
//     CHANGE = 27,
//     ALTER = 28,
//     ADD = 29,
//     RENAME = 30,
//     DESC = 31,
//     REFERENCES = 32,
//     INDEX = 33,
//     AND = 34,
//     DATE = 35,  	
//     FLOAT = 36,
//     FOREIGN = 37,
//     CHAR = 38
// };

#endif // __PARSER_H__
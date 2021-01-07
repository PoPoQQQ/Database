//lex.l和yacc.y共同使用的头文件
#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <vector>
#include <iostream>//使用C++库
#include <stdio.h>//printf和FILE要用的
#include "../Record/Database.h"
#include "../Record/Data.h"
#include "../Record/Field.h"
#include "../Record/FieldDesc.h"
#include "./ColObj.h"
#include "./SetClauseObj.h"
#include "./ExprObj.h"
#include "./OpEnum.h"
#include "../Index/WhereCondition.h"
#include "../Record/SelectFieldList.h"

using namespace std;

/*当lex每识别出一个记号后，是通过变量yylval向yacc传递数据的。默认情况下yylval是int类型，也就是只能传递整型数据。
yylval是用YYSTYPE宏定义的，只要重定义YYSTYPE宏，就能重新指定yylval的类型(可参见yacc自动生成的头文件yacc.tab.h)。
在我们的例子里，当识别出标识符后要向yacc传递这个标识符串，yylval定义成整型不太方便(要先强制转换成整型，yacc里再转换回char*)。
这里把YYSTYPE重定义为struct Type，可存放多种信息*/
struct Type//通常这里面每个成员，每次只会使用其中一个，一般是定义成union以节省空间(但这里用了string等复杂类型造成不可以)
{
	string m_sId;
	unsigned int m_nInt;
	float m_nFloat;
	string m_nString;
	char m_cOp;
	Data m_data;
	Field m_field;
	FieldDesc m_field_desc;
	ColObj m_col;
	SetClauseObj m_set_clause;
	ExprObj m_expr;
	OpEnum m_op;
	WhereCondition m_whereClause;

	vector<FieldDesc> m_field_desc_list;
	FieldList m_fieldList;
	vector<Data> m_valueList;
	vector<string> m_stringList;
	vector<vector<Data>> m_valueLists;
	vector<ColObj> m_colList;
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

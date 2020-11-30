%{
/*本yacc的生成文件是yacc.tab.c和yacc.tab.h
yacc文件由3段组成，用2个%%行把这3段隔开。
第1段是声明段，包括：
1-C代码部分：include头文件、函数、类型等声明，这些声明会原样拷到生成的.c文件中。
2-记号声明，如%token
3-类型声明，如%type
第2段是规则段，是yacc文件的主体，包括每个产生式是如何匹配的，以及匹配后要执行的C代码动作。
第3段是C函数定义段，如yyerror()的定义，这些C代码会原样拷到生成的.c文件中。该段内容可以为空*/

//第1段：声明段
#include "parser.h"	//lex和yacc要共用的头文件，里面包含了一些头文件，重定义了YYSTYPE

extern "C"			//为了能够在C++程序里面调用C函数，必须把每一个需要使用的C函数，其声明都包括在extern "C"{}块里面，这样C++链接时才能成功链接它们。extern "C"用来在C++环境下设置C链接类型。
{					//lex.l中也有类似的这段extern "C"，可以把它们合并成一段，放到共同的头文件main.h中
	void yyerror(const char *s);
	extern int yylex(void);//该函数是在lex.yy.c里定义的，yyparse()里要调用该函数，为了能编译和链接，必须用extern加以声明
}

%}

/*lex里要return的记号的声明
用token后加一对<member>来定义记号，旨在用于简化书写方式。
假定某个产生式中第1个终结符是记号OPERATOR，则引用OPERATOR属性的方式：
1-如果记号OPERATOR是以普通方式定义的，如%token OPERATOR，则在动作中要写$1.m_cOp，以指明使用YYSTYPE的哪个成员
2-用%token<m_cOp>OPERATOR方式定义后，只需要写$1，yacc会自动替换为$1.m_cOp
另外用<>定义记号后，非终结符如file, tokenlist，必须用%type<member>来定义(否则会报错)，以指明它们的属性对应YYSTYPE中哪个成员，这时对该非终结符的引用，如$$，会自动替换为$$.member*/
// %token<m_nInt> INTEGER
// %token<m_nString> STRING
%token<m_sId> IDENTIFIER
%token<m_cOp> OPERATOR
// %type<m_sId> file
// %type<m_sId> tokenlist

%token<m_nInt> VALUE_INT
%token<m_nString> VALUE_STRING
%token DATABASE	DATABASES	TABLE		SHOW	CREATE
%token DROP	USE	PRIMARY	KEY	NOT	NULLTOKEN
%token INSERT	INTO	VALUES	DELETE	FROM	WHERE
%token UPDATE	SET	SELECT	IS	INTTOKEN	VARCHARTOKEN
%token DEFAULT	CONSTRAINT	CHANGE	ALTER	ADD	RENAME
%token DESC	REFERENCES	INDEX	AND		FLOATTOKEN	FOREIGN
%token CHARTOKEN
%token<m_sId> DATETOKEN
%token<m_sId> TABLES

%left AND

%type<m_sId> dbName
%type<m_sId> tbName
%type<m_sId> colName
%type<m_cplx> type
%type<m_cplx> field
%type<m_vec> fieldList

%%

program: 	program stmt
		{
		}
	|	/* empty */
		{
		}
	;

stmt: 	sysStmt ';'
		{
		}
	| 	dbStmt ';'
		{
		}
	| 	tbStmt ';'
		{
		}
	// | idxStmt ';'
	// {

	// }
	// | alterStmt ';'
	// {

	// }
	;

sysStmt: 	SHOW DATABASES
			{
				cout << "show databases: -----------" << endl;
				Database::ShowDatabases();
			}
		;

dbStmt:		CREATE DATABASE dbName
			{
				Database::CreateDatabase($3.c_str());
				cout << "CREATE DATABASE:" << $3 << " succeed!" << endl;
			}
		| 	DROP DATABASE dbName
			{
				// TODO
				cout << "TODO: DROP DATABASE" << endl;
			}
		| 	USE dbName
			{
				Database::OpenDatabase($2.c_str());
				cout << "USE " << $2 << ": OpenDatabase" << endl;
			}
		|	SHOW TABLES
			{
				// TODO
				cout << "TODO: SHOW TABLES" << endl;
			}
		;
tbStmt  : CREATE TABLE tbName '(' fieldList ')'
			{
				vector<Field> fields;
				for(int i = 0; i < $5.size(); ++i) {
					fields.push_back(Field(
						$5[i].field.fieldName,
						Data(
							$5[i].field.data.dataType,
							$5[i].field.data.param1,
							$5[i].field.data.param2
						)
					));
				}
				Database::CreateTable($3.c_str(), fields);
				cout << "CREATE TABLE: " << $3 << "succeed!" << endl;
			}
        | DROP TABLE tbName
			{
				cout << "TODO: DROP TABLE: " << $3 << endl;
			}
        | DESC tbName
			{

			}
        | INSERT INTO tbName VALUES valueLists
			{

			}
        | DELETE FROM tbName WHERE whereClause
			{

			}
        | UPDATE tbName SET setClause WHERE whereClause
			{

			}
        | SELECT selector FROM tableList WHERE whereClause
			{

			}
		;

fieldList	:	field
				{
					if($1.field.type == FieldStruct::FieldType::DATA)
						$$.push_back($1);
				}
			|	fieldList ',' field
				{
					if($3.field.type == FieldStruct::FieldType::DATA)
						$$.push_back($3);
				}
			;

field  	: 	colName type
			{
				$$.field.set($1.c_str(), $2.data, FieldStruct::FieldType::DATA);
			}
      	| 	colName type NOT NULLTOKEN
		  	{
				$$.field.set($1.c_str(), $2.data, FieldStruct::FieldType::DATA);
			}
		| 	colName	type DEFAULT	value
			{
				$$.field.set($1.c_str(), $2.data, FieldStruct::FieldType::DATA);
			}
		|	colName type NOT NULLTOKEN DEFAULT value
			{
				$$.field.set($1.c_str(), $2.data, FieldStruct::FieldType::DATA);
			}
      	|	PRIMARY KEY '(' columnList ')'
		  	{
				$$.field.type = FieldStruct::FieldType::PRIMARY;
			}
      	|	FOREIGN KEY '(' colName ')' REFERENCES  tbName '(' colName ')'
			{
				$$.field.type = FieldStruct::FieldType::FOREIGN;
			}
		;
type  	: INTTOKEN '(' VALUE_INT ')'
			{
				$$.data.set(Data::DataType::INTEGER);
			}
		| CHARTOKEN '(' VALUE_INT ')'
			{
				$$.data.set(Data::DataType::CHAR, $3);
			}
        | VARCHARTOKEN '(' VALUE_INT ')'
			{
				$$.data.set(Data::DataType::VARCHAR, $3);
			}
        | DATETOKEN
			{
				$$.data.set(Data::DataType::DATE);
			}
        | FLOATTOKEN
			{
				$$.data.set(Data::DataType::FLOAT);
			}
		;
valueLists  : '('valueList')'
				{

				}
			| valueLists','  '('valueList')'
				{

				}
			;
valueList  	: value
				{

				}
			| valueList ',' value
				{

				}
			;

value	:	VALUE_INT
			{

			}
        | 	VALUE_STRING
			{

			}
        | 	NULLTOKEN
			{

			}
		;

whereClause : 	col op expr
			| 	col	IS NULLTOKEN
            | 	col IS NOT NULLTOKEN
            | 	whereClause	AND	whereClause
			;

col			: 	tbName '.' colName
			|	colName
			;
op  : '=' | "<>" | "<=" | ">=" | '<' | '>'
	;

expr  : value
      | col
	  ;

setClause  	: colName '=' value
			| setClause ',' colName '=' value
			;
selector	: '*'
			| colList
			;

colList 	: col
			| colList ',' col
				{

				}
			;
tableList	:	tbName
			|	tableList ',' tbName
			;

columnList  :	colName
            |	columnList ',' colName
			;

dbName  : 	IDENTIFIER
			{
				StringValidator::Check($1.c_str());
				$$ = $1;
			}
		;
tbName  :	IDENTIFIER
			{
				StringValidator::Check($1.c_str());
				$$ = $1;
			}
		;
colName :	IDENTIFIER
			{
				StringValidator::Check($1.c_str());
				$$ = $1;
			}
		| 	DATETOKEN
			{
				StringValidator::Check($1.c_str());
				$$ = $1;
			}
		| 	TABLES
			{
				StringValidator::Check($1.c_str());
				$$ = $1;
			}
		;

%%

void yyerror(const char *s)			//当yacc遇到语法错误时，会回调yyerror函数，并且把错误信息放在参数s中
{
	cerr<<s<<endl;					//直接输出错误信息
}



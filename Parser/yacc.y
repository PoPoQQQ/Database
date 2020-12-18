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
// %type<m_sId> file
// %type<m_sId> tokenlist
%token<m_sId> IDENTIFIER IDENTIFIER2
%token<m_nInt> VALUE_INT
%token<m_nFloat> VALUE_FLOAT
%token<m_nString> VALUE_STRING
%token DATABASE DATABASES TABLE SHOW CREATE
%token DROP USE PRIMARY KEY NOT NULLTOKEN
%token INSERT INTO VALUES DELETE FROM WHERE
%token UPDATE SET SELECT IS INTTOKEN VARCHARTOKEN
%token DEFAULT CONSTRAINT CHANGE ALTER ADD RENAME
%token DESC	REFERENCES INDEX AND FLOATTOKEN FOREIGN ON TO
%token<m_sId> DATETOKEN TABLES

%left AND

%type<m_sId> dbName tbName colName
%type<m_data> value type
%type<m_field_desc> field
%type<m_field_desc_list> fieldList
%type<m_valueList> valueList
%type<m_valueLists> valueLists
%type<m_stringList> columnList

%%

program:	program stmt
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
				Database::ShowDatabases();
			}
		;

dbStmt:		CREATE DATABASE dbName
			{
				Database::CreateDatabase($3.c_str());
			}
		| 	DROP DATABASE dbName
			{
				Database::DropDatabase($3.c_str());
			}
		| 	USE dbName
			{
				Database::OpenDatabase($2.c_str());
			}
		|	SHOW TABLES
			{
				Database::ShowTables();
			}
		;
tbStmt  :	CREATE TABLE tbName '(' fieldList ')'
			{
				// TODO: 需要通过数组来生成
				// Database::CreateTable(($3).c_str(), $5);
			}
        |	DROP TABLE tbName
			{
				cout << "TODO: DROP TABLE: " << $3 << endl;
			}
        |	DESC tbName
			{

			}
        |	INSERT INTO tbName VALUES valueLists
			{
				Database::Insert($3.c_str(), $5);
			}
        |	DELETE FROM tbName WHERE whereClause
			{

			}
        |	UPDATE tbName SET setClause WHERE whereClause
			{

			}
        |	SELECT selector FROM tableList WHERE whereClause
			{

			}
		;

fieldList	:	field
				{
					$$.push_back($1);
					// $$.AddField($1);
				}
			|	fieldList ',' field
				{
					$$ = $1;
					$$.push_back($3);
					// $$.AddFiel;d($3);
				}
			;

field  	: 	colName type
			{
				$$.field = Field($1.c_str());
				$$.field.SetDataType($2);
			}
      	| 	colName type NOT NULLTOKEN
		  	{
				$$.field = Field($1.c_str());
				$$.field.SetDataType($2);
				$$.field.SetNotNull();
			}
		| 	colName	type DEFAULT value
			{
				$$.field = Field($1.c_str());
				$$.field.SetDataType($2);
				$$.field.SetDefault($4);
			}
		|	colName type NOT NULLTOKEN DEFAULT value
			{
				$$.field = Field($1.c_str());
				$$.field.SetDataType($2);
				$$.field.SetNotNull();
				$$.field.SetDefault($6);
			}
		|	PRIMARY KEY '(' columnList ')'
			{
				$$.type = FieldDesc::FieldType::PRIMARY;
				$$.columnList = $4;
			}
		| 	FOREIGN KEY '(' columnList ')' REFERENCES tbName '(' columnList ')'
			{
				$$.type = FieldDesc::FieldType::FOREIGN;
				$$.columnList = $4;
				$$.tbName = $7;
				$$.ref_columnList = $9;
			}
		;
type  	:	INTTOKEN '(' VALUE_INT ')'
			{
				$$ = Data(Data::INT, $3);
			}
		/*
		| CHARTOKEN '(' VALUE_INT ')'
			{
				$$ = Data(Data::CHAR, $3);
			}
        */
        |	VARCHARTOKEN '(' VALUE_INT ')'
        	{
        		$$ = Data(Data::VARCHAR, $3);
			}
        |	DATETOKEN
        	{
        		$$ = Data(Data::DATE);
			}
        |	FLOATTOKEN
        	{
        		$$ = Data(Data::FLOAT);
			}

		;
valueLists  : '('valueList')'
			{
				$$.push_back($2);
			}
			| valueLists','  '('valueList')'
			{
				$$ = $1;
				$$.push_back($4);
			}
			;
valueList  	: value
			{
				$$.push_back($1);
			}
			| valueList ',' value
			{
				$$ = $1;
				$$.push_back($3);
			}
			;

value	:	VALUE_INT
			{
				$$ = Data(Data::INT).SetData($1);
			}
		|	VALUE_FLOAT
			{
				$$ = Data(Data::FLOAT).SetData($1);
			}
        | 	VALUE_STRING
			{
				$$ = Data(Data::VARCHAR, 255).SetData(($1).c_str());
			}
        | 	NULLTOKEN
			{
				$$ = Data(Data::UNDEFINED);
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
				{
					$$.push_back($1);
				}
            |	columnList ',' colName
				{
					$$ = $1;
					$$.push_back($3);
				}
			;

dbName  : 	IDENTIFIER
			{
				$$ = $1;
			}
		;
tbName  :	IDENTIFIER
			{
				$$ = $1;
			}
		;
colName :	IDENTIFIER
			{
				$$ = $1;
			}
		| 	DATETOKEN
			{
				$$ = $1;
			}
		| 	TABLES
			{
				$$ = $1;
			}
		;

%%

void yyerror(const char *s)			//当yacc遇到语法错误时，会回调yyerror函数，并且把错误信息放在参数s中
{
	cerr << s << endl;					//直接输出错误信息
}



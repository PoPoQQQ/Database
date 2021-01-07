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

%type<m_sId> dbName tbName colName pkName fkName idxName
%type<m_data> value type
%type<m_field_desc> field
%type<m_field_desc_list> fieldList
%type<m_valueList> valueList
%type<m_valueLists> valueLists
%type<m_stringList> columnList tableList
%type<m_col> col
%type<m_colList> colList selector
%type<m_set_clause> setClause
%type<m_expr> expr
%type<m_op> op
%type<m_whereClause> whereClause

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
	| 	idxStmt ';'
		{

		}
	| 	alterStmt ';'
		{
		}
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
				FieldList fieldList;
				fieldList.AddFieldDescVec($3.c_str(), $5);
				Database::CreateTable(($3).c_str(), fieldList);
			}
        |	DROP TABLE tbName
			{
				cout << "TODO: DROP TABLE: " << $3 << endl;
			}
        |	DESC tbName
			{
				Database::GetTable($2.c_str())->DescTable();
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
				// 检查并读取表格
				Table *table = Database::GetTable($2.c_str()); 
				// 检查 setClause
				$4.validate(table->fieldList);
				// 检查 whereClause
				if(!$6.validate(table)){
					throw "whereClause Error";
				}
				WhereCondition& whereClause = $6;
				SetClauseObj& setClause = $4;
				// 根据 whereClause 中的条件进行搜索，并且利用 setClause 中的内容进行内容的更新
				function<void(Record&, BufType)> it = [&whereClause, &setClause](Record& record, BufType b) {
					if((whereClause).check(record)) {
						(setClause).apply(record);
						record.Save(b);
					}
				};
				table->IterTable(it);
				cout << "Update finished!" << endl;
			}
		|	SELECT selector FROM tableList
			{
				if ($4.size() == 1) {
					// 查询单个 db
					Table* table = Database::GetTable($4[0].c_str());
					if($2.size() == 0) {
						// 如果是 * , 则可以偷懒直接打印
						table->PrintTable();
					} else {
						// 验证每一个 col 是否合法
						for(int i = 0;i < $2.size(); ++i) {
							// 检查 selector
							if(!$2[i].isInTable(*table)) {
								char buf[256];
								snprintf(buf, 256, "Error: Selector (%s.%s) doesn't exist in table %s", $2[i].tbName.c_str(), $2[i].colName.c_str(), table->tableName.c_str());
								throw string(buf);
							}
						}

						FieldList tFieldList;
						const vector<ColObj>& selector = $2;
						for(int i = 0;i < selector.size(); ++i) {
							tFieldList.AddField(table->fieldList.GetColumn(table->fieldList.GetColumnIndex(selector[i].colName.c_str())));
						}
						tFieldList.PrintFields();
						function<void(Record&, BufType)> it = [&tFieldList](Record& record, BufType b) {
							unsigned int bitmap = 0;
							int index = -1;
							for(int i = 0;i < tFieldList.fields.size(); ++i) {
								index = record.fieldList.GetColumnIndex(tFieldList.fields[i].columnName);
								tFieldList.fields[i] = record.fieldList.GetColumn(index);
								bitmap |= (record.bitMap & (1u << index)) ? 1u << i : 0u;
							}
							tFieldList.PrintDatas(bitmap);
						};
						table->IterTable(it);
					}
				} else {
					// 笛卡尔积
					vector<string>& tbList = $4;
					vector<ColObj>& selector = $2;
					// 1. 检查 tableList 中的名称是否存在且唯一
					// 检查 tbList 中的内容是否存在且唯一
					map<string, Table*> tbMap;
					Table* tTable = nullptr;
					for(int i = 0;i < tbList.size(); ++i) {
						tTable = Database::GetTable(tbList[i].c_str());
						if(tbMap.find(tbList[i]) != tbMap.end()) {
							// 如果存在相同的表名则报错
							throw "Error: table name in selector should be unique";
						} else {
							tbMap[tbList[i]] = tTable;
						}
					}
					// 2. 根据 selector 进行不同的操作
					if($2.size() == 0) {
						// 如果是 * ，则将所有的 FieldList 连接起来
						SelectFieldList tFieldList;
						for(int i = 0;i < tbList.size(); ++i) {
							FieldList& fieldList = tbMap.find(tbList[i])->second->fieldList;
							for(int j = 0; j < fieldList.fields.size(); ++j) {
								tFieldList.AddSelectField(fieldList.fields[j]);
							}
						}
						int depth = 0;
						unsigned int bitmap = 0;
						unsigned int bitmapPos = 0;
						function<void(Record&, BufType)> it = [&tFieldList, &depth, &tbList, &tbMap, &bitmap, &bitmapPos, &it](Record& record, BufType b) {
							depth++;
							for(int i = 0;i < record.fieldList.fields.size(); ++i) {
								tFieldList.fields[bitmapPos] = record.fieldList.fields[i];
								bitmap |= (record.bitMap & (1u << i)) ? (1u << bitmapPos) : 0u;
								bitmapPos++;
							}
							if(depth == tbList.size()) {
								tFieldList.PrintDatas(bitmap);
							} else {
								tbMap.find(tbList[depth])->second->IterTable(it);
							}
							depth--;
							bitmap &= (0xffffffffu ^ ((1u << bitmapPos)-(1 << (bitmapPos - record.fieldList.fields.size()))));
							bitmapPos -= record.fieldList.fields.size();
						};
						// 进行递归打印笛卡尔积
						tFieldList.PrintFields();
						tbMap.find(tbList[0])->second->IterTable(it);
					} else {
						// 检查 selector 是否都在其中
						for(int i = 0;i < selector.size(); ++i) {
							if(!selector[i].isInTbMap(tbMap)) {
								char buf[128];
								snprintf(buf, 128, "selector (%s.%s) doesn't exist in table list", selector[i].tbName.c_str(), selector[i].colName.c_str());
								throw string(buf);
							}
						}

						SelectFieldList tFieldList;
						Table* tTable = nullptr;
						for(int i = 0;i < selector.size(); ++i) {
							tTable = Database::GetTable(selector[i].tbName.c_str());
							tFieldList.AddSelectField(tTable->fieldList.GetColumn(tTable->fieldList.GetColumnIndex(selector[i].colName.c_str())));
						}

						int depth = 0;
						unsigned int bitmap = 0;
						unsigned int bitmapPos = 0;
						function<void(Record&, BufType)> it = [&tFieldList, &depth, &selector, &tbMap, &bitmap, &bitmapPos, &it](Record& record, BufType b) {
							// selector 对应的列（目前已经在表中）
							const int cIndex = record.fieldList.GetColumnIndex(selector[depth].colName.c_str());
							const Field& field = record.fieldList.GetColumn(cIndex);
							tFieldList.fields[bitmapPos] = field;
							bitmap |= (record.bitMap & (1u << cIndex)) ? (1u << bitmapPos) : 0u;
							bitmapPos++;
							// 进入下一次递归
							depth++;
							if(depth == selector.size()) {
								tFieldList.PrintDatas(bitmap);
							} else {
								tbMap.find(selector[depth].tbName)->second->IterTable(it);
							}
							// 回退
							depth--;
							bitmapPos--;
							bitmap &= (0xffffffffu ^ (1 << bitmapPos));
						};
						
						// 进行递归打印笛卡尔积
						tFieldList.PrintFields();
						tbMap.find(tbList[0])->second->IterTable(it);
					}
				}
				cout << "SELECT finished!" << endl;
			}
        |	SELECT selector FROM tableList WHERE whereClause
			{
				if ($4.size() == 1) {
					// 查询单个 db
					Table* table = Database::GetTable($4[0].c_str());
					if(!$6.validate(table)){
						throw "whereClause Error";
					}
					if($2.size() == 0) {
						// 如果是 * ，则类似于 UPDATE
						// 根据 whereClause 中的条件进行搜索，如果满足条件则打印
						WhereCondition& whereClause = $6;
						table->fieldList.PrintFields();
						function<void(Record&, BufType)> it = [&whereClause](Record& record, BufType b) {
							if((whereClause).check(record)) {
								record.fieldList.PrintDatas(record.bitMap);
							}
						};
					} else {
						// 验证每一个 col 是否合法
						const vector<ColObj>& selector = $2;
						for(int i = 0;i < selector.size(); ++i) {
							// 检查 selector
							if(!selector[i].isInTable(*table)) {
								char buf[256];
								snprintf(buf, 256, "Error: Selector (%s.%s) doesn't exist in table %s", selector[i].tbName.c_str(), selector[i].colName.c_str(), table->tableName.c_str());
								throw string(buf);
							}
						}

						FieldList tFieldList;
						for(int i = 0;i < selector.size(); ++i) {
							tFieldList.AddField(table->fieldList.GetColumn(table->fieldList.GetColumnIndex(selector[i].colName.c_str())));
						}
						tFieldList.PrintFields();
						WhereCondition& whereClause = $6;
						function<void(Record&, BufType)> it = [&tFieldList, &whereClause](Record& record, BufType b) {
							if(whereClause.check(record)) {
								unsigned int bitmap = 0;
								int index = -1;
								for(int i = 0;i < tFieldList.fields.size(); ++i) {
									index = record.fieldList.GetColumnIndex(tFieldList.fields[i].columnName);
									tFieldList.fields[i] = record.fieldList.GetColumn(index);
									bitmap |= (record.bitMap & (1u << index)) ? 1u << i : 0u;
								}
								tFieldList.PrintDatas(bitmap);
							}
						};
						table->IterTable(it);
					}
				} else {
					// 笛卡尔积
					// 1. 确认 tableList 中的内容不是重复的，创建一个 map
					if($2.size() == 0) {

					}
				}
				cout << "SELECT finished!" << endl;
			}
		;
idxStmt		:	CREATE INDEX idxName ON tbName '(' columnList ')'
			|	DROP INDEX idxName
			|	ALTER TABLE tbName ADD INDEX idxName '(' columnList ')'
			|	ALTER TABLE tbName DROP INDEX idxName
			;
			
alterStmt	:	ALTER TABLE tbName ADD field
			|   ALTER TABLE tbName DROP colName
			|	ALTER TABLE tbName CHANGE colName field
			|	ALTER TABLE tbName RENAME TO tbName
			|	ALTER TABLE tbName DROP PRIMARY KEY
			|	ALTER TABLE tbName ADD CONSTRAINT pkName PRIMARY KEY '(' columnList ')'
			|	ALTER TABLE tbName DROP PRIMARY KEY pkName
			|	ALTER TABLE tbName ADD CONSTRAINT fkName FOREIGN KEY '(' columnList ')' REFERENCES tbName '(' columnList ')'
			|	ALTER TABLE tbName DROP FOREIGN KEY fkName
			;
fieldList	:	field
				{
					$$.push_back($1);
				}
			|	fieldList ',' field
				{
					$$ = $1;
					$$.push_back($3);
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
				// 在索引的时候就事先进行部分可行的检查
				// 1. 检查两个 List 的长度是否相等
				if($4.size() != $9.size()) {
					throw "Error: Foreign key lists have different length.";
				}
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
			| valueLists ',' '(' valueList ')'
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
				{
					$$.type = WhereCondition::CondType::EXPR;
					$$.col = $1;
					$$.op = $2;
					$$.expr = $3;
				}
			| 	col	IS NULLTOKEN
				{
					$$.type = WhereCondition::CondType::IS_NULL;
					$$.col = $1;
				}
            | 	col IS NOT NULLTOKEN
				{
					$$.type = WhereCondition::CondType::IS_NOT_NULL;
					$$.col = $1;
				}
            | 	whereClause	AND	whereClause
				{
					$$.type = WhereCondition::CondType::COMBINDED;
					$$.condition1 = new WhereCondition($1);
					$$.condition2 = new WhereCondition($3);
				}
			;

col			: 	tbName '.' colName
				{
					$$.tbName = $1;
					$$.colName = $3;
				}
			|	colName
				{
					$$.colName = $1;
				}
			;
op  : '='
		{
			$$ = OpEnum::EQUAL;
		}
	| "<>" 
		{
			$$ = OpEnum::NOTEQUAL;
		}
	| "<=" 
		{
			$$ = OpEnum::LEQUAL;
		}
	| ">=" 
		{
			$$ = OpEnum::GEQUAL;
		}
	| '<' 
		{
			$$ = OpEnum::LESS;
		}
	| '>'
		{
			$$ = OpEnum::GREATER;
		}
	;

expr  : value
		{
			$$.isCol = false;
			$$.value = $1;
			$$.isInited = true;
		}
      | col
	  	{
			$$.isCol = true;
			$$.col = $1;
			$$.isInited = true;
		}
	  ;

setClause  	: colName '=' value
				{
					// 如果语句中有重复的列名，则报错退出
					if($$.setClauseMap.find($1) != $$.setClauseMap.end()) {
						char buf[256];
						snprintf(buf, 256, "Error: set clause has duplicate colName: %s", $1.c_str());
						throw string(buf);
					} else {
						$$.setClauseMap.insert(make_pair($1, $3));
					}
				}
			| setClause ',' colName '=' value
				{
					$$ = $1;
					if($$.setClauseMap.find($3) != $$.setClauseMap.end()) {
						char buf[256];
						snprintf(buf, 256, "Error: set clause has duplicate colName: %s", $3.c_str());
						throw string(buf);
					} else {
						$$.setClauseMap.insert(make_pair($3, $5));
					}
				}
			;
selector	: '*'
				{

				}
			| colList
				{
					$$ = $1;
				}
			;

colList 	: col
				{
					$$.push_back($1);
				}
			| colList ',' col
				{
					$$ = $1;
					$$.push_back($3);
				}
			;
tableList	:	tbName
				{
					$$.push_back($1);
				}
			|	tableList ',' tbName
				{
					$$ = $1;
					$$.push_back($3);
				}
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

pkName : IDENTIFIER
			{
				$$ = $1;
			}
		;

fkName : IDENTIFIER
			{
				$$ = $1;
			}
		;
idxName : IDENTIFIER
			{
				$$ = $1;
			}
		;
%%

void yyerror(const char *s)			//当yacc遇到语法错误时，会回调yyerror函数，并且把错误信息放在参数s中
{
	cerr << s << endl;					//直接输出错误信息
}



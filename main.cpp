#include <vector>
#include <cstring>
#include <stdio.h>
#include <functional>
#include <iostream>
#include "Index/Index.h"
#include "Record/Table.h"
#include "Utils/Global.h"
#include "Record/Database.h"
#include "Record/FieldList.h"
#include "Utils/Constraints.h"
#include "FileIO/FileManager.h"
#include "BufManager/BufPageManager.h"
#include "Index/WhereCondition.h"
#include "Parser/OpEnum.h"
#include "Parser/SetClauseObj.h"
using namespace std;

extern FILE *yyin;
extern int yyparse();

int main(int argc, const char* argv[]) {
	
	// 初始化数据库管理系统
	Global::getInstance()->fm = new FileManager();
	Global::getInstance()->bpm = new BufPageManager(Global::getInstance()->fm);

	// 创建数据库管理文件夹
	Database::LoadDatabases();

    if (argc == 2) {
    	yyin = fopen(argv[1], "r");
	    if (yyin == NULL) {
	        printf("Open file failed: %s\n", argv[1]);
	        return 0;
	    }

		printf("-----begin parsing %s\n", argv[1]);
		try {
			yyparse(); //使yacc开始读取输入和解析，它会调用lex的yylex()读取记号
		}
		catch (const char* err) {
			cout << err << endl;
		}
		catch (string err) {
			cout << err << endl;
		}
		
		puts("-----end parsing");

		fclose(yyin);

		yyin = NULL;
    }
    else {
		try {
			Database::CreateDatabase("MyDatabase");
			Database::OpenDatabase("MyDatabase");

			FieldList fieldList;
			fieldList.AddField(Field("a").SetDataType(Data(Data::INT)));
			fieldList.AddField(Field("b").SetDataType(Data(Data::DATE)));
			fieldList.AddField(Field("c").SetDataType(Data(Data::FLOAT)));
			fieldList.AddField(Field("d").SetDataType(Data(Data::VARCHAR, 255)));
			Table *table = Database::CreateTable("TestTable", fieldList);

			Record record = table->EmptyRecord();
			for(int i = 0; i < 128; i++) {
				record.CleanData();
				record.FillData("a", Data(Data::INT).SetData((unsigned)i));
				record.FillData("b", Data(Data::DATE).SetData("1998/04/02"));
				record.FillData("c", Data(Data::FLOAT).SetData(233.33f));
				record.FillData("d", Data(Data::VARCHAR, 255).SetData("A quick brown fox jump over the lazy dog."));
				table->AddRecord(record);
			}
			
			// table->PrintTable();
			WhereCondition where;
			where.type = WhereCondition::EXPR;
			
			where.col.colName = "a";
			where.op = OpEnum::GEQUAL;
			where.expr.isCol = false;
			where.expr.value = Data(Data::INT).SetData((unsigned) 100);
			where.expr.isInited = true;

			SetClauseObj setClause;
			setClause.setClauseMap.insert(make_pair(string("a"), Data(Data::INT).SetData((unsigned) 666)));
			setClause.setClauseMap.insert(make_pair("c", Data(Data::FLOAT).SetData((float) 7451.76)));
			
			cout << setClause.validate(table->fieldList) << endl;
			cout << where.validateUpdate(*table) << endl;

			function<void(Record&, BufType)> it = [&where, &setClause](Record& record, BufType b) {
				static int count = 0;
				printf("Record(%d): %d\n", count++, where.check(record));
				if(where.check(record)) {
					setClause.apply(record);
					record.Save(b);
				}
			};
			table->IterTable(it);
			table->PrintTable();

			//Database::CreateDatabase("MyDatabase");
			//Database::OpenDatabase("MyDatabase");

			// vector<Data> keys;
			// keys.push_back(Data(Data::INT));
			// Index* index = new Index("MyDatabase", "MyTable", "MyIndex", keys);
			// for(int i = 1; i <= 21; i++) {
			// 	keys[0].SetData((unsigned)i);
			// 	index->Insert(keys, i * 10);
			// 	cout << "=============================" << i << endl;
			// 	index->Print();
			// }
			// for(int i = 1; i <= 21; i++) {
			// 	keys[0].SetData((unsigned)i);
			// 	index->Remove(keys);
			// 	cout << "=============================" << i << endl;
			// 	index->Print();
			// }
		}
    	catch (const char* err) {
			cout << "error: " << endl;
			cout << err << endl;
		}
    }

	Global::getInstance()->bpm->close();
	return 0;
}

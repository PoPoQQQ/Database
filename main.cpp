#include <vector>
#include <map>
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
#include "Parser/ColObj.h"
#include "Record/SelectFieldList.h"
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
			fieldList.AddField(Field("e").SetDataType(Data(Data::INT)));
			Table *table2 = Database::CreateTable("TestTable2", fieldList);

			vector<vector<Data>> dataLists;
			vector<vector<Data>> dataLists2;
			for(int i = 0; i < 128; i++) {
				vector<Data> dataList;
				dataList.push_back(Data(Data::INT).SetData((unsigned)i));
				dataList.push_back(Data(Data::DATE).SetData("1998/04/02"));
				dataList.push_back(Data(Data::FLOAT).SetData(233.33f));
				dataList.push_back(Data(Data::VARCHAR, 255).SetData("A quick brown fox jump over the lazy dog."));
				dataLists.push_back(dataList);

				vector<Data> dataList2;
				dataList2.push_back(Data(Data::INT).SetData((unsigned)i+100));
				dataList2.push_back(Data(Data::DATE).SetData("2020/04/02"));
				dataList2.push_back(Data(Data::FLOAT).SetData(233.33f));
				dataList2.push_back(Data(Data::VARCHAR, 255).SetData("*****************."));
				dataList2.push_back(Data());
				dataLists2.push_back(dataList2);
			}
			Database::Insert("TestTable", dataLists);
			Database::Insert("TestTable2", dataLists2);

			Database::RenameTable("TestTable", "TestTable3");
			/*
			vector<string> columns;
			vector<Data> keys;
			columns.push_back("Column");
			keys.push_back(Data(Data::INT));
			Index* index = new Index("MyDatabase", "MyTable", "MyIndex", columns, keys);
			for(int i = 1; i <= 21; i++) {
				keys[0].SetData((unsigned)i);
				index->Insert(keys, i * 10);
				cout << "=============================" << i << endl;
				index->Print();
			}
			for(int i = 1; i <= 21; i++) {
				keys[0].SetData((unsigned)i);
				index->Remove(keys);
				cout << "=============================" << i << endl;
				index->Print();
			}*/

		}
    	catch (const char* err) {
			cout << "error: " << endl;
			cout << err << endl;
		}
		catch (string err) {
			cout << err << endl;
		}
    }
	Database::CloseDatabase();
	Global::getInstance()->bpm->close();
	return 0;
}

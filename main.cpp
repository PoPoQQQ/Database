#include <vector>
#include <cstring>
#include <stdio.h>
#include <iostream>
#include "Index/Index.h"
#include "Record/Field.h"
#include "Record/Table.h"
#include "Utils/Global.h"
#include "Record/Database.h"
#include "Utils/Constraints.h"
#include "FileIO/FileManager.h"
#include "BufManager/BufPageManager.h"
using namespace std;

extern FILE *yyin;
extern int yyparse();

int main(int argc, const char* argv[]) {
	
	// 初始化数据库管理系统
	Global::getInstance()->fm = new FileManager();
	Global::getInstance()->bpm = new BufPageManager(Global::getInstance()->fm);

	// 创建数据库管理文件夹
	//Database::LoadDatabases();

    if (argc == 2) {
    	yyin = fopen(argv[1], "r");
	    if (yyin == NULL) {
	        printf("Open file failed: %s\n", argv[1]);
	        return 0;
	    }

		printf("-----begin parsing %s\n", argv[1]);
		yyparse();						//使yacc开始读取输入和解析，它会调用lex的yylex()读取记号
		puts("-----end parsing");

		fclose(yyin);

		yyin = NULL;
    }
    else {
    	Database::CreateDatabase("MyDatabase");
		Database::OpenDatabase("MyDatabase");

		vector<Field> fields;
		fields.push_back(Field("a", Data(Data::INT)));
		fields.push_back(Field("b", Data(Data::DATE)));
		fields.push_back(Field("c", Data(Data::FLOAT)));
		fields.push_back(Field("d", Data(Data::VARCHAR, 255)));
		Table *table = Database::CreateTable("TestTable", fields);

		cout << 0 << endl;
		Record *record = table->CreateEmptyRecord();
		for(int i = 0; i < 128; i++) {
			record->CleanData();
			cout << 1 << endl;
			record->FillData(0, Data(Data::INT).SetData((unsigned)i));
			cout << 2 << endl;
			record->FillData(1, Data(Data::DATE).SetData("1998/04/02"));
			cout << 3 << endl;
			record->FillData(2, Data(Data::FLOAT).SetData(233.33f));
			cout << 4 << endl;
			record->FillData(3, Data(Data::VARCHAR, 255).SetData("A quick brown fox jump over the lazy dog."));
			cout << 5 << endl;
			table->AddRecord(record);
		}
		delete record;
		
		table->PrintTable();
    }
    

    /*
	vector<Data> keys;
	keys.push_back(Data(Data::INT));
	Index* index = new Index("MyDatabase", "MyTable", "MyIndex", keys);
	for(int i = 1; i <= 21; i++) {
		keys[0].SetData(i);
		index->Insert(keys, i * 10);
		cout << "=============================" << i << endl;
		index->Print();
	}
	for(int i = 1; i <= 21; i++) {
		keys[0].SetData(i);
		index->Remove(keys);
		cout << "=============================" << i << endl;
		index->Print();
	}*/
	

	Global::getInstance()->bpm->close();
	return 0;
}

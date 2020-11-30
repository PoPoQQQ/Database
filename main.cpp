#include <iostream>
#include <cstring>
#include <stdio.h>
#include <vector>
#include "Record/Database.h"
#include "Record/Field.h"
#include "Record/Table.h"
#include "Utils/Constraints.h"
#include "FileIO/FileManager.h"
#include "BufManager/BufPageManager.h"
using namespace std;

extern FILE *yyin;
extern int yyparse();

int main(int argc, const char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return 0;
    }
	// 初始化数据库管理系统
	Global::getInstance()->fm = new FileManager();
	Global::getInstance()->bpm = new BufPageManager(Global::getInstance()->fm);
	// 创建数据库管理文件夹
	Database::LoadDatabases();

	// // Database::CreateDatabase("TestDatabase");
	// Database::OpenDatabase("TestDatabase");

	// vector<Field> fields;
	// // fields.push_back(Field("a", Data(Data::INTEGER)));
	// // fields.push_back(Field("b", Data(Data::INTEGER)));
	// fields.push_back(Field("c", Data(Data::CHAR, 40)));
	// Table *table = Database::CreateTable("TestTable", fields);

	// Record *record = table->CreateEmptyRecord();
	// for(int i = 0; i < 128; i++) {
	// 	record->CleanData();
	// 	// record->FillData(0, Data(Data::INTEGER).SetData(0x003e2590));
	// 	// record->FillData(1, Data(Data::INTEGER).SetData(0x23333333));
	// 	// if(rand() & 1)
	// 	record->FillData(0, Data(Data::CHAR, 40).SetData("I think something is really happening.  "));
	// 	table->AddRecord(record);
	// }
	// delete record;
	
	// table->PrintTable();
	
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


	Global::getInstance()->bpm->close();
	return 0;
}

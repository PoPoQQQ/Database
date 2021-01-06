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
#include "Parser/ColObj.h"
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
			fieldList.AddField(Field("e").SetDataType(Data(Data::INT)));
			Table *table = Database::CreateTable("TestTable", fieldList);

			Record record = table->EmptyRecord();
			for(int i = 0; i < 128; i++) {
				record.CleanData();
				record.FillData("a", Data(Data::INT).SetData((unsigned)i));
				record.FillData("b", Data(Data::DATE).SetData("1998/04/02"));
				record.FillData("c", Data(Data::FLOAT).SetData(233.33f));
				record.FillData("d", Data(Data::VARCHAR, 255).SetData("A quick brown fox jump over the lazy dog."));
				record.FillData("e", Data());
				table->AddRecord(record);
			}
			
			vector<ColObj> selector;
			
			ColObj col1, col2, col3;
			col1.colName = "a";
			col2.tbName = "TestTable";
			col2.colName = "d";
			col3.colName = "e";
			selector.push_back(col1);
			selector.push_back(col2);
			selector.push_back(col3);

			for(int i = 0;i < selector.size(); ++i) {
				// 检查 selector
				if(!selector[i].isInTable(*table)) {
					char buf[256];
					snprintf(buf, 256, "Error: Selector (%s.%s) doesn't exist in table %s", selector[i].tbName.c_str(), selector[i].colName.c_str(), table->tableName.c_str());
					throw string(buf);
				}
			}

			if(selector.size() > 0) {
				FieldList tFieldList;
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
			} else {
				// 代表要选择所有的列
				// 可以偷懒直接把表打印出来
				table->PrintTable();
			}
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
		catch (string err) {
			cout << err << endl;
		}
    }

	Global::getInstance()->bpm->close();
	return 0;
}

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

			table2->PrintTable();
			vector<ColObj> selector;
			
			ColObj col1, col2, col3;
			col1.colName = "a";
			col1.tbName = "TestTable";
			col2.tbName = "TestTable2";
			col2.colName = "d";
			col3.colName = "a";
			col3.tbName = "TestTable2";
			selector.push_back(col1);
			selector.push_back(col2);
			selector.push_back(col3);
			
			vector<string> tbList;
			tbList.push_back("TestTable");
			tbList.push_back("TestTable2");
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
			// 检查 selector 是否都在其中
			for(int i = 0;i < selector.size(); ++i) {
				if(!selector[i].isInTbMap(tbMap)) {
					char buf[128];
					snprintf(buf, 128, "selector (%s.%s) doesn't exist in table list", selector[i].tbName.c_str(), selector[i].colName.c_str());
					throw string(buf);
				}
			}

			if(selector.size() > 0) {
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
			} else {
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
			}

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

	Global::getInstance()->bpm->close();
	return 0;
}

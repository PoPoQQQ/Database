/*
 * testfilesystem.cpp
 *
 *  Created on: 2015年10月6日
 *      Author: lql
 *          QQ: 896849432
 * 各位同学十分抱歉，之前给的样例程序不支持同时打开多个文件
 * 是因为初始化没有做，现在的程序加上了初始化（main函数中的第一行）
 * 已经可以支持多个文件的维护
 *
 * 但是还是建议大家只维护一个文件，因为首先没有必要，我们可以把数据库中
 * 的索引和数据都放在同一个文件中，当我们开启一个数据库时，就关掉上一个
 * 其次，多个文件就要对应多个fileID，在BufPageManager中是利用一个hash函数
 * 将(fileID,pageID)映射为一个整数，但由于我设计的hash函数过于简单，就是fileID和
 * pageID的和，所以不同文件的页很有可能映射为同一个数，增加了hash的碰撞率，影响效率
 * 
 * 还有非常重要的一点，BufType b = bpm->allocPage(...)
 * 在利用上述allocPage函数或者getPage函数获得指向申请缓存的指针后，
 * 不要自行进行类似的delete[] b操作，内存的申请和释放都在BufPageManager中做好
 * 如果自行进行类似free(b)或者delete[] b的操作，可能会导致严重错误
 */
#include <iostream>
#include "Record/Database.h"
#include "Utils/Constraints.h"
#include "FileIO/FileManager.h"
#include "BufManager/BufPageManager.h"
using namespace std;

int main() {

#ifdef MACRO_NON_DEFINED
	FileManager* fm = new FileManager();
	BufPageManager* bpm = new BufPageManager(fm);
	fm->createFile("testfile.txt"); //新建文件
	fm->createFile("testfile2.txt");
	int fileID, f2;
	fm->openFile("testfile.txt", fileID); //打开文件，fileID是返回的文件id
	fm->openFile("testfile2.txt", f2);
	for (int pageID = 0; pageID < 1000; ++ pageID) {
		int index;
		//为pageID获取一个缓存页
		BufType b = bpm->allocPage(fileID, pageID, index, false);
		//注意，在allocPage或者getPage后，千万不要进行delete[] b这样的操作
		//内存的分配和管理都在BufPageManager中做好，不需要关心，如果自行释放会导致问题
		b[0] = pageID; //对缓存页进行写操作
		b[1] = fileID;
		bpm->markDirty(index); //标记脏页
		//在重新调用allocPage获取另一个页的数据时并没有将原先b指向的内存释放掉
		//因为内存管理都在BufPageManager中做好了
		b = bpm->allocPage(f2, pageID, index, false);
		b[0] = pageID;
		b[1] = f2;
		bpm->markDirty(index);
		bpm->writeBack(index);
	}
	for (int pageID = 0; pageID < 1000; ++ pageID) {
		int index;
		//为pageID获取一个缓存页
		BufType b = bpm->getPage(fileID, pageID, index);
		//注意，在allocPage或者getPage后，千万不要进行delete[] b这样的操作
		//内存的分配和管理都在BufPageManager中做好，不需要关心，如果自行释放会导致问题
		cout << b[0] << ":" << b[1] << endl; 		//读取缓存页中第一个整数
		bpm->access(index); //标记访问
		b = bpm->getPage(f2, pageID, index);
		cout << b[0] << ":" << b[1] << endl;
		bpm->access(index);
	}
	// 测试删除
	if(fm->removeFile("testfile.txt")){
		cout << "删除文件 testfile.txt 失败" << endl;
	} else {
		cout << "已删除文件 testfile.txt " << endl;
	}
	//程序结束前可以调用BufPageManager的某个函数将缓存中的内容写回
	//具体的函数大家可以看看ppt或者程序的注释
#endif

	Global::fm = new FileManager();
	Global::bpm = new BufPageManager(Global::fm);
	Database::LoadDatabases();

	Database::CreateDatabase("TestDatabase");
	Database::OpenDatabase("TestDatabase");

	TableHeader *tableHeader = new TableHeader();
	tableHeader->AddField(Field("a", Data(Data::INTEGER)));
	tableHeader->AddField(Field("b", Data(Data::INTEGER)));
	tableHeader->AddField(Field("c", Data(Data::CHAR, 40)));
	Table *table = Database::CreateTable("TestTable", tableHeader);
	delete tableHeader;

	Record *record = table->CreateEmptyRecord();
	for(int i = 0; i < 128; i++) {
		record->CleanData();
		record->FillData(0, Data(Data::INTEGER).SetData(0x003e2590));
		record->FillData(1, Data(Data::INTEGER).SetData(0x23333333));
		if(rand() & 1)
			record->FillData(2, Data(Data::CHAR, 40).SetData("I think something is really happening.  "));
		table->AddRecord(record);
	}
	delete record;
	
	table->PrintTable();

	Global::bpm->close();
	return 0;
}

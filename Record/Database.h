#pragma once
#include <map>
#include <set>
#include <vector>
#include "Table.h"
using namespace std;

void RemoveDirectory(const char* dir);

class Database {
public:
	static set<string> databases; // 全局的数据库名称信息（数据库不会嵌套或者重名）
	static Database *currentDatabase; // 当前打开的数据库
	char databaseName[MAX_IDENTIFIER_LEN + 1];
	map<string, Table*> tables;
	/**
	 * 打印所有的数据库名称
	 */
	static void ShowDatabases();
	/**
	 * 清空当前数据库储存的信息
	 * 从 Database 目录下重新读取目录作为全局的数据库信息
	 * 	*/
	static void LoadDatabases();
	/**
	 * 在 DataBase 目录下创建一个名为 databaseName 的目录作为一个数据库
	 * @param databaseName 要创建的数据库的名字
	 * */
	static void CreateDatabase(const char *databaseName);
	/**
	 * 删除 DataBase 目录下名为 databaseName 的数据库目录
	 * @param databaseName 要删除的数据库的名字
	 * */
	static void DropDatabase(const char *databaseName);
	/**
	 * 根据提供的数据库名读取并生成数据库对象
	 * currentDatabase会指向这个最新的数据库对象
	 * @param databaseName 要打开的数据库名称
	 * */
	static void OpenDatabase(const char *databaseName);
	/**
	 * 关闭当前打开的数据库
	 * 如果没有打开的数据库则报错并退出
	*/
	static void CloseDatabase();
	/**
	 * 打印当前数据库中所有的数据库名称
	 */
	static void ShowTables();
	/**
	 * 在当前打开的数据库中创建一个新的表
	 * 如果当前没有打开数据库或是表已经存在则会报错退出
	 * @param tableName 表名
	 * @param fieldList 域列表
	 * */
	static Table* CreateTable(const char *tableName, FieldList fieldList);
	/**
	 * 从当前打开的数据库中获得某个 table 对象指针
	 * 如果当前数据库不存在或者表名不存在则会直接报错退出
	 * @param tableName 表名
	 * @return Table* 指向该表的指针
	 * */
	static Table* GetTable(const char *tableName);
	static void Insert(const char *tableName, vector<vector<Data> > dataLists);
};



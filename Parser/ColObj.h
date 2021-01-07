//! 用于在 parser 中传递 col 变量数据的结构体
#ifndef __COL_OBJ_H__
#define __COL_OBJ_H__
#include <string>
#include <iostream>
#include <map>
using namespace std;
class Table;
class Field;
class FieldList;
struct ColObj {
    string tbName;
    string colName;
    ColObj(){}
    ColObj(const ColObj& other) {
        tbName = other.tbName;
        colName = other.colName;
    }
    ColObj& operator = (const ColObj& other) {
        tbName = other.tbName;
        colName = other.colName;
        return *this;
    }
    void print() {
        std::cout << "Col{tbName:" << tbName << ", colName: " << colName << " }" << std::endl;
    }
    /**
     * 不会抛出错误
     * 如果该列在给定的表内则返回真值
     * */
    bool isInTable(const Table&) const;
    /** 从目标表中找到 col 所对应的 Field
     * 如果解析错误不会 throw 错误，而会返回 nullptr
     * 如果找到则会返回指针
     * */
    Field* getFieldOrNull(Table &);
    /** 
     * 在当前数据库中搜索字段是否存在
     * 不会抛出错误
     * */
    bool isExistInDB(Table* table = nullptr) const;
    /** 在当前数据库中搜索表与字段是否存在
     * 如果存在则会返回对应的 Field 的指针
     * 如果不存在则会返回 nullptr
     * 不会抛出异常
     * */
    Field* getFieldOrNullGlobal(Table* table = nullptr);

    /**
     * 判断 col 是否唯一存在于给定的 map 中
     * 如果当前 col 没有 tbName ，则这个函数会在 list 为其搜索到其对应的 tbName 并且赋值
     * @throws string err 如果发现 tbName 不存在且 colName 存在于两个表中，则抛出异常
     * */
    bool isInTbMap(const map<string, Table*> tbMap);
    /**
     * 根据自身的列名返回给定 FieldList 中对应列名的 Field
     * 不判断 tbName，仅对 colName 进行判断
     * @return Field* 如果 col 不存在则返回 nullptr
     * */
    Field* getFieldFromList(FieldList&);
};
#endif // __COL_OBJ_H__

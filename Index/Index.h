//* Index.h
//* 作者：马川
//* 功能：一个特定表的特定索引，利用 B+ 树进行管理
#ifndef __INDEX_H__
#define __INDEX_H__
#include "../Utils/BplusTree.h"
#include "../Record/Table.h"
template<typename K, typename T>
class Index {
public:
    Index(Table* table){

    }
private:
    BplusTree<K, T> tree;
};
#endif // __INDEX_H__
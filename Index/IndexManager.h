//* IndexManager.h
//* 作者：马川
//* 作用：将B+树作为文件进行创建、读取、储存和删除
#ifndef __INDEXMANAGER_H__
#define __INDEXMANAGER_H__
#include <vector>
#include "../FileIO/FileManager.h"
#include "../BufManager/BufPageManager.h"
#include "../Utils/BplusTree.h"
#include "../Utils/Global.h"
class IndexManager {
public:
    IndexManager(){
    }
    bool createIndex() {
        Global::fm->createFile("index.txt");
    }
    bool deleteIndex() {
        Global::fm->removeFile("index.txt");
    }
    bool openIndex(int& fileID) {
        Global::fm->openFile("index.txt", fileID);
    }
    bool closeIndex(int fileID) {
        Global::fm->closeFile(fileID);
    }
private:
};
#endif
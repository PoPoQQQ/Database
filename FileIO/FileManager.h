#pragma once
#include <string>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "../Utils/MyBitMap.h"
using namespace std;
class FileManager {
private:
	FILE* fd[MAX_FILE_NUM];
	MyBitMap* fm;
	MyBitMap* tm;
	int _createFile(const char* name) {
		FILE* f = fopen(name, "ab+");
		if (f == NULL) {
			cout << "fail" << endl;
			return -1;
		}
		fclose(f);
		return 0;
	}
	int _openFile(const char* name, int fileID) {
		FILE* f = fopen(name, "ab+");
		if (f == NULL) {
			return -1;
		}
		fd[fileID] = f;
		return 0;
	}
	/**
	 * 删除一个文件
	*/
	int _removeFile(const char* filename) {
		//int f = remove(filename);
		if(remove(filename) == 0){
			return 0;
		} else {
			return -1;
		}
	}
public:
	/*
	 * FilManager构造函数
	 */
	FileManager() {
		fm = new MyBitMap(MAX_FILE_NUM, 1);
		tm = new MyBitMap(MAX_TYPE_NUM, 1);
	}
	/*
	 * @函数名writePage
	 * @参数fileID:文件id，用于区别已经打开的文件
	 * @参数pageID:文件的页号
	 * @参数buf:存储信息的缓存(4字节无符号整数数组)
	 * @参数off:偏移量
	 * 功能:将buf+off开始的2048个四字节整数(8kb信息)写入fileID和pageID指定的文件页中
	 * 返回:成功操作返回0
	 */
	int writePage(int fileID, int pageID, BufType buf, int off) {
		FILE* f = fd[fileID];
		long offset = pageID;
		offset <<= PAGE_SIZE_IDX;
		if (fseek(f, offset, SEEK_SET) != 0)
			return -1;
		BufType b = buf + off;
		fwrite((void*)b, 1, PAGE_SIZE, f);
		return 0;
	}
	/*
	 * @函数名readPage
	 * @参数fileID:文件id，用于区别已经打开的文件
	 * @参数pageID:文件页号
	 * @参数buf:存储信息的缓存(4字节无符号整数数组)
	 * @参数off:偏移量
	 * 功能:将fileID和pageID指定的文件页中2048个四字节整数(8kb)读入到buf+off开始的内存中
	 * 返回:成功操作返回0
	 */
	int readPage(int fileID, int pageID, BufType buf, int off) {
		FILE* f = fd[fileID];
		long offset = pageID;
		offset <<= PAGE_SIZE_IDX;
		if (fseek(f, offset, SEEK_SET) != 0)
			return -1;
		BufType b = buf + off;
		return fread((void*)b, 1, PAGE_SIZE, f);
	}
	/*
	 * @函数名closeFile
	 * @参数fileID:用于区别已经打开的文件
	 * 功能:关闭文件
	 * 返回:操作成功，返回0
	 */
	int closeFile(int fileID) {
		fm->setBit(fileID, 1);
		FILE* f = fd[fileID];
		fclose(f);
		return 0;
	}
	/*
	 * @函数名createFile
	 * @参数name:文件名
	 * 功能:新建name指定的文件名
	 * 返回:操作成功，返回true
	 */
	bool createFile(const char* name) {
		return _createFile(name) == 0;
	}
	/*
	 * @函数名openFile
	 * @参数name:文件名
	 * @参数fileID:函数返回时，如果成功打开文件，那么为该文件分配一个id，记录在fileID中
	 * 功能:打开文件
	 * 返回:如果成功打开，在fileID中存储为该文件分配的id，返回true，否则返回false
	 */
	bool openFile(const char* name, int& fileID) {
		fileID = fm->findLeftOne();
		if(_openFile(name, fileID) == 0) {
			fm->setBit(fileID, 0);
			return true;
		}
		else 
			return false;
	}
	bool removeFile(const char* name) {
		return _removeFile(name) == 0; 
	}
	int newType() {
		int t = tm->findLeftOne();
		tm->setBit(t, 0);
		return t;
	}
	void closeType(int typeID) {
		tm->setBit(typeID, 1);
	}
	void shutdown() {
		delete tm;
		delete fm;
	}
	~FileManager() {
		this->shutdown();
	}
};

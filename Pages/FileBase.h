//! FileBase
//! 所有与文件读写有关的类的基类，负责相关类对应文件和缓存的处理
#pragma once
#include "../Pages/PageBase.h"
#include "../Utils/MyBitMap.h"
#include "../BufManager/BufPageManager.h"
class FileBase {
public:
	int fileID;
	int numberOfPage;
	string fileDirectory;
	int pageIndex;
	int bitMapPage;

	FileBase(string fileDirectory, bool createFile);
	virtual ~FileBase();

	PageBase* GetAvailablePage(int pageType);
	PageBase* LoadPage(int pageNumber);
	PageBase* CreatePage(int pageType);
	void FreePage(int pageNumber);

	void SetBit(int index, int bit);
	int FindLeftOne();

protected:
	virtual void LoadHeader() = 0;
	virtual void SaveHeader() const = 0;
	BufType GetHeaderBufType() const;
	void MarkDirty() const;
	/**
	 * 强制要求将自身的内容从 Buffer 中写回文件
	 * 请自行调用该函数处理相关的 Buffer 写回问题
	 * FileBase 类仅保证与类绑定的文件在构造和析构的时候同步地打开和关闭
	 * */
	void ForceWriteBack();
};

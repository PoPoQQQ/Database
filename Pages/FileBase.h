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
	MyBitMap *bitMap;
	FileBase(string fileDirectory, bool createFile);
	~FileBase();
	PageBase* GetAvailablePage(int pageType);
	PageBase* LoadPage(int pageNumber);
	PageBase* CreatePage(int pageType);
	void FreePage(int pageNumber);
protected:
	virtual void LoadHeader() = 0;
	virtual void SaveHeader() const = 0;
	BufType GetHeaderBufType() const;
	void MarkDirty() const;
};

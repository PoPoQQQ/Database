#pragma once
/*
全局变量将会被储存在这里
*/
class FileManager;
class BufPageManager;
class Global{
public:
	FileManager* fm;
	BufPageManager* bpm;
	static Global* getInstance();
private:
	Global();
	Global(const Global&);
	~Global();
	Global& operator=(const Global&);

	static Global *instance;
};

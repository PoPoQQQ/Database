#pragma once
/*
全局变量将会被储存在这里
*/
class FileManager;
class BufPageManager;
namespace Global {
	FileManager* fm = NULL;
	BufPageManager* bpm = NULL;
};

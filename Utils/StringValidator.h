#pragma once
#include <cstdio>
#include <cstring>
#include "../Utils/Constraints.h"
using namespace std;

class StringValidator {
public:
	/**
	 * 检查一个字符串是否满足数据库的规定
	 * 要求一个字符串只能是[0-9]|[a-z]|[A-Z]|_的组合
	 * 如果不满足条件则直接报错退出
	 * @param str const char * 
	 * */
	static void Check(const char *str) {
		int len = strlen(str);
		if(len <= 0 || len > MAX_STRING_LEN)
		{
			cerr << "String length invalid!" << endl;
			exit(-1);
		}
		for(int i = 0; str[i]; i++)
		{
			if(i != 0 && str[i] >= '0' && str[i] <= '9')
				continue;
			if(str[i] >= 'a' && str[i] <= 'z')
				continue;
			if(str[i] >= 'A' && str[i] <= 'Z')
				continue;
			if(str[i] == '_')
				continue;
			cerr << "Invalid name syntax!" << endl;
			exit(-1);
		}
	}
};

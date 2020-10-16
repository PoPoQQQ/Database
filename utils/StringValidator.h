#pragma once
#include <cstdio>
#include <cstring>
#include "../Utils/Constraints.h"
using namespace std;

class StringValidator {
public:
	static void Check(const char *str) {
		int len = strlen(str);
		if(len <= 0 || len > MAX_STRING_LEN)
		{
			cerr << "String length invalid!" << endl;
			exit(-1);
		}
		for(int i = 0; str[i]; i++)
		{
			if(str[i] >= '0' && str[i] <= '9')
				continue;
			if(str[i] >= 'a' && str[i] <= 'z')
				continue;
			if(str[i] >= 'A' && str[i] <= 'Z')
				continue;
			if(str[i] == '_')
				continue;
			cerr << "Invalid character detected!" << endl;
			exit(-1);
		}
	}
};

#include "utils/MyBitMap.h"
#include <iostream>

using namespace std;

int _main()
{
	MyBitMap *m = new MyBitMap(16);
	for(int i = 0; i < 16; i++)
		cout << m->getBit(i) << endl;
	cout << endl;
	for(int i = 0; i < 16; i++)
		if(i % 3 != 0)
			m->setBit(i, 1);
	for(int i = 0; i < 16; i++)
		cout << m->getBit(i) << endl;
	return 0;
}

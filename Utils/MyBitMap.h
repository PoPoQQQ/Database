#pragma once
typedef unsigned int uint;
/*
#define LEAF_BIT 32
#define MAX_LEVEL 5
#define MAX_INNER_NUM 67
#define MOD 61
#define BIAS 5*/
#include <iostream>
#include <string.h>
using namespace std;

#define LEAF_BIT 32
#define MAX_LEVEL 5
#define MAX_INNER_NUM 67
//#define MOD 61
#define BIAS 5
extern unsigned char h[61];


class MyBitMap {
protected:
//	static const int LEAF_BIT = 32;
//	static const int MAX_LEVEL = 5;
//	static const int MAX_INNER_NUM = 10;
//	static const int MOD = 61;
//	static unsigned char h[MOD];
	static uint getMask(int k) {
		uint s = 0;
		for (int i = 0; i < k; ++ i) {
			s += (1 << i);
		}
		return s;
	}
	uint* data;
	int size;
	int rootBit;
	int rootLevel;
	int rootIndex;
	uint inner[MAX_INNER_NUM];
	uint innerMask;
	uint rootMask;
	//virtual
	uint getLeafData(int index) {
		return data[index];
	}
	//virtual
	void setLeafData(int index, uint v) {
		data[index] = v;
	}
	int getLeafBit(int index) {
		int pos, bit;
		getPos(index, pos, bit);
		uint umask = (1 << bit);
		uint w = getLeafData(pos) & umask;
		return w ? 1 : 0;
	}
	int setLeafBit(int index, uint k) {
		int pos, bit;
		getPos(index, pos, bit);
		uint umask = (1 << bit);
		uint mask = (~umask);
		if (k == 0) {
			umask = 0;
		}
		uint w = ((getLeafData(pos) & mask) | umask);
		setLeafData(pos, w);
		return pos;
	}
	uint childWord(int start, int bitNum, int i, int j) {
		//cout << start << " " << bitNum << " " << i << " " << j << endl;
		int index = (i << BIAS) + j;
		if (start == 0) {
			return getLeafData(index);
		} else {
			//cout << start - bitNum + index << endl;
			return inner[start - bitNum + index];
		}
	}
	void init() {
		MyBitMap::initConst();   //新加的初始化
		rootLevel = 0;
		int s = size;
		rootIndex = 0;
		while (s > LEAF_BIT) {
			int wordNum = (s >> BIAS);
			//cout << rootIndex << " " << s << " " << wordNum << endl;
			for (int i = 0; i < wordNum; ++ i) {
				uint w = 0;
				//cout << "---------------------------------------" << endl;
				for (int j = 0; j < LEAF_BIT; ++ j) {
					//cout << i << endl;
					uint k = (1 << j);
					uint c = childWord(rootIndex, s, i, j);
					if (c != 0) {
						w += k;
					}
				}
				inner[rootIndex + i] = w;
			}
			rootLevel ++;
			rootIndex += wordNum;
			s = wordNum;
		}
		rootBit = s;
		int i = 0;
		uint w = 0;
		for (int j = 0; j < rootBit; ++ j) {
			uint k = (1 << j);
			uint c = childWord(rootIndex, s, i, j);
			if (c != 0) {
				w += k;
			}
		}
		inner[rootIndex] = w;
		innerMask = getMask(BIAS);
		rootMask = getMask(s);
	}
	int _setBit(uint* start, int index, uint k) {
		int pos, bit;
		getPos(index, pos, bit);
		uint umask = (1 << bit);
		uint mask = (~umask);
		if (k == 0) {
			umask = 0;
		}
		start[pos] = ((start[pos] & mask) | umask);
		return pos;
	}
	void updateInner(int level, int offset, int index, int levelCap, uint k) {
		//cout << level << " " << rootLevel << endl;
		uint* start = (&inner[offset]);
		int pos = _setBit(start, index, k);
		if (level == rootLevel) {
			return;
		}
		uint c = 1;
		if (start[pos] == 0) {
			c = 0;
		}
		/*
		if (level == 1) {
			cout << "level1:" << start[index] << " " << c << endl;
		}*/
		updateInner(level + 1, offset + levelCap, pos, (levelCap >> BIAS), c);
	}
	int _findLeftOne(int level, int offset, int pos, int prevLevelCap) {
		//cout << inner[offset + pos] << endl;
		uint lb = lowbit(inner[offset + pos]);
		int index = h[_hash(lb)];
		/*if (level == 0) {
			cout << "level0:" << index << " " << pos << endl;
		}*/
		int nPos = (pos << BIAS) + index;
		if (level == 0) {
		//	cout << "npos " << nPos << endl;
			return nPos;
		}
		return _findLeftOne(level - 1, offset - prevLevelCap, nPos, (prevLevelCap << BIAS));
	}
public:
//	static const int BIAS;/* = 5;*/
//	static void initConst();
	/* {
		for (int i = 0; i < 32; ++ i) {
			unsigned int k = (1 << i);
			MyBitMap::h[MyBitMap::_hash(k)] = i;
		}
	}
	*/
	static int _hash(uint i) {
		return i % 61;
	}
	static void initConst() {
		for (int i = 0; i < 32; ++ i) {
			unsigned int k = (1 << i);
			h[_hash(k)] = i;
		}
	}
	static int getIndex(uint k)
	{
		return h[_hash(k)];
	}
	static uint lowbit(uint k) {
		return (k & (-k));
	}
	static void getPos(int index, int& pos, int& bit) {
		pos = (index >> BIAS);
		bit = index - (pos << BIAS);
	}
	static int getSize(int cap) {
		uint mask = (1 << BIAS) - 1;
		return (cap >> BIAS) + (cap & mask ? 1 : 0);
	}
	uint data0(){
		return data[0];
	}
	int getBit(int index) {
		return getLeafBit(index);
	}
	void setBit(int index, uint k) {
		//cout << data[0]<<endl;
		int p = setLeafBit(index, k);
		//cout <<"seting "<<data[0]<<endl;
		//cout << "ok" << endl;
		uint c = 1;
		if (getLeafData(p) == 0) {
			c = 0;
		}
		//cout << p << " " << c << endl;
		updateInner(0, 0, p, (size >> BIAS), c);
	}
	int findLeftOne() {
		if(inner[rootIndex] == 0)
			return -1;
		int i = _findLeftOne(rootLevel, rootIndex, 0, rootBit);
		/*
		for (i = 0; i < size;++i){
			if (data[i] !=0)break;
		}*/
		//cout << "nPosi " << i << " " << getLeafData(i) << endl;
		//cout << i << endl;
		//cout << data[0] << endl;
		uint lb = lowbit(getLeafData(i));
		int index = h[_hash(lb)];
		return (i << BIAS) + index;
	}
	void save(uint* da) {
		memcpy(da, data, size * sizeof(uint));
	}
	MyBitMap(int cap, uint k = 0) {
		size = getSize(cap);
		data = new uint[size];
		uint fill = 0;
		if (k == 1) {
			fill = 0xffffffff;
		}
		for (int i = 0; i < size; ++ i) {
			data[i] = fill;
		}
		init();
	}
	MyBitMap(int cap, const uint* da) {
		size = getSize(cap);
		data = new uint[size];
		memcpy(data, da, size * sizeof(uint));
		init();
	}
	~MyBitMap() {
		delete[] data;
	}
	/*
	void reLoad(uint* da) {
		data = da;
	}
	*/
};

#pragma once
#include <errno.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * 一个页面中的字节数
 */
#define PAGE_SIZE 8192
/*
 * 一个页面中的整数个数
 */
#define PAGE_INT_NUM 2048
/*
 * 页面字节数以2为底的指数
 */
#define PAGE_SIZE_IDX 13
#define MAX_FMT_INT_NUM 128
//#define BUF_PAGE_NUM 65536
#define MAX_FILE_NUM 128
#define MAX_TYPE_NUM 256
/*
 * 缓存中页面个数上限
 */
#define CAP 60000
/*
 * hash算法的模
 */
#define MOD 60000
#define IN_DEBUG 0
#define DEBUG_DELETE 0
#define DEBUG_ERASE 1
#define DEBUG_NEXT 1
/*
 * 一个表中列的上限
 */
#define MAX_COL_NUM 31
/*
 * 数据库中表的个数上限
 */
#define MAX_TB_NUM 31
/*
 * 数据库中字符串的长度上限
 */
#define MAX_STRING_LEN 96
/*
 * 数据库中记录长度的下限
 */
#define MIN_RECORD_SIZE 64

#define RELEASE 1
typedef unsigned int* BufType;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long long ull;
typedef long long ll;
typedef double db;
typedef int INT;
typedef int(cf)(uchar*, uchar*);
int current = 0;
int tt = 0;

enum DataType {
	TINYINT = 1,
	SMALLINT = 2,
	MEDIUMINT = 3,
	INTEGER = 4,
	BIGINT = 5,
	FLOAT = 6,
	DOUBLE = 7,
	DECIMAL = 8,
	
	DATE = 9,
	TIME = 10,
	YEAR = 11,
	DATETIME = 12,
	TIMESTAMP = 13,

	CHAR = 14,
	VARCHAR = 15,
	TINYBLOB = 16,
	TINYTEXT = 17,
	BLOB = 18,
	TEXT = 19,
	MEDIUMBLOB = 20,
	MEDIUMTEXT = 21,
	LONGBLOB = 22,
	LONGTEXT = 23
};

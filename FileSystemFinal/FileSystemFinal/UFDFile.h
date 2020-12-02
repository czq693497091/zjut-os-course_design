#pragma once

#define MODE_N 0 // 不可读也不可写
#define MODE_R 1 // 只读
#define MODE_W 2 // 只写
#define MODE_RW 3 // 可读可写
#define MODE_OVERWRITE 0 // 覆盖
#define MODE_APPEND 1 // 添加到尾部
#include<stdio.h>
#include<string>

// 2#－17#物理块:固定用于存放用户文件目录UFD。
class UFDFile {
	char filename[20];
	int mode;
	int size;
	int blockptr;

public:
	UFDFile();
	UFDFile(const char *, int, int, int);
	~UFDFile();
	char * getFilename();
	int getBlockptr();
	int getMode();
	int getSize();
	void setFilename(const char *);
	void setMode(int);
	void setSize(int);
	void setBlockptr(int);
};
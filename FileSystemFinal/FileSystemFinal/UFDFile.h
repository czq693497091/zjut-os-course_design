#pragma once

#define MODE_N 0 // ���ɶ�Ҳ����д
#define MODE_R 1 // ֻ��
#define MODE_W 2 // ֻд
#define MODE_RW 3 // �ɶ���д
#define MODE_OVERWRITE 0 // ����
#define MODE_APPEND 1 // ��ӵ�β��
#include<stdio.h>
#include<string>

// 2#��17#�����:�̶����ڴ���û��ļ�Ŀ¼UFD��
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
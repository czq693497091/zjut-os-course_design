#pragma once
#include <stdio.h>
#include <string>

/*
34＃－100＃：数据块（物理块），用于存放文件内容；
为了实现物理块的分配和回收，程序始终维护一个空闲物理块表，
以物理块号从小到大排列。物理块以链接分配方式，
以最先适应法从空闲表中分配。
*/

class Block {
	int next;
	char data[508];
public:
	Block();
	~Block();
	int getNext();
	void setNext(int);
	char * getData();
	void setData(const char *);
};
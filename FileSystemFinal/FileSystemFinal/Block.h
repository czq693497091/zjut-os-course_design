#pragma once
#include <stdio.h>
#include <string>

/*
34����100�������ݿ飨����飩�����ڴ���ļ����ݣ�
Ϊ��ʵ�������ķ���ͻ��գ�����ʼ��ά��һ������������
�������Ŵ�С�������С�����������ӷ��䷽ʽ��
��������Ӧ���ӿ��б��з��䡣
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
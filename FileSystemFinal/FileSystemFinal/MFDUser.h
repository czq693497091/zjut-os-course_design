#pragma once
#include<stdio.h>
#include<string>


// 1#��Ӳ�̵ĵ�1�������̶����ڴ�����ļ�Ŀ¼MFD��
// ���û����˺������Լ�UFDָ�롣
class MFDUser {

	char username[14];//�û���14B
	char password[14];//����14B
	int UFDptr;//ָ��UFD���������� 4B
public:
	MFDUser();
	MFDUser(const char *, const char *, int);
	~MFDUser();
	char *getUsername();
	char *getPassword();
	int getUFDptr(); 
	void setUsername(const char *);
	void setPassword(const char *);

};
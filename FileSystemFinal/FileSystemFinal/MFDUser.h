#pragma once
#include<stdio.h>
#include<string>


// 1#，硬盘的第1个物理块固定用于存放主文件目录MFD。
// 即用户的账号密码以及UFD指针。
class MFDUser {

	char username[14];//用户名14B
	char password[14];//密码14B
	int UFDptr;//指向UFD所在物理块号 4B
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
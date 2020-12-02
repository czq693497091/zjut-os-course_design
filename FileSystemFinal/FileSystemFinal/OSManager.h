#pragma once

//定义二级文件管理系统

#define BLOCKSIZE 512

#include<stdio.h>
#include<iostream>
#include<fstream>
#include<vector>
#include<sstream>
#include"MFDUser.h"
#include"UFDFile.h"
#include"Block.h"
#include"Return.h"

using namespace std;

extern vector<MFDUser> MFD;//定义用户列表
extern UFDFile UFDs[16][16];
extern Block Blocks[256];
extern bool UFDistaken[16];
extern bool Blockistaken[256];

class OSManager {
private:
	FILE * stream;
	void split(vector<string>&, const string&, int);//切割字符串
	void UFDRead(MFDUser mfduser);
	void FileRead(UFDFile ufdfile, bool onlyPtr);
	void MFDWrite();
	void UFDWrite(MFDUser mfduser);
	void FileWrite(UFDFile ufdfile);
	void removeBlocks(int amount, vector<int>& itakenBlocks);
	void removeFile(int, UFDFile *);
	int findBlocks(int UFDindex, int amount, vector<int>& itakenBlocks);
	

public:
	OSManager();
	~OSManager();
	string user;//当前用户
	string directory;//当前目录

	void open_write();
	void open_read();
	void close();

	int findUser(const char *);
	void MFDRead();

	int useradd(const char *, const char *);//添加用户
	int userdel(const char *);//删除用户
	int usermod(const char *, const char *);//修改密码
	int touch(const char *);//新建文件
	int su(int index, const char*);//登录
	int rm(const char *);//删除文件
	int mv(const char *, const char *);//修改文件名称
	int cd(const char*);//目录切换
	vector<string> ls();//列出当前目录下的文件
	int chmod(const char *, int);//设置文件权限
	int cat(const char *);//读取文件内容
	int write(const char *, string, int);//向文件中写入信息
	
	int cp(const char *, const char *);//复制文件
	int cpd(const char*, const char*);//复制目录
	int chown(const char*, const char*);

	void logout();//退出
	int reformat();//初始化硬盘
	
};

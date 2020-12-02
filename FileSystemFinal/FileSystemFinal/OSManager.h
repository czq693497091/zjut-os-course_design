#pragma once

//��������ļ�����ϵͳ

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

extern vector<MFDUser> MFD;//�����û��б�
extern UFDFile UFDs[16][16];
extern Block Blocks[256];
extern bool UFDistaken[16];
extern bool Blockistaken[256];

class OSManager {
private:
	FILE * stream;
	void split(vector<string>&, const string&, int);//�и��ַ���
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
	string user;//��ǰ�û�
	string directory;//��ǰĿ¼

	void open_write();
	void open_read();
	void close();

	int findUser(const char *);
	void MFDRead();

	int useradd(const char *, const char *);//����û�
	int userdel(const char *);//ɾ���û�
	int usermod(const char *, const char *);//�޸�����
	int touch(const char *);//�½��ļ�
	int su(int index, const char*);//��¼
	int rm(const char *);//ɾ���ļ�
	int mv(const char *, const char *);//�޸��ļ�����
	int cd(const char*);//Ŀ¼�л�
	vector<string> ls();//�г���ǰĿ¼�µ��ļ�
	int chmod(const char *, int);//�����ļ�Ȩ��
	int cat(const char *);//��ȡ�ļ�����
	int write(const char *, string, int);//���ļ���д����Ϣ
	
	int cp(const char *, const char *);//�����ļ�
	int cpd(const char*, const char*);//����Ŀ¼
	int chown(const char*, const char*);

	void logout();//�˳�
	int reformat();//��ʼ��Ӳ��
	
};

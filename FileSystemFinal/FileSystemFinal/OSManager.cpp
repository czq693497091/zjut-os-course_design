#include "pch.h"
#include "OSManager.h"

OSManager::OSManager() {
	this->user = "/";
	this->directory = "/";
}

OSManager::~OSManager() {

}

/**
* 下划线分区以上的9个函数，主要用于和磁盘之间的读取和写入
*	1.在读或写之前，请先执行open_write()或者open_read()函数，目的是打开文件，选择读取方式
*	2.在读或写之后，执行close()关闭文件
*/

// 该函数主要用于将文件流写入到#1区域的MFD用户信息
void OSManager::MFDRead() {
	MFDUser mfdusers[16];
	fseek(stream, 0L, SEEK_SET);

	// 共16个用户，每个用户32字节，一共512字节，在读的时候，连同末尾的换行符读入
	fread(mfdusers, BLOCKSIZE, 1, stream);
	for (int i = 0; i < 16; i++) {

		// 只将磁盘中，存在的用户，加入到MFD队列中，并对UFDistaken表示添加注解
		if (strlen(mfdusers[i].getUsername()) == 0 || mfdusers[i].getUsername()[0] == ' ')
			continue;
		MFD.push_back(mfdusers[i]);

		// 只要这个用户存在，则将他归属的UFD分区打上已使用标志
		UFDistaken[mfdusers[i].getUFDptr()] = true;
	}
}

// 该函数目的，是读取#2-16区域中的文件目录索引信息
void OSManager::UFDRead(MFDUser mfduser) {
	int UFDptr = mfduser.getUFDptr();
	// 每个UFD有32B,对于每一个用户，至多存放16个文件。
	// 相当于16*32=512B，加上最后换行符共513B
	// 最后加上的513B是要先跳过#1区
	long offset = UFDptr * 513 + 513;
	fseek(stream, offset, SEEK_SET);
	fread(&UFDs[UFDptr], BLOCKSIZE, 1, stream);
	for (int i = 0; i < 16; i++) {
		if (UFDs[UFDptr][i].getFilename()[0] != ' ') // 非空就是已经被使用
			//将指定用户目录下，指定文件的指定块的使用情况标记为已使用
			Blockistaken[UFDs[UFDptr][i].getBlockptr()] = true;
	}
}


// 该函数主要用于读取#17-#273区域中指定文件目录索引下的所有文件内容，写入到Blocks中
void OSManager::FileRead(UFDFile ufdfile, bool onlyPtr) {
	int blockptr = ufdfile.getBlockptr();

	// 只要某个文件目录索引下的文件，Block被使用，则从文件流中读入
	while (Blockistaken[blockptr]) {
		//由于UOF区在从#17开始，每部分数据有512B，加上尾部后缀1B，以513位单位跳动
		long offset = blockptr * 513 + 17 * 513;

		//从指定偏移开始读入文件流,SEEK_SET是指从文件开头开始偏移
		fseek(stream, offset, SEEK_SET);

		// 从文件流stream中，读取1次4字节读入，读到对应的Blocks[blockptr]中
		// 1次读4字节，表示只读指针(Block地址)
		if (onlyPtr) fread(&Blocks[blockptr], 4, 1, stream);
		// 否则就是指针连同内容一起读入，只有写回的时候需要如此
		else fread(&Blocks[blockptr], BLOCKSIZE, 1, stream);
		blockptr = Blocks[blockptr].getNext();//获取连续空间的下一个地址
	}
}

// 该函数将用户信息部分写入到磁盘当中
void OSManager::MFDWrite() {
	MFDUser *mfdusers = new MFDUser[MFD.size()];

	//将当前已经在用户列表中的内容，存储到mfdusers数组中
	memcpy(mfdusers, &MFD[0], MFD.size() * sizeof(MFDUser));
	int validSize = int(MFD.size() * sizeof(MFDUser));
	char * blank = new char[BLOCKSIZE - validSize];//两者长度之差，就是没有填满的部分
	for (int i = 0; i < BLOCKSIZE - validSize; i++)
		blank[i] = ' ';

	//准备将用户信息写回磁盘，获取文件流，从文件开始写入
	fseek(stream, 0L, SEEK_SET);
	
	//分别将用户信息、空白部分、以及尾部后缀\n写入到磁盘当中
	fwrite(mfdusers, validSize, 1, stream);
	fwrite(blank, BLOCKSIZE - validSize, 1, stream);
	fwrite("\n", 1, 1, stream);
	delete[] blank;
	delete[] mfdusers;
}

// 该函数将目录索引信息写入到磁盘当中
void OSManager::UFDWrite(MFDUser mfduser) {
	int UFDptr = mfduser.getUFDptr();

	// 将偏移量跳转到当前用户的目录索引起始地址下
	long offset = UFDptr * 513 + 513;
	fseek(stream, offset, SEEK_SET);
	fwrite(&UFDs[UFDptr], BLOCKSIZE, 1, stream);
	fwrite("\n", 1, 1, stream);
}

// 该函数将文件中，即连续的block内容写到磁盘当中
void OSManager::FileWrite(UFDFile ufdfile) {
	int blockptr = ufdfile.getBlockptr();
	while (Blockistaken[blockptr]) {
		long offset = blockptr * 513 + 17 * 513;//每个block有513B，加上从#17开始
		fseek(stream, offset, SEEK_SET);
		fwrite(&Blocks[blockptr], BLOCKSIZE, 1, stream);
		fwrite("\n", 1, 1, stream);
		blockptr = Blocks[blockptr].getNext();
	}
}

// 写文件，获取文件流
void OSManager::open_write() {

	// 以二进制形式打开，只允许读写
	stream = fopen("disk.txt", "rb+");
	if (stream == NULL) {
		perror("Open file disk.txt");
		exit(1);
	}
}

// 读文件，获取文件流
void OSManager::open_read() {
	// 以二进制方式，只读打开磁盘
	stream = fopen("disk.txt", "rb");
	if (stream == NULL) {
		perror("Open file disk.txt");
		exit(1);
	}
}

void OSManager::close() {
	fclose(stream);
}
/*----------------------------------------------------------------------------------------------------------*/



//将数据切成508B每一块，装入contents列表中
//因为Block大小为512B，数据部分508B，还有指向下一个地址的int类型4B。
void OSManager::split(vector<string>& contents, const string& content, int len) {
	int begin = 0;
	while (begin != content.length()) {
		int len = 507;
		if (begin + len > content.length())
			len = (int)content.length() - begin;
		contents.push_back(content.substr(begin, len));
		begin += len;
	}
}

/**
 * 删除文件，并在UFD块中做同步删除
 * The next index of the file block should be in memory
 * @param UFDindex 用户所在UFD区域的索引
 * @param ufdfile 文件指针
 */

void OSManager::removeFile(int UFDindex, UFDFile * ufdfile) {
	if (!UFDistaken[UFDindex]) return;//如果该块没有被占用，直接结束
	int firstptr = ufdfile->getBlockptr();//获得文件的起始地址
	
	// 设置文件名为空
	// ufdfile->setFilename(" ");
	
	//文件总长为508B，最后一位本来是留给'\0'的，但填充了'\n'，因此在记事本中显示乱码
	//将文件的实际大小减去1（减去尾部后缀），除507，加1，得到占用的总块数
	int amount = (ufdfile->getSize() - 1) / 507 + 1;
	vector<int> itakenBlocks;
	itakenBlocks.push_back(firstptr);
	int nextptr = firstptr;
	while (nextptr != -1) {
		// 往前迭代,将所有Blocks连续区域加入itakenBlocks
		// 最后再removeBlocks中，将所有加入的itakenBlocks的内容pop掉
		// 同时更新Blockistaken的状态
		if (Blocks[nextptr].getNext() != -1)
			itakenBlocks.push_back(Blocks[nextptr].getNext());
		nextptr = Blocks[nextptr].getNext();
	}
	removeBlocks(amount, itakenBlocks);
	memset(ufdfile, 0, 32);//删除掉之后，将对应目录索引内容全部置0
}

/**
* 使用最先适应算法，寻找指定数目空的块
* @param UFDindex UFD块的索引，定义查找区域
* @param amount 需要查找的块的数目
* @param itakenBlocks 该列表存储的是将要被使用的blocks
* @return SUCCESS 如果每一块都找到存储区域，返回成功，否则返回失败
*/

int OSManager::findBlocks(int UFDindex,int amount,vector<int>& itakenBlocks) {
	if (amount == 0) return SUCCESS;
	vector<int> notTaken;

	//在对应UFD目录下16个文件中进行遍历，每个文件中还未被使用的块
	for (int i = 16 * UFDindex; i < 16 * UFDindex + 16; i++) {
		if (!Blockistaken[i]) {//遍历找到对应空间内没被用的块的数目
			notTaken.push_back(i);
			if (notTaken.size() == amount) {
				itakenBlocks.insert(itakenBlocks.end(), notTaken.begin(), notTaken.end());
				for (int j = 0; j < notTaken.size(); j++) {
					Blockistaken[notTaken[j]] = true;//标志已经被使用
				}
				return SUCCESS;
			}
		}
	}
	return FAILED;
}

/**
 * 修改已经被占用Blocksistaken的标记为false
 * itakenBlocks清除掉已经占用的UFD编号
 * @param amount 待清除的block块
 * @param itakenBlocks 更新已经被使用的block表
 */

void OSManager::removeBlocks(int amount, vector<int>& itakenBlocks) {
	for (int i = 0; i < amount; i++) {
		Blockistaken[itakenBlocks.back()] = false;
		itakenBlocks.pop_back();
		if (itakenBlocks.size() > 0)
			//初始化已占用UFD编号的下一个地址为-1
			Blocks[itakenBlocks.back()].setNext(-1);
	}
}

/**
 * Find the user from MFD
 * @param username 待查找的username
 * @return 找到则返回user索引，否则-1
 */

int OSManager::findUser(const char username[14]) {
	for (int i = 0; i < MFD.size(); i++) {
		if (strcmp(username, MFD[i].getUsername()) == 0) {
			return i;
		}
	}
	return -1;
}

int OSManager::useradd(const char username[14],const char password[14]){
	//MFDRead();
	// 如果执行者不是管理员，提示权限不足
	if (strcmp(username, "root") != 0 && this->user != "root") return PERMISSION_DENIED;
	// 如果用户名或者密码为空，则提示字段错误
	if (strlen(username) == 0 || strlen(password) == 0) return WRONG_PARAMETER;
	// 密码长度小于3也无效
	if (strlen(password) < 3) return INVALID_PASSWORD;
	// 用户空间的上限是16
	if (MFD.size() == 16) return NOT_ENOUGH_STORAGE;
	if (findUser(username) != -1) return DUPLICATED;
	
	//找一个空的用户空间，将新用户填进去
	for (int i = 0; i < 16; i++) {
		if (!UFDistaken[i]) {
			MFDUser mfduser(username, password, i);
			UFDistaken[i] = true;
			MFD.push_back(mfduser);
			open_write();//打开文件
			MFDWrite();//写用户信息
			close();//关文件
			printf("%s%s%s\n", "User \"", username, "\" has been successfully created.");
			return SUCCESS;
		}
	}
	
	return UNKNOWN;
}

int OSManager::usermod(const char username[14],const char password[14]) {
	// 确保是在已经登录的状态下进行修改信息
	int userIndex = this->findUser(this->directory.data());
	if (userIndex == -1) return INVALID_OPERATION;
	int usernamelen = (int)strlen(username);
	int passwordlen = (int)strlen(password);
	if (usernamelen == 0 || passwordlen == 0) return WRONG_PARAMETER;
	bool update_username = false;
	if (usernamelen != 0) {
		// root的用户信息不可修改
		if (this->directory == "root") return OTHER_ERROR;
		// 用户名与原来一致也修改失败
		if (strcmp(username, this->user.data()) == 0) return UNKNOWN;
		// 修改后的用户名不能是已经存在的
		if (findUser(username) != -1) return DUPLICATED;

		update_username = true;
	}
	if (passwordlen != 0) {
		if(passwordlen < 3) return INVALID_PASSWORD;
		MFD[userIndex].setPassword(password);
	} 
	if (update_username) {
		MFD[userIndex].setUsername(username);
		// 更改后，立即生效
		if (this->user != "root") this->user = MFD[userIndex].getUsername();
		
		// 目录切换到
		this->directory = MFD[userIndex].getUsername();
	}
	open_write();
	MFDWrite();
	close();
	return SUCCESS;
}

int OSManager::userdel(const char username[20]) {
	// root是不能被删除的
	if (strcmp(username, "root") == 0) return OTHER_ERROR;

	// 只有是root权限，或者就是用户本人有权限进行删除操作
	if (this->user == "root" || strcmp(username, this->user.data()) == 0) {
		int userIndex = this->findUser(username);
		if (userIndex == -1) return NO_SUCH_DIRECTORY;

		//获取待删除用户的文件区域索引
		int UFDindex = MFD[userIndex].getUFDptr();
		open_write();

		//将该用户的所有文件删除，即所有block
		for (int i = 0; i < 16; i++) {
			UFDFile *ufdfile = &UFDs[UFDindex][i];
			if (ufdfile->getFilename()[0] != ' ') {// 对于所有存在的文件
				FileRead(UFDs[UFDindex][i], true);
				removeFile(UFDindex, ufdfile);
			}
		}
		UFDistaken[UFDindex] = false;
		UFDWrite(MFD[userIndex]);

		//在内存中，初始化被删除用户的列表信息
		memset(&MFD[userIndex], 0, sizeof(MFDUser));
		MFDWrite();
		close();
		this->directory = "/";
		if (this->user != "root")
			this->user = "/";

		//更新当前MFD的vector
		/*vector<MFDUser> MFDTemp;
		for (int i = 0; i < MFD.size();i++) 
			if(i!=userIndex) MFDTemp.push_back(MFD[i]);
		MFD = MFDTemp;*/

		return SUCCESS;
	}
	else return PERMISSION_DENIED;
}

// 切换用户名
int OSManager::su(int index,const char password[14]) {
	if (strcmp(password, MFD[index].getPassword()) == 0) {
		memset(UFDs, ' ', sizeof(UFDs));
		memset(Blocks, ' ', sizeof(Blocks));
		this->user = MFD[index].getUsername();
		this->directory = this->user;
		open_read();
		UFDRead(MFD[index]);
		close();
		return SUCCESS;
	}
	return INCORRECT_PASSWORD;
}

// 新建一个文件
int OSManager::touch(const char filename[20]) {
	int index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;
	if (strlen(filename) == 0) return WRONG_PARAMETER;
	int UFDindex = MFD[index].getUFDptr();
	for (int i = 0; i < 16; i++) {
		if (strcmp(filename, UFDs[UFDindex][i].getFilename()) == 0) return DUPLICATED;
	}
	for (int i = 0; i < 16; i++) {
		//cout << "Filename:" << strlen(UFDs[UFDindex][i].getFilename()) << ":" << UFDs[UFDindex][i].getFilename() << endl;
		if (UFDs[UFDindex][i].getFilename()[0] == ' ') {
			// 遍历该用户的所有文件索引
			
			for (int j = 16 * UFDindex; j < 16 * UFDindex + 16; j++) {
				if (!Blockistaken[j]) { // 如果该Block是空的

					// 初始化该Block
					memset(&Blocks[j], -1, 4); 
					memset((char*)(&Blocks[j]) + 4, ' ', BLOCKSIZE - 4);

					UFDs[UFDindex][i].setFilename(filename);
					UFDs[UFDindex][i].setMode(MODE_RW);// 设置文件可读可写
					UFDs[UFDindex][i].setSize(0); // 新建一个文件，没有写入内容，长度为0
					UFDs[UFDindex][i].setBlockptr(j);
					Blockistaken[j] = true;
					open_write();
					UFDWrite(MFD[index]);
					FileWrite(UFDs[UFDindex][i]);
					close();
					return SUCCESS;
				}
				/*if (j == 16 * UFDindex + 16)
					return NOT_ENOUGH_STORAGE;*/
			}
		}
	}
	return NOT_ENOUGH_STORAGE;
}

// 列出当前目录下的所有文件
vector<string> OSManager::ls() {
	vector<string> vec;
	if (this->directory == "/") {
		for (int i = 0; i < MFD.size(); i++) {
			vec.push_back(MFD[i].getUsername());
		}
	}
	else {
		int UFDindex = MFD[this->findUser(this->directory.data())].getUFDptr();
		for (int i = 0; i < 16; i++) {
			UFDFile *ufdfile = &UFDs[UFDindex][i];
			if (ufdfile->getFilename()[0] != ' ' || strlen(ufdfile->getFilename()) == 0) {
				stringstream ss;
				string out = "File name: ";
				out += UFDs[UFDindex][i].getFilename();
				out += "\tsize: ";
				ss << UFDs[UFDindex][i].getSize();
				string size, mode;
				ss >> size;
				out += size;
				out += "B\tmode: ";
				switch (UFDs[UFDindex][i].getMode())
				{
				case MODE_N:mode = "CAN NOT READ OR WRITE"; break;
				case MODE_R:mode = "ONLY READ"; break;
				case MODE_W:mode = "ONLY WRITE"; break;
				case MODE_RW:mode = "READ AND WRITE"; break;
				default:break;
				}
				out += mode;
				vec.push_back(out);
			}
		}
	}
	return vec;
}

// 向指定文件中写入内容
int OSManager::write(const char filename[20], string content, int mode) {
	int index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;
	if (strlen(filename) == 0 || content.empty())
		return WRONG_PARAMETER;
	if (mode != MODE_OVERWRITE && mode != MODE_APPEND)
		return WRONG_PARAMETER;
	int UFDindex = MFD[index].getUFDptr();
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		if (strcmp(filename, ufdfile->getFilename()) == 0) {
			// 检查权限
			if (this->user != "root"&&ufdfile->getMode() != MODE_W && ufdfile->getMode() != MODE_RW)
				return PERMISSION_DENIED;
			open_read();
			
			// 将该文件的原内容先读入到文件流中
			FileRead(UFDs[UFDindex][i], false);
			close();

			int firstptr = ufdfile->getBlockptr();
			vector<int> itakenBlocks;
			itakenBlocks.push_back(firstptr);
			int nextbptr = firstptr;

			// 找到该文件当前已经连续占用的block的最后一个，如果是append，则要从最后一个指针开始
			while (nextbptr != -1) {
				if (Blocks[nextbptr].getNext() != -1)
					itakenBlocks.push_back(Blocks[nextbptr].getNext());
				nextbptr = Blocks[nextbptr].getNext();
			}
			vector<string> contents;

			// 如果原文件为空，默认是覆盖写
			if (ufdfile->getSize() == 0)
				mode = MODE_OVERWRITE;

			// 写入文件前，需要对itakenBlock以及Blocks的标识做对应处理
			// 文件长度也要做同步
			if (mode == MODE_APPEND) {
				firstptr = itakenBlocks.back();
				string firstContent = Blocks[firstptr].getData();
				int contentlen = (int)content.length();
				// 由于最后一个block中，内容未被完全占据，所以最后一个block的内容要单独提出来
				// 与原content相加，便于进行block划分
				content = firstContent + content;
				this->split(contents, content, 507);
				int extra = (int)contents.size() - 1;// 需要的额外的Block
				if (findBlocks(UFDindex, extra, itakenBlocks) == FAILED)
					return NOT_ENOUGH_STORAGE;
				ufdfile->setSize(ufdfile->getSize() + contentlen);
			}
			else if (mode == MODE_OVERWRITE) {
				this->split(contents, content, 507);

				// 额外需要的block块等于新内容所需块数-原block拥有数
				int extra = (int)(contents.size()-itakenBlocks.size());
				if (extra > 0) {
					if (findBlocks(UFDindex, extra, itakenBlocks) == FAILED)
						return NOT_ENOUGH_STORAGE;
					
				}
				else if (extra < 0) {
					removeBlocks(-1 * extra, itakenBlocks);
				}
				ufdfile->setSize((int)content.length());
			}
			itakenBlocks.push_back(-1);//因为在给Block设置连续地址的时候，最后一位为-1
			int nextptr = firstptr;
			char charArray[508];

			// 将数据部分写入到block的数据部分当中
			for (int j = 0; j < itakenBlocks.size() - 1; j++) {
				Blocks[nextptr].setNext(itakenBlocks[j + 1]);
				memset(charArray, ' ', 507);
				// 类似缓冲区，每次从文件内容读507B（事先通过split分割好）
				//cout << contents[j].length() << endl;
				strcpy_s(charArray, contents[j].data());
				Blocks[nextptr].setData(charArray);
				nextptr = itakenBlocks[j + 1];
			}

			// 将block的数据更新到磁盘
			open_write();
			UFDWrite(MFD[index]);
			FileWrite(UFDs[UFDindex][i]);
			close();
			return SUCCESS;
		}
	}
	return NO_SUCH_FILE;
}

// 读取文件内容
int OSManager::cat(const char filename[20]) {
	if (this->findUser(this->directory.data()) == -1)
		return INVALID_OPERATION;
	if (strlen(filename) == 0)
		return WRONG_PARAMETER;
	int UFDindex = MFD[this->findUser(this->directory.data())].getUFDptr();
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		if (strcmp(filename, ufdfile->getFilename()) == 0) {
			open_read();
			UFDRead(MFD[this->findUser(this->directory.data())]);
			close();
			if (this->user != "root" &&ufdfile->getMode() != MODE_R && ufdfile->getMode() != MODE_RW)
				return PERMISSION_DENIED;
			open_read();
			FileRead(UFDs[UFDindex][i], false);
			close();
			cout << "File profile: " << filename << " - " << ufdfile->getSize() << "B" << " - ";
			switch (ufdfile->getMode()) {
			case 0:
				cout << "N" << endl; break;
			case 1:
				cout << "R" << endl; break;
			case 2:
				cout << "W" << endl; break;
			case 3:
				cout << "RW" << endl; break;
			default:
				break;
			}
			if (ufdfile->getSize() == 0) {
				cout << endl;
				return SUCCESS;
			}
			int blockptr = ufdfile->getBlockptr();
			while (Blockistaken[blockptr] && blockptr != -1) {
				cout << Blocks[blockptr].getData();
				blockptr = Blocks[blockptr].getNext();
			}
			cout << endl;
			return SUCCESS;
		}
	}
	return NO_SUCH_FILE;
}

int OSManager::cp(const char oldfilename[20],const char newfilename[20]) {
	int index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;
	if (strlen(oldfilename) == 0 || strlen(newfilename) == 0) return WRONG_PARAMETER;
	if (strcmp(oldfilename, newfilename) == 0) return UNKNOWN;// 复制文件不可以同名
	int UFDindex = MFD[index].getUFDptr();

	// 查看当前目录下有没有同名的文件
	for (int i = 0; i < 16; i++) {
		if (strcmp(newfilename, UFDs[UFDindex][i].getFilename()) == 0)
			return DUPLICATED;
	}

	// 只有可读的文件，才支持复制
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		//cout << ufdfile->getFilename() << endl;
		if (strcmp(oldfilename, ufdfile->getFilename()) == 0) {
			if (this->user != "root"&&ufdfile->getMode() != MODE_R && ufdfile->getMode() != MODE_RW)
				return PERMISSION_DENIED;

			// 以上判定条件结束后，首先是读取老文件内容，然后新建文件，最后将老文件中的内容写入到新文件中

			// 1.读取老文件中的内容
			open_read();
			FileRead(UFDs[index][i], false);
			close();
			string copydata;
			if (ufdfile->getSize() != 0) {
				int blockptr = ufdfile->getBlockptr();
				while (Blockistaken[blockptr] && blockptr != -1) {
					copydata += Blocks[blockptr].getData();
					blockptr = Blocks[blockptr].getNext();
				}
			}

			//cout << "copydata" << endl;
			// 2.新建文件
			int result = touch(newfilename);
			if (result != SUCCESS) return result;

			// 3.写入数据
			result = write(newfilename, copydata, 0);
			return result;
		}
		//return NO_SUCH_FILE;
	}
	return UNKNOWN;
}

// 修改文件权限
int OSManager::chmod(const char filename[20],int mode) {
	int index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;
	if (mode<MODE_N || mode>MODE_RW) return WRONG_PARAMETER;
	int UFDindex = MFD[index].getUFDptr();
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		if (strcmp(filename, ufdfile->getFilename()) == 0) {
			ufdfile->setMode(mode);
			open_write();
			UFDWrite(MFD[index]);
			close();
			return SUCCESS;
		}
	}
	return NO_SUCH_FILE;
}

// 修改文件名称
int OSManager::mv(const char oldfilename[20], const char newfilename[20]) {
	int index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;
	if (strlen(oldfilename) == 0 || strlen(newfilename) == 0) return WRONG_PARAMETER;
	if (strcmp(oldfilename, newfilename) == 0) return UNKNOWN;
	int UFDindex = MFD[index].getUFDptr();

	// 查看当前目录下有没有同名的文件
	for (int i = 0; i < 16; i++) {
		if (strcmp(newfilename, UFDs[UFDindex][i].getFilename()) == 0)
			return DUPLICATED;
	}

	// 找到旧文件并进行修改
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		if (strcmp(oldfilename, ufdfile->getFilename()) == 0) {
			ufdfile->setFilename(newfilename);
			open_write();
			UFDWrite(MFD[index]);
			close();
			return SUCCESS;
		}
	}
	return NO_SUCH_FILE;
}

// 删除文件
int OSManager::rm(const char filename[20]) {
	int index = this->findUser(this->directory.data());
	if (index == -1)
		return INVALID_OPERATION;
	if (strlen(filename) == 0)
		return WRONG_PARAMETER;
	int UFDindex = MFD[index].getUFDptr();
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		if (strcmp(filename, ufdfile->getFilename()) == 0) {
			open_write();
			FileRead(UFDs[UFDindex][i], true);
			removeFile(UFDindex, ufdfile);
			ufdfile->setFilename(" ");
			UFDWrite(MFD[index]);
			close();
			return SUCCESS;
		}
	}
	return NO_SUCH_FILE;
}

// 切换目录
int OSManager::cd(const char username[14]) {
	if (strlen(username) == 0) return WRONG_PARAMETER;
	if (strcmp(username, "/") == 0) {
		this->directory = "/";
		return SUCCESS;
	}
	if (this->user == "root") {
		for (int i = 0; i < MFD.size(); i++) {
			if (strcmp(username, MFD[i].getUsername()) == 0) {
				this->directory = username;
				int index = findUser(this->directory.data());
				open_read();//切记，且目录的时候，一定要把磁盘里的文件读进来
				UFDRead(MFD[index]);
				close();
				return SUCCESS;
			}
		}
		return NO_SUCH_DIRECTORY;
	}

	// 非root用户进行cd切换
	if (strcmp(username, this->user.data()) != 0) 
		return PERMISSION_DENIED;
	this->directory = username;
	return SUCCESS;
}



// 退出当前用户
void OSManager::logout() {
	this->user = "/";
	this->directory = this->user;
	memset(UFDs, ' ', sizeof(UFDs));
	memset(Blocks, ' ', sizeof(Blocks));
}

// 格式化硬盘，即恢复到只有root的初始状态
int OSManager::reformat() {
	if (this->directory != "root") return PERMISSION_DENIED;
	MFD.clear();
	memset(UFDs, ' ', 256 * sizeof(UFDFile));
	for (int i = 0; i < 256; i++) {
		memset(&Blocks[i], -1, 4);//Block的下一个地址全部-1
		memset((char*)(&Blocks[i]) + 4, ' ', BLOCKSIZE - 4);
	}
	memset(Blockistaken, 0, 256 * sizeof(bool));
	memset(UFDistaken, 0, 16 * sizeof(bool));
	useradd("root", "root");
	this->user = "/";
	this->directory = "/";
	open_write();
	fseek(stream, 513L, SEEK_SET);
	for (int i = 0; i < 16; i++) {
		fwrite(&UFDs[i], BLOCKSIZE, 1, stream);//更新的目录索引信息到磁盘
		fwrite("\n", 1, 1, stream);
	}
	for (int i = 0; i < 256; i++) {
		fwrite(&Blocks[i], BLOCKSIZE, 1, stream);
		fwrite("\n", 1, 1, stream);
	}
	close();
	return SUCCESS;
}

// 在root的权限下，可以将一个用户的文件送给另一个用户
int OSManager::chown(const char filename[20],const char username[14]){
	if (this->user != "root") return PERMISSION_DENIED;
	
	string originUser = this->directory;//被改归属的用户
	int index = -1;
	int indexTarget = this->findUser(username);// 查找待切入的用户是否存在
	if (indexTarget == -1) return INVALID_OPERATION;

	index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;

	if (strlen(filename) == 0) return WRONG_PARAMETER;
	int UFDindex = MFD[index].getUFDptr();

	// 第一步，读文件，记录文件的名称，权限
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];

		// 找到待改变归属的文件
		if (strcmp(filename, ufdfile->getFilename()) == 0) {
			open_read();
			FileRead(UFDs[index][i], false);
			close();

			int blockptr;
			int mode = ufdfile->getMode();
			string copydata;
			if (ufdfile->getSize() != 0) {
				blockptr = ufdfile->getBlockptr();
				while (Blockistaken[blockptr] && blockptr != -1) {
					copydata += Blocks[blockptr].getData();
					blockptr = Blocks[blockptr].getNext();
				}
			}

			// 第二步，在切入用户的目录中，新建这个文件
			this->directory = username;
			int result = touch(filename);
			if (result != SUCCESS) return result;

			// 第三步，在这个文件中，写入
			result = write(filename, copydata, 0);//覆盖写
			if (result != SUCCESS) return result;

			// 第四步，文件权限同步
			result = chmod(filename, mode);
			if (result != SUCCESS) return result;

			// 第五步，切回到原目录下，将原目录的文件删除
			this->directory = originUser;
			return rm(filename);
			
		}
	}
	return NO_SUCH_FILE;
}






#include "pch.h"
#include "OSManager.h"

OSManager::OSManager() {
	this->user = "/";
	this->directory = "/";
}

OSManager::~OSManager() {

}

/**
* �»��߷������ϵ�9����������Ҫ���ںʹ���֮��Ķ�ȡ��д��
*	1.�ڶ���д֮ǰ������ִ��open_write()����open_read()������Ŀ���Ǵ��ļ���ѡ���ȡ��ʽ
*	2.�ڶ���д֮��ִ��close()�ر��ļ�
*/

// �ú�����Ҫ���ڽ��ļ���д�뵽#1�����MFD�û���Ϣ
void OSManager::MFDRead() {
	MFDUser mfdusers[16];
	fseek(stream, 0L, SEEK_SET);

	// ��16���û���ÿ���û�32�ֽڣ�һ��512�ֽڣ��ڶ���ʱ����ͬĩβ�Ļ��з�����
	fread(mfdusers, BLOCKSIZE, 1, stream);
	for (int i = 0; i < 16; i++) {

		// ֻ�������У����ڵ��û������뵽MFD�����У�����UFDistaken��ʾ���ע��
		if (strlen(mfdusers[i].getUsername()) == 0 || mfdusers[i].getUsername()[0] == ' ')
			continue;
		MFD.push_back(mfdusers[i]);

		// ֻҪ����û����ڣ�����������UFD����������ʹ�ñ�־
		UFDistaken[mfdusers[i].getUFDptr()] = true;
	}
}

// �ú���Ŀ�ģ��Ƕ�ȡ#2-16�����е��ļ�Ŀ¼������Ϣ
void OSManager::UFDRead(MFDUser mfduser) {
	int UFDptr = mfduser.getUFDptr();
	// ÿ��UFD��32B,����ÿһ���û���������16���ļ���
	// �൱��16*32=512B����������з���513B
	// �����ϵ�513B��Ҫ������#1��
	long offset = UFDptr * 513 + 513;
	fseek(stream, offset, SEEK_SET);
	fread(&UFDs[UFDptr], BLOCKSIZE, 1, stream);
	for (int i = 0; i < 16; i++) {
		if (UFDs[UFDptr][i].getFilename()[0] != ' ') // �ǿվ����Ѿ���ʹ��
			//��ָ���û�Ŀ¼�£�ָ���ļ���ָ�����ʹ��������Ϊ��ʹ��
			Blockistaken[UFDs[UFDptr][i].getBlockptr()] = true;
	}
}


// �ú�����Ҫ���ڶ�ȡ#17-#273������ָ���ļ�Ŀ¼�����µ������ļ����ݣ�д�뵽Blocks��
void OSManager::FileRead(UFDFile ufdfile, bool onlyPtr) {
	int blockptr = ufdfile.getBlockptr();

	// ֻҪĳ���ļ�Ŀ¼�����µ��ļ���Block��ʹ�ã�����ļ����ж���
	while (Blockistaken[blockptr]) {
		//����UOF���ڴ�#17��ʼ��ÿ����������512B������β����׺1B����513λ��λ����
		long offset = blockptr * 513 + 17 * 513;

		//��ָ��ƫ�ƿ�ʼ�����ļ���,SEEK_SET��ָ���ļ���ͷ��ʼƫ��
		fseek(stream, offset, SEEK_SET);

		// ���ļ���stream�У���ȡ1��4�ֽڶ��룬������Ӧ��Blocks[blockptr]��
		// 1�ζ�4�ֽڣ���ʾֻ��ָ��(Block��ַ)
		if (onlyPtr) fread(&Blocks[blockptr], 4, 1, stream);
		// �������ָ����ͬ����һ����룬ֻ��д�ص�ʱ����Ҫ���
		else fread(&Blocks[blockptr], BLOCKSIZE, 1, stream);
		blockptr = Blocks[blockptr].getNext();//��ȡ�����ռ����һ����ַ
	}
}

// �ú������û���Ϣ����д�뵽���̵���
void OSManager::MFDWrite() {
	MFDUser *mfdusers = new MFDUser[MFD.size()];

	//����ǰ�Ѿ����û��б��е����ݣ��洢��mfdusers������
	memcpy(mfdusers, &MFD[0], MFD.size() * sizeof(MFDUser));
	int validSize = int(MFD.size() * sizeof(MFDUser));
	char * blank = new char[BLOCKSIZE - validSize];//���߳���֮�����û�������Ĳ���
	for (int i = 0; i < BLOCKSIZE - validSize; i++)
		blank[i] = ' ';

	//׼�����û���Ϣд�ش��̣���ȡ�ļ��������ļ���ʼд��
	fseek(stream, 0L, SEEK_SET);
	
	//�ֱ��û���Ϣ���հײ��֡��Լ�β����׺\nд�뵽���̵���
	fwrite(mfdusers, validSize, 1, stream);
	fwrite(blank, BLOCKSIZE - validSize, 1, stream);
	fwrite("\n", 1, 1, stream);
	delete[] blank;
	delete[] mfdusers;
}

// �ú�����Ŀ¼������Ϣд�뵽���̵���
void OSManager::UFDWrite(MFDUser mfduser) {
	int UFDptr = mfduser.getUFDptr();

	// ��ƫ������ת����ǰ�û���Ŀ¼������ʼ��ַ��
	long offset = UFDptr * 513 + 513;
	fseek(stream, offset, SEEK_SET);
	fwrite(&UFDs[UFDptr], BLOCKSIZE, 1, stream);
	fwrite("\n", 1, 1, stream);
}

// �ú������ļ��У���������block����д�����̵���
void OSManager::FileWrite(UFDFile ufdfile) {
	int blockptr = ufdfile.getBlockptr();
	while (Blockistaken[blockptr]) {
		long offset = blockptr * 513 + 17 * 513;//ÿ��block��513B�����ϴ�#17��ʼ
		fseek(stream, offset, SEEK_SET);
		fwrite(&Blocks[blockptr], BLOCKSIZE, 1, stream);
		fwrite("\n", 1, 1, stream);
		blockptr = Blocks[blockptr].getNext();
	}
}

// д�ļ�����ȡ�ļ���
void OSManager::open_write() {

	// �Զ�������ʽ�򿪣�ֻ�����д
	stream = fopen("disk.txt", "rb+");
	if (stream == NULL) {
		perror("Open file disk.txt");
		exit(1);
	}
}

// ���ļ�����ȡ�ļ���
void OSManager::open_read() {
	// �Զ����Ʒ�ʽ��ֻ���򿪴���
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



//�������г�508Bÿһ�飬װ��contents�б���
//��ΪBlock��СΪ512B�����ݲ���508B������ָ����һ����ַ��int����4B��
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
 * ɾ���ļ�������UFD������ͬ��ɾ��
 * The next index of the file block should be in memory
 * @param UFDindex �û�����UFD���������
 * @param ufdfile �ļ�ָ��
 */

void OSManager::removeFile(int UFDindex, UFDFile * ufdfile) {
	if (!UFDistaken[UFDindex]) return;//����ÿ�û�б�ռ�ã�ֱ�ӽ���
	int firstptr = ufdfile->getBlockptr();//����ļ�����ʼ��ַ
	
	// �����ļ���Ϊ��
	// ufdfile->setFilename(" ");
	
	//�ļ��ܳ�Ϊ508B�����һλ����������'\0'�ģ��������'\n'������ڼ��±�����ʾ����
	//���ļ���ʵ�ʴ�С��ȥ1����ȥβ����׺������507����1���õ�ռ�õ��ܿ���
	int amount = (ufdfile->getSize() - 1) / 507 + 1;
	vector<int> itakenBlocks;
	itakenBlocks.push_back(firstptr);
	int nextptr = firstptr;
	while (nextptr != -1) {
		// ��ǰ����,������Blocks�����������itakenBlocks
		// �����removeBlocks�У������м����itakenBlocks������pop��
		// ͬʱ����Blockistaken��״̬
		if (Blocks[nextptr].getNext() != -1)
			itakenBlocks.push_back(Blocks[nextptr].getNext());
		nextptr = Blocks[nextptr].getNext();
	}
	removeBlocks(amount, itakenBlocks);
	memset(ufdfile, 0, 32);//ɾ����֮�󣬽���ӦĿ¼��������ȫ����0
}

/**
* ʹ��������Ӧ�㷨��Ѱ��ָ����Ŀ�յĿ�
* @param UFDindex UFD��������������������
* @param amount ��Ҫ���ҵĿ����Ŀ
* @param itakenBlocks ���б�洢���ǽ�Ҫ��ʹ�õ�blocks
* @return SUCCESS ���ÿһ�鶼�ҵ��洢���򣬷��سɹ������򷵻�ʧ��
*/

int OSManager::findBlocks(int UFDindex,int amount,vector<int>& itakenBlocks) {
	if (amount == 0) return SUCCESS;
	vector<int> notTaken;

	//�ڶ�ӦUFDĿ¼��16���ļ��н��б�����ÿ���ļ��л�δ��ʹ�õĿ�
	for (int i = 16 * UFDindex; i < 16 * UFDindex + 16; i++) {
		if (!Blockistaken[i]) {//�����ҵ���Ӧ�ռ���û���õĿ����Ŀ
			notTaken.push_back(i);
			if (notTaken.size() == amount) {
				itakenBlocks.insert(itakenBlocks.end(), notTaken.begin(), notTaken.end());
				for (int j = 0; j < notTaken.size(); j++) {
					Blockistaken[notTaken[j]] = true;//��־�Ѿ���ʹ��
				}
				return SUCCESS;
			}
		}
	}
	return FAILED;
}

/**
 * �޸��Ѿ���ռ��Blocksistaken�ı��Ϊfalse
 * itakenBlocks������Ѿ�ռ�õ�UFD���
 * @param amount �������block��
 * @param itakenBlocks �����Ѿ���ʹ�õ�block��
 */

void OSManager::removeBlocks(int amount, vector<int>& itakenBlocks) {
	for (int i = 0; i < amount; i++) {
		Blockistaken[itakenBlocks.back()] = false;
		itakenBlocks.pop_back();
		if (itakenBlocks.size() > 0)
			//��ʼ����ռ��UFD��ŵ���һ����ַΪ-1
			Blocks[itakenBlocks.back()].setNext(-1);
	}
}

/**
 * Find the user from MFD
 * @param username �����ҵ�username
 * @return �ҵ��򷵻�user����������-1
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
	// ���ִ���߲��ǹ���Ա����ʾȨ�޲���
	if (strcmp(username, "root") != 0 && this->user != "root") return PERMISSION_DENIED;
	// ����û�����������Ϊ�գ�����ʾ�ֶδ���
	if (strlen(username) == 0 || strlen(password) == 0) return WRONG_PARAMETER;
	// ���볤��С��3Ҳ��Ч
	if (strlen(password) < 3) return INVALID_PASSWORD;
	// �û��ռ��������16
	if (MFD.size() == 16) return NOT_ENOUGH_STORAGE;
	if (findUser(username) != -1) return DUPLICATED;
	
	//��һ���յ��û��ռ䣬�����û����ȥ
	for (int i = 0; i < 16; i++) {
		if (!UFDistaken[i]) {
			MFDUser mfduser(username, password, i);
			UFDistaken[i] = true;
			MFD.push_back(mfduser);
			open_write();//���ļ�
			MFDWrite();//д�û���Ϣ
			close();//���ļ�
			printf("%s%s%s\n", "User \"", username, "\" has been successfully created.");
			return SUCCESS;
		}
	}
	
	return UNKNOWN;
}

int OSManager::usermod(const char username[14],const char password[14]) {
	// ȷ�������Ѿ���¼��״̬�½����޸���Ϣ
	int userIndex = this->findUser(this->directory.data());
	if (userIndex == -1) return INVALID_OPERATION;
	int usernamelen = (int)strlen(username);
	int passwordlen = (int)strlen(password);
	if (usernamelen == 0 || passwordlen == 0) return WRONG_PARAMETER;
	bool update_username = false;
	if (usernamelen != 0) {
		// root���û���Ϣ�����޸�
		if (this->directory == "root") return OTHER_ERROR;
		// �û�����ԭ��һ��Ҳ�޸�ʧ��
		if (strcmp(username, this->user.data()) == 0) return UNKNOWN;
		// �޸ĺ���û����������Ѿ����ڵ�
		if (findUser(username) != -1) return DUPLICATED;

		update_username = true;
	}
	if (passwordlen != 0) {
		if(passwordlen < 3) return INVALID_PASSWORD;
		MFD[userIndex].setPassword(password);
	} 
	if (update_username) {
		MFD[userIndex].setUsername(username);
		// ���ĺ�������Ч
		if (this->user != "root") this->user = MFD[userIndex].getUsername();
		
		// Ŀ¼�л���
		this->directory = MFD[userIndex].getUsername();
	}
	open_write();
	MFDWrite();
	close();
	return SUCCESS;
}

int OSManager::userdel(const char username[20]) {
	// root�ǲ��ܱ�ɾ����
	if (strcmp(username, "root") == 0) return OTHER_ERROR;

	// ֻ����rootȨ�ޣ����߾����û�������Ȩ�޽���ɾ������
	if (this->user == "root" || strcmp(username, this->user.data()) == 0) {
		int userIndex = this->findUser(username);
		if (userIndex == -1) return NO_SUCH_DIRECTORY;

		//��ȡ��ɾ���û����ļ���������
		int UFDindex = MFD[userIndex].getUFDptr();
		open_write();

		//�����û��������ļ�ɾ����������block
		for (int i = 0; i < 16; i++) {
			UFDFile *ufdfile = &UFDs[UFDindex][i];
			if (ufdfile->getFilename()[0] != ' ') {// �������д��ڵ��ļ�
				FileRead(UFDs[UFDindex][i], true);
				removeFile(UFDindex, ufdfile);
			}
		}
		UFDistaken[UFDindex] = false;
		UFDWrite(MFD[userIndex]);

		//���ڴ��У���ʼ����ɾ���û����б���Ϣ
		memset(&MFD[userIndex], 0, sizeof(MFDUser));
		MFDWrite();
		close();
		this->directory = "/";
		if (this->user != "root")
			this->user = "/";

		//���µ�ǰMFD��vector
		/*vector<MFDUser> MFDTemp;
		for (int i = 0; i < MFD.size();i++) 
			if(i!=userIndex) MFDTemp.push_back(MFD[i]);
		MFD = MFDTemp;*/

		return SUCCESS;
	}
	else return PERMISSION_DENIED;
}

// �л��û���
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

// �½�һ���ļ�
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
			// �������û��������ļ�����
			
			for (int j = 16 * UFDindex; j < 16 * UFDindex + 16; j++) {
				if (!Blockistaken[j]) { // �����Block�ǿյ�

					// ��ʼ����Block
					memset(&Blocks[j], -1, 4); 
					memset((char*)(&Blocks[j]) + 4, ' ', BLOCKSIZE - 4);

					UFDs[UFDindex][i].setFilename(filename);
					UFDs[UFDindex][i].setMode(MODE_RW);// �����ļ��ɶ���д
					UFDs[UFDindex][i].setSize(0); // �½�һ���ļ���û��д�����ݣ�����Ϊ0
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

// �г���ǰĿ¼�µ������ļ�
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

// ��ָ���ļ���д������
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
			// ���Ȩ��
			if (this->user != "root"&&ufdfile->getMode() != MODE_W && ufdfile->getMode() != MODE_RW)
				return PERMISSION_DENIED;
			open_read();
			
			// �����ļ���ԭ�����ȶ��뵽�ļ�����
			FileRead(UFDs[UFDindex][i], false);
			close();

			int firstptr = ufdfile->getBlockptr();
			vector<int> itakenBlocks;
			itakenBlocks.push_back(firstptr);
			int nextbptr = firstptr;

			// �ҵ����ļ���ǰ�Ѿ�����ռ�õ�block�����һ���������append����Ҫ�����һ��ָ�뿪ʼ
			while (nextbptr != -1) {
				if (Blocks[nextbptr].getNext() != -1)
					itakenBlocks.push_back(Blocks[nextbptr].getNext());
				nextbptr = Blocks[nextbptr].getNext();
			}
			vector<string> contents;

			// ���ԭ�ļ�Ϊ�գ�Ĭ���Ǹ���д
			if (ufdfile->getSize() == 0)
				mode = MODE_OVERWRITE;

			// д���ļ�ǰ����Ҫ��itakenBlock�Լ�Blocks�ı�ʶ����Ӧ����
			// �ļ�����ҲҪ��ͬ��
			if (mode == MODE_APPEND) {
				firstptr = itakenBlocks.back();
				string firstContent = Blocks[firstptr].getData();
				int contentlen = (int)content.length();
				// �������һ��block�У�����δ����ȫռ�ݣ��������һ��block������Ҫ���������
				// ��ԭcontent��ӣ����ڽ���block����
				content = firstContent + content;
				this->split(contents, content, 507);
				int extra = (int)contents.size() - 1;// ��Ҫ�Ķ����Block
				if (findBlocks(UFDindex, extra, itakenBlocks) == FAILED)
					return NOT_ENOUGH_STORAGE;
				ufdfile->setSize(ufdfile->getSize() + contentlen);
			}
			else if (mode == MODE_OVERWRITE) {
				this->split(contents, content, 507);

				// ������Ҫ��block������������������-ԭblockӵ����
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
			itakenBlocks.push_back(-1);//��Ϊ�ڸ�Block����������ַ��ʱ�����һλΪ-1
			int nextptr = firstptr;
			char charArray[508];

			// �����ݲ���д�뵽block�����ݲ��ֵ���
			for (int j = 0; j < itakenBlocks.size() - 1; j++) {
				Blocks[nextptr].setNext(itakenBlocks[j + 1]);
				memset(charArray, ' ', 507);
				// ���ƻ�������ÿ�δ��ļ����ݶ�507B������ͨ��split�ָ�ã�
				//cout << contents[j].length() << endl;
				strcpy_s(charArray, contents[j].data());
				Blocks[nextptr].setData(charArray);
				nextptr = itakenBlocks[j + 1];
			}

			// ��block�����ݸ��µ�����
			open_write();
			UFDWrite(MFD[index]);
			FileWrite(UFDs[UFDindex][i]);
			close();
			return SUCCESS;
		}
	}
	return NO_SUCH_FILE;
}

// ��ȡ�ļ�����
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
	if (strcmp(oldfilename, newfilename) == 0) return UNKNOWN;// �����ļ�������ͬ��
	int UFDindex = MFD[index].getUFDptr();

	// �鿴��ǰĿ¼����û��ͬ�����ļ�
	for (int i = 0; i < 16; i++) {
		if (strcmp(newfilename, UFDs[UFDindex][i].getFilename()) == 0)
			return DUPLICATED;
	}

	// ֻ�пɶ����ļ�����֧�ָ���
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];
		//cout << ufdfile->getFilename() << endl;
		if (strcmp(oldfilename, ufdfile->getFilename()) == 0) {
			if (this->user != "root"&&ufdfile->getMode() != MODE_R && ufdfile->getMode() != MODE_RW)
				return PERMISSION_DENIED;

			// �����ж����������������Ƕ�ȡ���ļ����ݣ�Ȼ���½��ļ���������ļ��е�����д�뵽���ļ���

			// 1.��ȡ���ļ��е�����
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
			// 2.�½��ļ�
			int result = touch(newfilename);
			if (result != SUCCESS) return result;

			// 3.д������
			result = write(newfilename, copydata, 0);
			return result;
		}
		//return NO_SUCH_FILE;
	}
	return UNKNOWN;
}

// �޸��ļ�Ȩ��
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

// �޸��ļ�����
int OSManager::mv(const char oldfilename[20], const char newfilename[20]) {
	int index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;
	if (strlen(oldfilename) == 0 || strlen(newfilename) == 0) return WRONG_PARAMETER;
	if (strcmp(oldfilename, newfilename) == 0) return UNKNOWN;
	int UFDindex = MFD[index].getUFDptr();

	// �鿴��ǰĿ¼����û��ͬ�����ļ�
	for (int i = 0; i < 16; i++) {
		if (strcmp(newfilename, UFDs[UFDindex][i].getFilename()) == 0)
			return DUPLICATED;
	}

	// �ҵ����ļ��������޸�
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

// ɾ���ļ�
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

// �л�Ŀ¼
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
				open_read();//�мǣ���Ŀ¼��ʱ��һ��Ҫ�Ѵ�������ļ�������
				UFDRead(MFD[index]);
				close();
				return SUCCESS;
			}
		}
		return NO_SUCH_DIRECTORY;
	}

	// ��root�û�����cd�л�
	if (strcmp(username, this->user.data()) != 0) 
		return PERMISSION_DENIED;
	this->directory = username;
	return SUCCESS;
}



// �˳���ǰ�û�
void OSManager::logout() {
	this->user = "/";
	this->directory = this->user;
	memset(UFDs, ' ', sizeof(UFDs));
	memset(Blocks, ' ', sizeof(Blocks));
}

// ��ʽ��Ӳ�̣����ָ���ֻ��root�ĳ�ʼ״̬
int OSManager::reformat() {
	if (this->directory != "root") return PERMISSION_DENIED;
	MFD.clear();
	memset(UFDs, ' ', 256 * sizeof(UFDFile));
	for (int i = 0; i < 256; i++) {
		memset(&Blocks[i], -1, 4);//Block����һ����ַȫ��-1
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
		fwrite(&UFDs[i], BLOCKSIZE, 1, stream);//���µ�Ŀ¼������Ϣ������
		fwrite("\n", 1, 1, stream);
	}
	for (int i = 0; i < 256; i++) {
		fwrite(&Blocks[i], BLOCKSIZE, 1, stream);
		fwrite("\n", 1, 1, stream);
	}
	close();
	return SUCCESS;
}

// ��root��Ȩ���£����Խ�һ���û����ļ��͸���һ���û�
int OSManager::chown(const char filename[20],const char username[14]){
	if (this->user != "root") return PERMISSION_DENIED;
	
	string originUser = this->directory;//���Ĺ������û�
	int index = -1;
	int indexTarget = this->findUser(username);// ���Ҵ�������û��Ƿ����
	if (indexTarget == -1) return INVALID_OPERATION;

	index = this->findUser(this->directory.data());
	if (index == -1) return INVALID_OPERATION;

	if (strlen(filename) == 0) return WRONG_PARAMETER;
	int UFDindex = MFD[index].getUFDptr();

	// ��һ�������ļ�����¼�ļ������ƣ�Ȩ��
	for (int i = 0; i < 16; i++) {
		UFDFile *ufdfile = &UFDs[UFDindex][i];

		// �ҵ����ı�������ļ�
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

			// �ڶ������������û���Ŀ¼�У��½�����ļ�
			this->directory = username;
			int result = touch(filename);
			if (result != SUCCESS) return result;

			// ��������������ļ��У�д��
			result = write(filename, copydata, 0);//����д
			if (result != SUCCESS) return result;

			// ���Ĳ����ļ�Ȩ��ͬ��
			result = chmod(filename, mode);
			if (result != SUCCESS) return result;

			// ���岽���лص�ԭĿ¼�£���ԭĿ¼���ļ�ɾ��
			this->directory = originUser;
			return rm(filename);
			
		}
	}
	return NO_SUCH_FILE;
}






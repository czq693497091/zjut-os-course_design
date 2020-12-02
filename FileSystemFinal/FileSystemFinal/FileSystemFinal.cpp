// FileSystemFinal.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <sstream>
#include <map>
#include "OSManager.h"
using namespace std;

void commandInit();
string getHelper(string define,string usage);



OSManager osm;
vector<MFDUser> MFD;
UFDFile UFDs[16][16];
Block Blocks[256];
bool UFDistaken[16];
bool Blockistaken[256];

map<string, int> command;
map<string, string> commandHelper;

int main() {
	//cout << "123" << endl;
	commandInit();
	osm.open_read();
	osm.MFDRead(); // 读入用户信息
	osm.close();
	map<string, int>::iterator iter;
	cout << "\nWelcome to the two-level directory structure!" << endl;
	cout << "You can input commands as follow:\n[";
	for (iter = command.begin(); iter != command.end(); iter++)
		cout << iter->first << " ";
	cout << "]\n";
	cout << "\nEnter help to get more information" << endl;
	cout << "Or append the command with ? to find details" << endl;

	while (true) {
		string cmd;
		cout << osm.directory << " ";
		if (osm.user == "root") cout << "# ";
		else cout << "$ ";
		getline(cin, cmd);
		if (cmd.empty()) continue;
		bool findQuotes = false;
		string pre,inner, post;

		// 在写入文件的时候，写入内容要用 “” 圈起来
		int quotes1 = (int)cmd.find('\"');
		int quotes2 = (int)cmd.rfind('\"');

		if (quotes1 != -1 && quotes1 != quotes2) {
			pre = cmd.substr(0, quotes1);
			inner = cmd.substr(quotes1 + 1, quotes2 - quotes1 - 1);
			post = cmd.substr(quotes2 + 1, cmd.length() - quotes2 - 1);
			cmd = pre;
			findQuotes = true;
		}
		
		vector<string> param;
		stringstream input(cmd);
		string result;
		while (input >> result)
			param.push_back(result);

		if (findQuotes) {
			param.push_back(inner);
			stringstream input(post);
			while (input >> result)
				param.push_back(result);
		}

		if (param[0].back() == '?') {
			string tempCmd = param[0].substr(0, param[0].size() - 1);
			if (commandHelper.count(tempCmd) == 0) 
				cerr << "No such command!" << endl;
			else 
				cout << commandHelper[tempCmd] << endl;
			continue;
		}

		int cmd_code = -1;//  对应的命令
		if (command.count(param[0]) != 0)
			cmd_code = command[param[0]];
		int code = -1;// Result.h中的返回值

		/*
		command["exit"] = 0;
		command["reformat"] = 1;
		command["su"] = 2;
		command["touch"] = 3;
		command["cat"] = 4;
		command["ls"] = 5;
		command["write"] = 6;
		command["useradd"] = 7;
		command["chmod"] = 8;
		command["mv"] = 9;
		command["usermod"] = 10;
		command["rm"] = 11;
		command["cd"] = 12;
		command["logout"] = 13;
		command["userdel"] = 14;
		command["cp"] = 15;
		command["chown"] = 16;
		command["help"] = 17;
		*/
		
		switch (cmd_code)
		{
		case -1: {// 命令错误
			cerr << "No such command!" << endl;
			break;
		}		
		case 0: {// 退出文件系统
			exit(0);
		}
		case 1: {// 格式化硬盘
			code = osm.reformat();
			break;
		}
		case 2: {// 登录
			if (param.size() != 2) 
				code = WRONG_PARAMETER;
			else {
				int index = osm.findUser(param[1].data());
				if (index != -1) {
					string password;
					cout << "password: ";
					cin >> password;
					code = osm.su(index, password.data());
					cin.get();
				}
				else cerr << "No such user!" << endl;
			}
			break;
		}
		case 3: {// 新建文件
			if (param.size() != 2)
				code = WRONG_PARAMETER;
			else
				code = osm.touch(param[1].data());
			break;
		}
		case 4: {// 读取文件
			if (param.size() < 2)
				code = WRONG_PARAMETER;
			else
				code = osm.cat(param[1].data());
			break;
		}
		case 5: {// 列出目录下的所有文件
			vector<string> files = osm.ls();
			/*for (int i = 0; i < (int)files.size() - 1; i++) {
				
				if (i % 3 == 2)
					cout << files[i] << endl;
				else
					cout << files[i] << "\t";
			}
			if (files.size() > 0)
				cout << files[files.size() - 1];
			cout << endl;*/
			for (int i = 0; i < files.size(); i++) {
				cout << files[i] << endl;
			}
			break;
		}
		case 6: {// 写文件
			if (param.size() != 4)
				code = WRONG_PARAMETER;
			else
				code = osm.write(param[1].data(), param[2], param[3][0] - 48);//ascii码字符转数字
			break;
		}
		case 7: {// 添加用户
			if (param.size() != 3)
				code = WRONG_PARAMETER;
			else
				code = osm.useradd(param[1].data(), param[2].data());
			break;
		}
		case 8: {// 修改文件权限
			if (param.size() != 3)
				code = WRONG_PARAMETER;
			else
				code = osm.chmod(param[1].data(), param[2][0] - 48);
			break;
		}
		case 9: {// 文件改名
			if (param.size() != 3)
				code = WRONG_PARAMETER;
			else
				code = osm.mv(param[1].data(), param[2].data());
			break;
		}
		case 10: {// 修改账户或者密码，必须是已经登录的前提下
			if (param.size() < 3)
				code = WRONG_PARAMETER;
			else {
				string newusername = "", newpassword = "";
				for (int j = 0; j < param.size() - 1; j++) {
					if (param[j] == "-p") newpassword = param[j + 1];
					if (param[j] == "-u") newusername = param[j + 1];
				}
				code = osm.usermod(newusername.data(), newpassword.data());
			}
			break;
		}
		case 11: {// 删除文件
			if (param.size() != 2)
				code = WRONG_PARAMETER;
			else
				code = osm.rm(param[1].data());
			break;
		}
		case 12: {// 切换目录
			if (param.size() != 2)
				code = WRONG_PARAMETER;
			code = osm.cd(param[1].data());
			break;
		}
		case 13: {// 退出
			osm.logout();
			break;
		}
		case 14: {// 删除用户
			if (param.size() != 2)
				code = WRONG_PARAMETER;
			else
				code = osm.userdel(param[1].data());
			if (code == OTHER_ERROR)
				cerr << "Sorry, root user can't be deleted!" << endl;
			break;
		}
		case 15: {// 复制文件
			if (param.size() != 3)
				code = WRONG_PARAMETER;
			else
				code = osm.cp(param[1].data(), param[2].data());
			break;
		}

		case 16: {
			if (param.size() != 3)
				code = WRONG_PARAMETER;
			else
				code = osm.chown(param[1].data(), param[2].data());
			break;
		}

		case 17: {
			map<string, string>::iterator it;
			for (it = commandHelper.begin(); it != commandHelper.end(); it++) {
				cout << it->first << endl;
				cout << it->second << endl << endl;
			}
			break;
		}	
		default:
			break;
		}

		// 异常情况集中处理
		switch (code){

		case DUPLICATED:cerr << "Duplicated file name!" << endl; break;
		case NO_SUCH_FILE:cerr << "No such file!" << endl; break;
		case NOT_ENOUGH_STORAGE:cerr << "Not enough storage!" << endl; break;
		case PERMISSION_DENIED:cerr << "Permission denied!" << endl; break;
		case INCORRECT_PASSWORD:cerr << "Incorrect password!" << endl; break;
		case INVALID_PASSWORD:cerr << "Password length should not be less than 3!" << endl; break;
		case NO_SUCH_DIRECTORY:cerr << "No such directory or user!" << endl; break;
		case INVALID_OPERATION:cerr << "Invalid operation, are you in a user directory?" << endl; break;
		case WRONG_PARAMETER:cerr << commandHelper[param[0]] << endl; break;
		default:break;

		}
	}
	return 0;
}

string getHelper(string define, string usage) {
	string result = "define: ";
	result += define + ".\n";
	result += "usage : " + usage;
	return result;
}

void commandInit() {

	// 初始化各个命令代表的数字
	command["exit"] = 0;
	command["reformat"] = 1;
	command["su"] = 2;
	command["touch"] = 3;
	command["cat"] = 4;
	command["ls"] = 5;
	command["write"] = 6;
	command["useradd"] = 7;
	command["chmod"] = 8;
	command["mv"] = 9;
	command["usermod"] = 10;
	command["rm"] = 11;
	command["cd"] = 12;
	command["logout"] = 13;
	command["userdel"] = 14;
	command["cp"] = 15;
	command["chown"] = 16;
	command["help"] = 17;

	//用getHelper的方式便捷生成帮助
	string exitHelper = getHelper("exit the program", "exit");
	string reformatHelper = getHelper("reformat the disk and system", "reformat");
	string suHelper = getHelper("switch user", "su username");
	string touchHelper = getHelper("create a blank file", "touch filename");
	string catHelper = getHelper("print the file", "cat filename");
	string lsHelper = getHelper("list directory contents", "ls");
	string writeHelper = getHelper("write files", "write filename \"content\" mode\nmode: 0 overwrite\n      1 append");
	string useraddHelper = getHelper("add an user in this system", "useradd username password");
	string chmodHelper = getHelper("change file mode", "chmod filename mode\nmode: 0 unreadable and unwritable\n      1 readable\n      2 writable\n      3 readable and writable");
	string mvHelper = getHelper("rename a file", "mv oldname newname");
	string usermodHelper = getHelper("modify the information of the user", "usermod -p newpassword\n                -u newusername");
	string rmHelper = getHelper("remove a file", "rm filename");
	string cdHelper = getHelper("change a directory", "cd directory");
	string logoutHelper = getHelper("logout", "logout");
	string userdelHelper = getHelper("delete a user", "userdel username");
	string cpHelper = getHelper("copy a file", "cp oldname newname");
	string chownHelper = getHelper("change the owner of file(only by root)", "chown filename username");
	string helpHelper = getHelper("print help document", "help");

	//将这些helper指令插入到commandHelper的Map中
	commandHelper["exit"] = exitHelper;
	commandHelper["reformat"] = reformatHelper;
	commandHelper["su"] = suHelper;
	commandHelper["touch"] = touchHelper;
	commandHelper["cat"] = catHelper;
	commandHelper["ls"] = lsHelper;
	commandHelper["write"] = writeHelper;
	commandHelper["useradd"] = useraddHelper;
	commandHelper["chmod"] = chmodHelper;
	commandHelper["mv"] = mvHelper;
	commandHelper["usermod"] = usermodHelper;
	commandHelper["rm"] = rmHelper;
	commandHelper["cd"] = cdHelper;
	commandHelper["logout"] = logoutHelper;
	commandHelper["userdel"] = userdelHelper;
	commandHelper["cp"] = cpHelper;
	commandHelper["chown"] = chownHelper;
	commandHelper["help"] = helpHelper;
}


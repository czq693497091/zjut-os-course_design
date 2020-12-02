#include "pch.h"
#include "MFDUser.h"

MFDUser::MFDUser() {

}

MFDUser::MFDUser(const char username[14], const char password[14], int UFDptr) {
	for (int i = 0; i < 14; i++) {
		this->username[i] = ' ';
		this->password[i] = ' ';
	}
	strcpy_s(this->username, username);
	strcpy_s(this->password, password);
	this->UFDptr = UFDptr;
}

MFDUser::~MFDUser() {

}

char * MFDUser::getUsername() {
	return username;
}

char * MFDUser::getPassword() {
	return password;
}

int MFDUser::getUFDptr() {
	return UFDptr;
}

void MFDUser::setUsername(const char username[14]) {
	strcpy_s(this->username, username);
}

void MFDUser::setPassword(const char password[14]) {
	strcpy_s(this->password, password);
}


#include "pch.h"
#include "UFDFile.h"

UFDFile::UFDFile() {
	for (int i = 0; i < 20; i++) {
		this->filename[i] = ' ';
	}
	this->mode = 0;
	this->size = 0;
	this->blockptr = -1;//一开始不指定拥有者
}

UFDFile::UFDFile(const char filename[20],int mode,int size,int blockptr) {
	for (int i = 0; i < 20; i++) {
		this->filename[i] = ' ';
	}
	strcpy_s(this->filename, filename);
	this->mode = mode;
	this->size = size;
	this->blockptr = blockptr;
}

UFDFile::~UFDFile() {

}

char * UFDFile::getFilename() {
	return filename;
}

void UFDFile::setFilename(const char filename[20]) {
	strcpy_s(this->filename, filename);
}

int UFDFile::getMode() {
	return mode;
}

void UFDFile::setMode(int mode) {
	this->mode = mode;
}

void UFDFile::setBlockptr(int blockptr) {
	this->blockptr = blockptr;
}

int UFDFile::getBlockptr() {
	return blockptr;
}

int UFDFile::getSize() {
	return size;
}

void UFDFile::setSize(int size) {
	this->size = size;
}
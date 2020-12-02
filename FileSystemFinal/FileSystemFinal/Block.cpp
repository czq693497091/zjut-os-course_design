#include"pch.h"
#include"Block.h"

Block::Block() {
	this->next = -1;
}

Block::~Block() {

}

int Block::getNext() {
	return next;
}

void Block::setNext(int next) {
	this->next = next;
}

char * Block::getData() {
	return data;
}

void Block::setData(const char data[508]) {
	memset(this->data, ' ', 507);
	strcpy_s(this->data, data);
}
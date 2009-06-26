/** \file
 * Implementation for the AutoBuffer
 */

#include "AutoBuffer.h"
#include <new>
#include <cstdlib>
#include <cstring>

typedef unsigned long ulong;
typedef char* caddr_t;

AutoBuffer::AutoBuffer(ulong initialsize)
	: buffer(0), size(0) {
	ChangeSize(initialsize);
}

AutoBuffer::AutoBuffer(const AutoBuffer& other) : buffer(0), size(0) {
	ulong othersize = other.GetSize();
	ChangeSize(othersize);
	other.Get(buffer, othersize);
}

AutoBuffer::AutoBuffer(void* other, ulong othersize) : buffer(0), size(0) {
	ChangeSize(othersize);
	Put(other, othersize);
}

AutoBuffer::~AutoBuffer() {
	ChangeSize(0);
}


AutoBuffer& AutoBuffer::operator=(const AutoBuffer& other) {
	if (size < other.GetSize()) {
		ChangeSize(other.GetSize());
	}
	other.Get(buffer, other.GetSize());
}

void* AutoBuffer::GetBuffer(ulong offset) {
	return (caddr_t)buffer + offset;
}

void AutoBuffer::ChangeSize(ulong newsize) {
	void *newbuffer = realloc(buffer, newsize);
	if (0 == newbuffer && newsize != 0) {
		throw std::bad_alloc();
	}
	buffer = newbuffer;
	size = newsize;
}


void AutoBuffer::Put(const void* other, const ulong othersize, const ulong offset) {
	if (offset + othersize > size) {
		ChangeSize(offset + othersize);
	}
	caddr_t spot = (caddr_t)buffer + offset;
	memcpy(buffer, other, othersize);
}

ulong AutoBuffer::Get(void* other, const ulong othersize, const ulong offset) const {
	caddr_t spot = (caddr_t)buffer + offset;
	ulong copysize = othersize;
	if (offset + othersize > size) {
		copysize = size - offset;
	}
	memcpy(other, spot, copysize);
	return copysize;
}


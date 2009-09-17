//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \brief Implementation for the static buffers
 * \author John Bridgman
 */

#include "StaticBuffer.h"
#include "AutoBuffer.h"
#include "Assert.h"
#include <cstring>

typedef unsigned long ulong;

StaticBuffer::StaticBuffer(const StaticBuffer& other)
    : buffer(other.GetBuffer()), size(other.GetSize()) {}

StaticBuffer::StaticBuffer(void* other, ulong othersize)
    : buffer(other), size(othersize) {}

StaticBuffer::StaticBuffer(AutoBuffer& other)
    : buffer(other.GetBuffer()), size(other.GetSize()) {}

StaticBuffer::~StaticBuffer() {
    buffer = 0;
    size = 0;
}

StaticBuffer& StaticBuffer::operator=(const StaticBuffer& other) {
    buffer = other.GetBuffer();
    size = other.GetSize();
    return *this;
}

void* StaticBuffer::GetBuffer(ulong offset) const {
    ASSERT(offset <= size);
    return ((char*)buffer) + offset;
}

ulong StaticBuffer::Put(const void* other, const ulong othersize, const ulong offset) const {
    ulong numtocopy = othersize;
    if (offset >= size) { return 0; }
    if (offset + othersize > size) {
        numtocopy = size - offset;
    }
    char* spot = (char*)buffer + offset;
    memcpy(spot, other, numtocopy);
    return numtocopy;
}

ulong StaticBuffer::Get(void* other, const ulong othersize, const ulong offset) const {
    ulong copysize = othersize;
    if (offset >= size) {
        return 0;
    } else if (offset + othersize >= size) {
        copysize = size - offset;
    }
    const char* spot = (const char*)buffer + offset;
    memcpy(other, spot, copysize);
    return copysize;
}

StaticConstBuffer::StaticConstBuffer(const StaticConstBuffer& other) 
    : buffer(other.GetBuffer()), size(other.GetSize()) {}

StaticConstBuffer::StaticConstBuffer(const StaticBuffer& other)
    : buffer(other.GetBuffer()), size(other.GetSize()) {}

StaticConstBuffer::StaticConstBuffer(const AutoBuffer& other)
    : buffer(other.GetBuffer()), size(other.GetSize()) {}

StaticConstBuffer::StaticConstBuffer(const void* other, ulong othersize)
    : buffer(other), size(othersize) {}

StaticConstBuffer::~StaticConstBuffer() {
    buffer = 0;
    size = 0;
}

StaticConstBuffer& StaticConstBuffer::operator=(const StaticConstBuffer& other) {
    buffer = other.GetBuffer();
    size = other.GetSize();
    return *this;
}

const void* StaticConstBuffer::GetBuffer(ulong offset) const {
    ASSERT(offset <= size);
    return ((char*)buffer) + offset;
}

ulong StaticConstBuffer::Get(void* other, const ulong othersize, const ulong offset) const {
    ulong copysize = othersize;
    if (offset >= size) {
        return 0;
    } else if (offset + othersize >= size) {
        copysize = size - offset;
    }
    const char* spot = (const char*)buffer + offset;
    memcpy(other, spot, copysize);
    return copysize;
}


bool operator==(const StaticConstBuffer& rhs, const StaticConstBuffer& lhs) {
    bool retval = true;
    ulong size = rhs.GetSize();
    if (size == lhs.GetSize()) {
        const char* rhsptr = (const char*)rhs.GetBuffer();
        const char* lhsptr = (const char*)lhs.GetBuffer();
        for (ulong i = 0; i < size; ++i) {
            if (*rhsptr != *lhsptr) {
                retval = false;
                break;
            }
            rhsptr++;
            lhsptr++;
        }
    } else { retval = false; }
    return retval;
}

bool operator!=(const StaticConstBuffer& rhs, const StaticConstBuffer& lhs) {
    return !(rhs == lhs);
}


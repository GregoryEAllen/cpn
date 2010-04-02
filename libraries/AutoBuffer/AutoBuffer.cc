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
 * \brief Implementation for the AutoBuffer
 * \author John Bridgman
 */

#include "AutoBuffer.h"
#include <new>
#include <cstdlib>
#include <cstring>

typedef unsigned long ulong;
const ulong MIN_SPACE = sizeof(long);
const ulong SHRINK_FACTOR = 4;

AutoBuffer::AutoBuffer()
    : buffer(0), size(0), memsize(0), pos(0) {
}

AutoBuffer::AutoBuffer(ulong initialsize)
    : buffer(0), size(0), memsize(0), pos(0) {
    ChangeSize(initialsize);
}

AutoBuffer::AutoBuffer(const AutoBuffer& other) : buffer(0), size(0), memsize(0), pos(0) {
    Put(other.GetBuffer(), other.GetSize());
}

AutoBuffer::AutoBuffer(const void* other, ulong othersize)
    : buffer(0), size(0), memsize(0), pos(0) {
    Put(other, othersize);
}

AutoBuffer::AutoBuffer(const StaticConstBuffer& other)
    : buffer(0), size(0), memsize(0), pos(0) {
    Put(other);
}

AutoBuffer::~AutoBuffer() {
    ChangeMemSize(0);
    size = 0;
    pos = 0;
}


AutoBuffer& AutoBuffer::operator=(const AutoBuffer& other) {
    Put(other.GetBuffer(), other.GetSize());
    return *this;
}

void* AutoBuffer::GetBuffer(ulong offset) {
    return (char*)buffer + offset + pos;
}

const void* AutoBuffer::GetBuffer(ulong offset) const {
	return (char*)buffer + offset + pos;
}

void AutoBuffer::SetPos(ulong newpos) {
    if (newpos > size) {
        ChangeSize(newpos - size);
    }
    pos = newpos;
}

ulong AutoBuffer::AddPos(ulong amount) {
    SetPos(amount + pos);
    return pos;
}

void AutoBuffer::ChangeSize(ulong newsize) {
    newsize += pos;
    if (newsize >= memsize) {
        ulong newmemsize = (memsize == 0) ? MIN_SPACE : (memsize << 1);
        while (newsize >= newmemsize) { newmemsize <<= 1; }
        ChangeMemSize(newmemsize);
    } else if (newsize == 0) {
        ChangeMemSize(0);
    } else if ( (SHRINK_FACTOR*newsize) < memsize && memsize > MIN_SPACE) {
        ulong newmemsize = memsize >> 1;
        while ( (SHRINK_FACTOR*newsize) < newmemsize && newmemsize > MIN_SPACE ) {
            newmemsize >>= 1;
        }
        ChangeMemSize(newmemsize);
    }
    size = newsize;
}

void AutoBuffer::ChangeMemSize(ulong newsize) {
    if (0 == buffer && 0 == newsize) return;
    if (0 == newsize) {
        free(buffer);
        buffer = 0;
        memsize = 0;
    } else {
        void *newbuffer = realloc(buffer, newsize);
        if (0 == newbuffer && newsize != 0) {
            throw std::bad_alloc();
        }
        buffer = newbuffer;
        memsize = newsize;
    }
}

void AutoBuffer::EnsureSize(ulong newsize) {
    if (size - pos < newsize) {
        ChangeSize(newsize);
    }
}

void AutoBuffer::Put(const void* other, ulong othersize, ulong offset) {
    offset += pos;
    if (offset + othersize > size) {
        ChangeSize(offset + othersize);
    }
    char* spot = (char*)buffer + offset;
    memcpy(spot, other, othersize);
}

ulong AutoBuffer::Get(void* other, ulong othersize, ulong offset) const {
    offset += pos;
    ulong copysize = othersize;
    if (offset >= size) {
        return 0;
    } else if (offset + othersize >= size) {
        copysize = size - offset;
    }
    char* spot = (char*)buffer + offset;
    memcpy(other, spot, copysize);
    return copysize;
}

StaticBuffer AutoBuffer::GetStaticBuffer(ulong buffsize, ulong offset) {
    offset += pos;
    if (offset + buffsize >= size) {
        ChangeSize(offset + buffsize);
    }
    char* spot = (char*)buffer + offset;
    return StaticBuffer(spot, buffsize);
}

StaticConstBuffer AutoBuffer::GetStaticConstBuffer(ulong buffsize, ulong offset) const {
    offset += pos;
    if (offset >= size) {
        offset = size;
        buffsize = 0;
    } else if (offset + buffsize >= size) {
        buffsize = size - offset;
    }
    const char* spot = (const char*)buffer + offset;
    return StaticConstBuffer(spot, buffsize);
}


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
 * \brief Implementation for AutoCircleBuffer
 * \author John Bridgman
 */

#include "AutoCircleBuffer.h"
#include "Assert.h"
#include <cstring>

const unsigned MIN_SIZE = 10;

AutoCircleBuffer::AutoCircleBuffer() : buff(), size(0), put(0), get(0) {
    ChangeMaxSize(MIN_SIZE);
}

AutoCircleBuffer::AutoCircleBuffer(int initialsize) : buff(),
    size(0), put(0), get(0)
{
    ChangeMaxSize(initialsize);
}

char* AutoCircleBuffer::AllocatePut(unsigned desired, unsigned &actual) {
    actual = desired;
    if (actual > MaxSize() - size) {
        actual = MaxSize() - size;
    }
    if (actual + put > MaxSize()) {
        actual = MaxSize() - put;
    }
    return (char*) buff.GetBuffer(put);
}
void AutoCircleBuffer::ReleasePut(unsigned amount) {
    size += amount;
    put = (put + amount)%MaxSize();
}

void AutoCircleBuffer::Put(const char* ptr, unsigned amount) {
    if (MaxSize() - size < amount) {
        EnsureMaxSize(size + amount);
    }
    unsigned pos = 0;
    while (pos < amount) {
        unsigned actual = 0;
        char* putptr = AllocatePut(amount - pos, actual);
        ASSERT(actual > 0);
        memcpy(putptr, &ptr[pos], actual);
        pos += actual;
        ReleasePut(actual);
    }
}

char* AutoCircleBuffer::AllocateGet(unsigned desired, unsigned &actual) {
    actual = desired;
    if (actual > size) {
        actual = size;
    }
    if (actual + get > MaxSize()) {
        actual = MaxSize() - get;
    }
    return (char*) buff.GetBuffer(get);
}
void AutoCircleBuffer::ReleaseGet(unsigned amount) {
    size -= amount;
    get = (get + amount)%MaxSize();
}

bool AutoCircleBuffer::Get(char* ptr, unsigned amount) {
    if (size < amount) { return false; }
    unsigned pos = 0;
    while (pos < amount) {
        unsigned actual = 0;
        char* getptr = AllocateGet(amount - pos, actual);
        memcpy(&ptr[pos], getptr, actual);
        ReleaseGet(actual);
        pos += actual;
    }
    return true;
}

void AutoCircleBuffer::ChangeMaxSize(unsigned newsize) {
    if (newsize < MIN_SIZE) {
        newsize = MIN_SIZE;
    }
    if (newsize < size) {
        newsize = size;
    }
    unsigned oldmax = MaxSize();
    // Round up to the next power of 2, ensure not zero
    unsigned newmax = oldmax != 0 ? oldmax : MIN_SIZE;
    // Shift down until equal or under
    while (newmax > newsize) { newmax >>= 1; }
    // Shift up until just over
    while (newmax < newsize) { newmax <<= 1; }
    if (newmax < oldmax) {
        // Only do a double copy if we need to
        // Could be more efficient but reduction in size is rare
        AutoBuffer storage(size);
        unsigned amount = 0;
        unsigned total = 0;
        while (size > 0) {
            char* ptr = AllocateGet(size, amount);
            storage.Put(ptr, amount, total);
            ReleaseGet(amount);
            total += amount;
        }
        buff.ChangeSize(newmax);
        while (size < total) {
            char* ptr = AllocatePut(total, amount);
            storage.Get(ptr, amount, size);
            ReleasePut(amount);
        }
    } else {
        buff.ChangeSize(newmax);
        // Only need to copy when there exists
        // data 'above' the put position
        // shift it to the end
        if (get >= put && size != 0) {
            unsigned oldget = get;
            get = oldget +  newmax - oldmax;
            memmove(buff.GetBuffer(get),
                    buff.GetBuffer(oldget),
                    oldmax - oldget);
        }
    }
}

void AutoCircleBuffer::EnsureMaxSize(unsigned newsize) {
    if (MaxSize() < newsize) {
        ChangeMaxSize(newsize);
    }
}


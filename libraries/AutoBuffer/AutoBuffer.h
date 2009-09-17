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
 * Header for the AutoBuffer classes. The concept of the
 * auto buffer is very simple. Basically
 * a class who's job it is to maintain the lifetime
 * of a variable buffer. StaticBuffer and StaticConstBuffer
 * are also provided to represent when we want to pass around
 * a buffer reference that cannot change size and a buffer
 * reference that cannot be changed respectively.
 * \author John Bridgman
 */

#ifndef AUTOBUFFER_H
#define AUTOBUFFER_H
#pragma once

#include "StaticBuffer.h"

/**
 * An AutoBuffer handles a dynamically sized buffer of raw memory
 * automatically. This class also provides a few convenience
 * methods for dealing with the raw memory.
 *
 * An AutoBuffer also keeps track of a position. The position
 * is automatically added to all offsets. This is so you can write
 * some to the memory move the position forward then hand off the
 * buffer to another function that write to the beginning starting
 * at the position you set. All constructors start the position
 * off at zero. The copy constructor and copy operator copy the
 * contents of the other buffer starting at the position.
 * All get buffer and void* cast operators return a pointer to the
 * current position. The comparison operator only compares
 * what is after the position.
 *
 * The actual memory size inside an auto buffer will be always
 * 2^N such that size is always less than 2^N. This is to help
 * with memory fragmentation and keep the number of allocation
 * down as the buffer increases in size.
 */
class AutoBuffer {
public:
    typedef unsigned long ulong;
    /// Initial size of 0
    AutoBuffer();
    /// Start with initialsize
    AutoBuffer(ulong initialsize);
    /// Copy other buffer
    AutoBuffer(const AutoBuffer& other);
    /// Copy a pointer
    AutoBuffer(const void* other, ulong othersize);
    /// Copy a const static buffer
    AutoBuffer(const StaticConstBuffer& other);
    ~AutoBuffer();
    /// Copy contents of other buffer into us destroying us 
    AutoBuffer& operator=(const AutoBuffer& other);
    /**
     * \return a pointer to the buffer.
     */
    void* GetBuffer() { return GetBuffer(0); }

    /**
     * \return a const pointer to the buffer.
     */
    const void* GetBuffer() const { return GetBuffer(0); }
    /**
     * \param offset how many bytes into the buffer
     * \return a pointer into the buffer at the given offset
     */
    void* GetBuffer(ulong offset);

    /**
     * \param offset number of bytes into the buffer
     * \return a const pointer into the buffer at the given offset
     */
    const void* GetBuffer(ulong offset) const;

    operator void*() { return GetBuffer(0); }
    operator const void* () const { return GetBuffer(0); }

    /** Set the current position.
     * \param newpos the new position in bytes from the beginning.
     */
    void SetPos(ulong newpos);
    /** \return the current position */
    ulong GetPos() { return pos; }
    /** Add to the current position for relative offsets.
     * \param amount the amount to add to the position
     * \return the new position
     */
    ulong AddPos(ulong amount);
    /**
     * \return the current size of the buffer.
     */
    ulong GetSize() const { return size - pos; }

    /**
     * Change the size of this buffer. If newsize is larger than
     * current size then the buffer will be extended. The data
     * already in buffer will remain unchanged. See realloc.
     * If newsize is smaller then the buffer will be truncated.
     * \param newsize the new size in bytes for this buffer
     */
    void ChangeSize(ulong newsize);

    /**
     * Ensure that the size of the buffer is at least newsize.
     * \param newsize the new size
     */
    void EnsureSize(ulong newsize);

    /**
     * Copy othersize bytes from other into this buffer starting at
     * offset. If there is not enough space this buffer will try to expand.
     * \param other the other memory buffer
     * \param othersize the number of bytes to copy
     * \param offset the offset into us to start copying at
     */
    void Put(const void* other, const ulong othersize, ulong offset=0);

    /**
     * Copy a StaticConstBuffer into us starting at the current offset
     * increase our size if necessary.
     * \param other the StaticConstBuffer
     * \param offset (default 0) how many bytes into the buffer to start
     * copying
     */
    void Put(const StaticConstBuffer& other, const ulong offset=0) {
        Put(other.GetBuffer(), other.GetSize(), offset);
    }

    /**
     * A templatize version of Put for placing an array of a specific
     * type.
     * \param offset how many bytes into the buffer
     * \param other a pointer to the object to copy
     * \param count the number to copy
     */
    template <class T>
    void Put(const ulong offset, const T* other, ulong count = 1) {
        Put(other, sizeof(T) * count, offset);
    }

    /**
     * Copy up to othersize bytes into other starting at offset.
     * If othersize+offset is larger than our size then only copy up to the
     * end of us. Returns the number of bytes actually copied.
     * \param other the other buffer to copy into
     * \param othersize the number of bytes to copy into other
     * \param offset where to start copying in us
     * \return the number of bytes actually copied
     */
    ulong Get(void* other, ulong othersize, ulong offset=0) const;

    /**
     * Copy from us into other up to the end of other.
     * \param other a static buffer to copy into
     * \param offset offset into the buffer to start copying from
     * \return the number of bytes copied into other
     */
    ulong Get(StaticBuffer& other, ulong offset=0) const {
        return Get(other.GetBuffer(), other.GetSize(), offset);
    }

    /**
     * Template version of Get
     * \param offset number of bytes into us to start copying from
     * \param other pointer to copy to
     * \param count number of objects to copy
     * \return the number of bytes copied, may not be a multiple of sizeof(T)
     */
    template <class T>
    ulong Get(ulong offset, T* other, ulong count = 1) const {
        return Get(other, sizeof(T) * count, offset);
    }

    /**
     * A convenience function to obtain a StaticBuffer pointing into
     * our buffer with a given size.
     * This function expands this buffer if needed.
     * \note The returned buffer will point to invalid memory if ChangeSize
     * is ever called when StaticBuffer exists.
     * \param buffsize the size the StaticBuffer should be
     * \param offset (default 0) the offset into us to start at
     */
    StaticBuffer GetStaticBuffer(ulong buffsize, ulong offset=0);

    /**
     * A convenience function to obtain a StaticConstBuffer pointing into
     * our buffer with a given size.
     * This function will not expand this buffer if needed but will instead
     * truncate the size of the StaticConstBuffer.
     * \param buffsize the desired size of the StaticConstBuffer
     * \param offset (default 0) the offset into us to start at
     */
    StaticConstBuffer GetStaticConstBuffer(ulong buffsize, ulong offset=0) const;

private:
    void ChangeMemSize(ulong newsize);
    void* buffer;
    ulong size;
    ulong memsize;
    ulong pos;
};

#endif

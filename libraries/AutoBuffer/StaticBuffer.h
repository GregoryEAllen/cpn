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
 * Header for the StaticBuffer and StaticConstbuffer classes.
 * The concept of the static buffer is simply to hold a void* and size.
 * The concet of the static const buffer is to hold a const void* and size
 * It does not maintain the lifetime of the memory
 * pointed to the void* The user is responsible for this.
 * \author John Bridgman
 */

#ifndef STATICBUFFER_H
#define STATICBUFFER_H
#pragma once

class AutoBuffer;
class StaticConstBuffer;

/**
 * A convenience container for a void* and size pair.
 * The user is responsible for maintaining the lifetiem
 * of the void*
 * Note that a const StaticBuffer just means that you
 * cannot change the internal pointer not the contents
 * of the pointer just like a const void*
 */
class StaticBuffer {
public:
    typedef unsigned long ulong;
    /// point to same void* and size as other
    StaticBuffer(const StaticBuffer& other);
    /// Init with given
    StaticBuffer(void* other, ulong othersize);
    /// point to GetBuffer() from other
    /// Warning if other changes this wont!
    StaticBuffer(AutoBuffer& other);
    ~StaticBuffer();
    /// Cause us to point to same void* and size
    StaticBuffer& operator=(const StaticBuffer& other);

    /**
     * \return a pointer to the buffer.
     */
    void* GetBuffer() const { return buffer; }

    /**
     * \return a pointer into buffer at the given offset
     */
    void* GetBuffer(ulong offset) const;

    operator void*() const { return buffer; }

    /**
     * \return the current size of the buffer.
     */
    ulong GetSize() const { return size; }

    /**
     * Copy othersize bytes from other into this buffer starting at
     * offset. If there is not enough space this buffer will try copy
     * as many as it can and return the number copied.
     * \param other the other memory buffer
     * \param othersize the number of bytes to copy
     * \param offset the offset into us to start copying at
     * \return the number actually copied
     */
    ulong Put(const void* other, ulong othersize, ulong offset=0) const;

    /// Same as above but use a StaticConstBuffer
    ulong Put(const StaticConstBuffer& other, ulong offset=0) const;

    /// Same as above but allows for an array of a type
    template <class T>
    ulong Put(ulong offset, const T* other, ulong count = 1) const {
        return Put(other, sizeof(T) * count, offset);
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

    /// Same as above except use a StaticBuffer
    ulong Get(const StaticBuffer& other, ulong offset=0) const {
        return Get(other.GetBuffer(), other.GetSize(), offset);
    }

    /// Same as above but allows for an array of a type
    template <class T>
    ulong Get(ulong offset, T* other, ulong count = 1) const {
        return Get(other, sizeof(T) * count, offset);
    }

private:
    void* buffer;
    ulong size;
};

class StaticConstBuffer {
public:
    typedef unsigned long ulong;
    /// Copy constructor
    StaticConstBuffer(const StaticConstBuffer& other);
    /// Create a StaticConstBuffer from a StaticBuffer
    StaticConstBuffer(const StaticBuffer& other);
    /// Create a StaticConstbuffer from an AutoBuffer
    /// \warning if the AutoBuffer's ChangeSize is ever called
    /// we will point to invalid memory
    StaticConstBuffer(const AutoBuffer& other);
    /// Create a StaticConstBuffer from a pointer and size
    StaticConstBuffer(const void* other, ulong othersize);
    ~StaticConstBuffer();
    /// Make us point to the same pointer and size as other
    StaticConstBuffer& operator=(const StaticConstBuffer& other);
    /// \return a const void* to our buffer
    const void* GetBuffer() const { return buffer; }
    /// \return a const void* to our buffer with an offset
    const void* GetBuffer(ulong offset) const;

    operator const void* () const { return buffer; }
    
    /// \return the size of this buffer
    ulong GetSize() const { return size; }

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

    /// Same as above except use a StaticBuffer
    ulong Get(const StaticBuffer& other, ulong offset=0) const {
        return Get(other.GetBuffer(), other.GetSize(), offset);
    }

    /// Same as above but allows for an array of a type
    template <class T>
    ulong Get(ulong offset, T* other, ulong count = 1) const {
        return Get(other, sizeof(T) * count, offset);
    }

private:
    const void* buffer;
    ulong size;
};

/**
 * Comparison operator for two buffers. This function compares
 * the two buffers form where GetBuffer() returns to the size.
 * If every byte is equal and they are of equal size they are the same.
 * \param rhs the right hand size
 * \param lhs the left hand size
 * \return true if they are byte equal
 */
bool operator==(const StaticConstBuffer& rhs, const StaticConstBuffer& lhs);
bool operator!=(const StaticConstBuffer& rhs, const StaticConstBuffer& lhs);

inline StaticBuffer::ulong StaticBuffer::Put(const StaticConstBuffer& other, ulong offset) const {
    return Put(other.GetBuffer(), other.GetSize(), offset);
}

#endif

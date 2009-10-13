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
 * \brief A very simple circle buffer (not circular).
 * \author John Bridgman
 */

#ifndef AUTOCIRCLEBUFFER_H
#define AUTOCIRCLEBUFFER_H
#pragma once

#include "AutoBuffer.h"
#include "StaticBuffer.h"

/**
 * This class is NOT a circular buffer but something slightly like it.
 * This class maintains a circle with the data but responds with
 * less space up to the end of the buffer if it cannot.
 * The idea with this is that you write a partial amount and try again.
 */
class AutoCircleBuffer {
public:
    /// Create an AutoCircleBuffer with a default initial size
    AutoCircleBuffer();
    /// Create an AutoCircleBuffer with the given initial size.
    AutoCircleBuffer(int initialsize);
    ~AutoCircleBuffer() {}
    /// \return the current maximum amount that can fit
    unsigned MaxSize() const { return buff.GetSize(); }
    /// \return the number of bytes of data this AutoCircleBuffer has
    unsigned Size() const { return size; }
    /// Reset the buffer and loose all data currently in it
    void Reset() { size = put = get = 0; }
    /**
     * Try to return a pointer into the buffer with desired
     * number of bytes available to write. Actual is the amount
     * actually available.
     * \param desire the desired number of bytes
     * \param actual (return) the number of bytes available
     * \return a pointer into the buffer to write to
     */
    char* AllocatePut(unsigned desired, unsigned &actual);
    /**
     * Indicate that amount bytes have been written to the buffer.
     * \param amount the number of bytes written
     */
    void ReleasePut(unsigned amount);
    /**
     * Put the given byte array into this buffer and expand the
     * buffer if needed.
     * \param ptr the byte array
     * \param amount the length of the array
     */
    void Put(const char* ptr, unsigned amount);
    void Put(const StaticConstBuffer& other) {
        Put((const char*)other.GetBuffer(), other.GetSize());
    }
    /**
     * Try to return a pointer into the buffer with the desired
     * number of bytes available to read.
     * actual is the number of bytes actually available to read.
     * \param desired the number of bytes we want to read
     * \param actual (return) the actual number of bytes available
     * in one read command
     * \return the pointer into the buffer
     */
    char* AllocateGet(unsigned desired, unsigned &actual);
    /**
     * This is the dual of ReleasePut. This function
     * tells this object that amount bytes have been
     * taken from the buffer and should be discarded.
     * \param amount the number of bytes removed.
     */
    void ReleaseGet(unsigned amount);
    /**
     * Get amount characters form this buffer and place them in
     * ptr array.
     * Fail if there are less than amount bytes.
     * \param ptr destination byte array
     * \param amount number of bytes
     * \return true on success false if there are not amount bytes in buffer.
     */
    bool Get(char* ptr, unsigned amount);
    bool Get(const StaticBuffer& other) {
        return Get((char*)other.GetBuffer(), other.GetSize());
    }
    /**
     * Change the size of the buffer.
     * Will not make the size smaller than the current
     * amount in the buffer.
     * Will change it to the closest greater power of two.
     * Minimum size of 256.
     * \note This might do a double copy so is rather
     * inefficient.
     * \param newsize the new size of the buffer
     */
    void ChangeMaxSize(unsigned newsize);
    /**
     * Ensure that the size of the buffer is at least
     * newsize.
     * \param newsize the size
     */
    void EnsureMaxSize(unsigned newsize);
private:
    AutoBuffer buff;
    unsigned size;
    unsigned put;
    unsigned get;
};

#endif


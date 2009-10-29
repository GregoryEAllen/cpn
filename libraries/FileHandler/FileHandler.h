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
 * A class to make it easy to deal with file descriptors.
 * \author John Bridgman
 */
#pragma once

// for the iovec struct
#include <sys/uio.h>

/**
 * Generic file handler
 * could be a file, or a socket or a device.
 *
 * If needed wouldn't be very hard to add an open
 * function here.
 */
class FileHandler {
public:

    static int Poll(FileHandler **fileds, unsigned numfds, double timeout);

    FileHandler();
    FileHandler(int filed);
    virtual ~FileHandler();

    int Poll(double timeout);

    virtual void OnRead() = 0;
    virtual void OnWrite() = 0;
    virtual void OnError() = 0;
    virtual void OnHup() = 0;
    virtual void OnInval() = 0;

    bool Readable(bool r) { return readable = r; }
    bool Readable() const { return readable; }
    bool Writeable(bool w) { return writeable = w; }
    bool Writeable() const { return writeable; }

    int FD() const { return fd; }
    int FD(int filed) { return fd = filed; }

    bool Eof() const { return eof; }
    bool Eof(bool e) { return eof = e; }
    bool Good() const { return !(eof || fd == -1); }
    bool Closed() const { return fd == -1; }

    void SetBlocking(bool blocking);
    bool IsBlocking() const;

    /**
     * Clear all internal state including the file
     * descriptor!
     */
    void Reset();

    void Close();
    unsigned Read(void *ptr, unsigned len);
    unsigned Readv(const iovec *iov, int iovcnt);
    unsigned Write(const void *ptr, unsigned len);
    unsigned Writev(const iovec *iov, int iovcnt);

    void Flush();
protected:
    int fd;
    bool readable;
    bool writeable;
    bool eof;
private:
    FileHandler(const FileHandler&);
    FileHandler &operator=(const FileHandler&);
};


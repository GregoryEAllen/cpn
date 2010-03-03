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
 * \brief A class to make it easy to deal with file descriptors.
 * \author John Bridgman
 */
#ifndef FILEHANDLE_H
#define FILEHANDLE_H
#pragma once

#include "PthreadMutex.h"
#include "AutoLock.h"
#include "IteratorRef.h"
// for the iovec struct
#include <sys/uio.h>

/**
 * \brief Generic file handle
 * could be a file, or a socket or a device.
 *
 * If needed wouldn't be very hard to add an open
 * function here.
 */
class FileHandle {
public:
    typedef Sync::AutoLock<PthreadMutex> AutoLock;

    /**
     * \brief poll a list of FileHandles for any activity and call the
     * appropriate On method.
     */
    static int Poll(IteratorRef<FileHandle*> begin, IteratorRef<FileHandle*> end, double timeout);

    /**
     * \brief Construct a closed FileHandle.
     */
    FileHandle();

    /**
     * \brief Construct a FileHandle with filed as the open
     * file descriptor
     * \param filed the file descriptor to use
     */
    FileHandle(int filed);
    
    /** \brief Close the file descriptor. Use Reset if one wants to
     * not close the file descriptor.
     */
    virtual ~FileHandle();

    /**
     * Poll the current file descriptor for activity specified
     * by the current readable or writeable status, if false poll that value,
     * and call one of OnWriteable or OnReadable whos default action is to set
     * Readable or Writeable to true.
     * \param timeout -1 to wait forever for activity
     * 0 to poll and return immediately or a time to wait
     * in seconds.
     * \return zero if timed out, positive if an event occurred
     * or negative if an error occurred.
     */
    int Poll(double timeout);

    /**
     * \brief Set that the file is currently readable or not.
     * \param r true or false
     * \return the new readable status
     */
    bool Readable(bool r) { AutoLock al(file_lock); return readable = r; }
    /**
     * \brief Gives the current readability status of the file.
     * \return true if it is known that a read will not block
     */
    bool Readable() const { AutoLock al(file_lock); return readable; }
    /**
     * \brief Set that this file is currently writeable or not
     * \param w true or false
     * \return the new writeable status
     */
    bool Writeable(bool w) { AutoLock al(file_lock); return writeable = w; }
    /**
     * \brief Gives the current writability status of the file
     * \return true if it is known that a write will not block
     */
    bool Writeable() const { AutoLock al(file_lock); return writeable; }

    /** \return the current set file descriptor
     */
    int FD() const { AutoLock al(file_lock); return fd; }
    /** \brief Set the current file descriptor
     * \param filed the new file descriptor
     * \return the new file descriptor
     */
    int FD(int filed) { AutoLock al(file_lock); return fd = filed; }

    /** \return the current end of file condition
     */
    bool Eof() const { AutoLock al(file_lock); return eof; }
    /** \brief Set/reset the end of file condition.
     * \param e the new end of file condition
     * \return the new end of file condition
     */
    bool Eof(bool e) { AutoLock al(file_lock); return eof = e; }
    /** \brief Convenience method for testing if
     * the file this FileHandle has is open and not at end of file
     * \return true if open and not at end of file otherwise false
     */
    bool Good() const { AutoLock al(file_lock); return !(eof || fd == -1); }
    /** \return false if this FileHandle has an open file
     */
    bool Closed() const { AutoLock al(file_lock); return fd == -1; }

    /** \brief Manipulate how the current file handles blocking
     * \param blocking true to set to blocking mode (default)
     * false to set to non blocking mode.
     *
     * In non blocking mode all reads and writes will not block.
     * Use Poll to block.
     */
    void SetBlocking(bool blocking);
    /** \brief a convenience method that allows one to set the
     * same blocking parameters for a file descriptor without
     * setting it inside a FileHandle.
     * \param fd the file descriptor
     * \param blocking true or false see SetBlock
     */
    static void SetBlocking(int fd, bool blocking);
    /**
     * \brief Test if the current file is in blocking or non blocking mode
     * \return true if blocking or false if non blocking
     */
    bool IsBlocking() const;
    /**
     * \brief Test the given file descriptor if it is in blocking mode.
     * \param fd the file descriptor to test
     * \return true if blocking or false if non blocking
     */
    static bool IsBlocking(int fd);

    /**
     * \brief Clear all internal state including the file
     * descriptor! WARNING does not close the file!
     */
    void Reset();

    /**
     * \brief Close the file and reset the internal state.
     */
    void Close();

    /**
     * \brief Read data from the file descriptor.
     *
     * Will set the end of file condition if read detects end of file.
     * \note All the read functions will set readable to false if they
     * read less than the requested amount or we are non blocking and
     * a would block condition happened.
     * \param ptr pointer to write data to
     * \param len the maximum number of bytes to write to ptr
     * \return 0 if no bytes read (check Eof)
     * or the number of bytes read
     */
    unsigned Read(void *ptr, unsigned len);
    /**
     * \brief scatter gather io version of Read
     * \return see Read
     */
    unsigned Readv(const iovec *iov, int iovcnt);
    /**
     * \brief Write data to the file descriptor.
     *
     * \note All write functions will set Writeable to false
     * if they write less than the amount requested or if
     * the file is in non blocking mode and the write returned
     * would block.
     * \param ptr pointer to beginning of data to write
     * \param len length of data to write
     * \return number of bytes written
     */
    unsigned Write(const void *ptr, unsigned len);
    /** \brief scatter gather io version of Write
     * \return see Write
     */
    unsigned Writev(const iovec *iov, int iovcnt);

    /**
     * \brief Tell the OS to flush any buffers it has.
     * May not be supported for all file types.
     */
    void Flush();
protected:
    /** \brief Called by Poll when it detects that the file is readable. */
    virtual void OnReadable() { Readable(true); }
    /** \brief Called by Poll when it detects that the file is writeable. */
    virtual void OnWriteable() { Writeable(true); }
    mutable PthreadMutex file_lock;
    int fd;
    bool readable;
    bool writeable;
    bool eof;
private:
    FileHandle(const FileHandle&);
    FileHandle &operator=(const FileHandle&);
};
#endif

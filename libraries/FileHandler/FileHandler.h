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
#pragma once

// for the iovec struct
#include <sys/uio.h>

/**
 * \brief Generic file handler
 * could be a file, or a socket or a device.
 *
 * If needed wouldn't be very hard to add an open
 * function here.
 */
class FileHandler {
public:

    /**
     * \brief poll a list of FileHandlers for any activity and call the
     * appropriate On method.
     */
    static int Poll(FileHandler **fileds, unsigned numfds, double timeout);

    /**
     * \breif Construct a closed FileHandler.
     */
    FileHandler();
    /**
     * \breif Construct a FileHandler with filed as the open
     * file descriptor
     * \param filed the file descriptor to use
     */
    FileHandler(int filed);
    virtual ~FileHandler();

    /**
     * \brief Poll the current file descriptor for activity calling
     * one of the On function if appropriate.
     * \param timeout -1 to wait forever for activity
     * 0 to poll and return immediately or a time to wait
     * in seconds.
     * \return zero if timed out, posiive if an event occured
     * or negative if an error occured.
     */
    int Poll(double timeout);

    /**
     * \brief Called by the Poll functions when the file can be read from without blocking.
     */
    virtual void OnRead() = 0;
    /**
     * \brief Called by the Poll functions when the file can be written to without blocking.
     */
    virtual void OnWrite() = 0;
    /**
     * \brief Called by Poll when an error has been detected.
     */
    virtual void OnError() = 0;
    /**
     * \brief Called by Poll when a hangup has been detected.
     */
    virtual void OnHup() = 0;
    /**
     * \brief Called by Poll when the file descritor is invalid.
     */
    virtual void OnInval() = 0;

    /**
     * \brief Set the readable status of this FileHandler
     * \param r true or false
     * \return the new readable status
     */
    bool Readable(bool r) { return readable = r; }
    /**
     * \breif Called by Poll to decide if poll should check for readability
     * \return the readable status
     */
    virtual bool Readable() const { return readable; }
    /**
     * \brief Set the writeable status of this FileHandler
     * \param w true or false
     * \return the new writeable status
     */
    bool Writeable(bool w) { return writeable = w; }
    /**
     * \brief Called by Poll to decide if poll shoudl check for writeability.
     * \return the writeable status
     */
    virtual bool Writeable() const { return writeable; }

    /** \return the current set file descriptor
     */
    int FD() const { return fd; }
    /** \brief Set the current file descriptor
     * \param filed the new file descriptor
     * \return the new file descriptor
     */
    int FD(int filed) { return fd = filed; }

    /** \return the current end of file condition
     */
    bool Eof() const { return eof; }
    /** \brief Set/reset the end of file condition.
     * \param e the new end of file condition
     * \return the new end of file condition
     */
    bool Eof(bool e) { return eof = e; }
    /** \brief Convenience method for testing if
     * the file this FileHandler has is open and not at end of file
     * \return true if open and not at end of file otherwise false
     */
    bool Good() const { return !(eof || fd == -1); }
    /** \return false if this FileHandler has an open file
     */
    bool Closed() const { return fd == -1; }

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
     * settign it inside a FileHandler.
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
     * \breif Read data from the file descriptor.
     *
     * Will set the end of file condition if read detects end of file.
     * \param ptr pointer to write data to
     * \param len the maximum number of bytes to write to ptr
     * \return 0 if no bytes read (check Eof)
     * or the number of bytes read
     */
    unsigned Read(void *ptr, unsigned len);
    /**
     * \breif scatter gather io version of Read
     * \return see Read
     */
    unsigned Readv(const iovec *iov, int iovcnt);
    /**
     * \breif Write data to the file descriptor.
     *
     * \param ptr pointer to beginning of data to write
     * \param len length of data to write
     * \return number of bytes written
     */
    unsigned Write(const void *ptr, unsigned len);
    /** \breif scatter gather io version of Write
     * \return see Write
     */
    unsigned Writev(const iovec *iov, int iovcnt);

    /**
     * \breif Tell the OS to flush any buffers it has.
     * May not be supported for all file types.
     */
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


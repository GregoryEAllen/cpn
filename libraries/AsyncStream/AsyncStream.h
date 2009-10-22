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
 * \brief An asynchronious stream implementation.
 * \author John Bridgman
 */

#ifndef ASTREAM_ASYNCSTREAM_H
#define ASTREAM_ASYNCSTREAM_H
#pragma once
#include <tr1/memory>
#include <vector>
#include <exception>
#include <sigc++/sigc++.h>

namespace Async {

    /**
     * Generic exception that the Async library throws on error.
     * what returns the error description.
     */
    class StreamException : public std::exception {
    public:
        StreamException(int err) throw();
        StreamException(const char *msg, int err) throw();
        virtual ~StreamException() throw();
        virtual const char* what() const throw();
        int Error() const { return error; }
    private:
        int error;
        std::vector<char> errstr;
    };

    class Stream;


    class Descriptor;
    typedef std::tr1::shared_ptr<Descriptor> DescriptorPtr;

    /** A Descriptor is a generic resource from the operating system that has
     * the properties that it can be closed. It can have an end.  It can be
     * polled in which case it will signal events OnWrite OnRead and OnError.
     * The Readable and Writeable events are used to check if you wish to check
     * for readability or writeability when polled.  You can set a Descriptor
     * into blocking and non blocking mode.  Non blocking is default.
     */
    class Descriptor {
        friend class Stream;
    public:

        static DescriptorPtr Create(int fd);

        static int Poll(std::vector<DescriptorPtr> &fildes, int timeout);

        virtual ~Descriptor() throw();

        /**
         * close the file descriptor
         * \throws StreamException on error
         */
        void Close() throw(StreamException);

        bool Eof() const throw() { return eof; }
        bool Closed() const throw() { return fd < 0; }
        operator bool() const throw() { return !Eof() && !Closed(); }
        
        sigc::connection ConnectReadable(sigc::slot<bool> slot) {
            return readable.connect(slot);
        }

        sigc::connection ConnectWriteable(sigc::slot<bool> slot) {
            return writeable.connect(slot);
        }

        sigc::connection ConnectOnRead(sigc::slot<void> slot) {
            return read.connect(slot);
        }

        sigc::connection ConnectOnWrite(sigc::slot<void> slot) {
            return write.connect(slot);
        }

        sigc::connection ConnectOnError(sigc::slot<void, int> slot) {
            return error.connect(slot);
        }

        void SetNonBlocking(bool nonblocking);
    protected:
        Descriptor(int fd_);
        Descriptor();
        Descriptor(const Descriptor&);
        Descriptor &operator=(const Descriptor&);
        void SetEof() { eof = true; }

        int fd;
        bool eof;
        sigc::signal<bool> readable;
        sigc::signal<bool> writeable;
        sigc::signal<void> read;
        sigc::signal<void> write;
        sigc::signal<void, int> error;
    };


    /**
     * A Stream is a wrapper around a file descriptor that lets you read and write
     * to that descriptor.
     */
    class Stream {
    public:

        Stream(DescriptorPtr desc) throw();

        /**
         * Attempt to read some data from the stream.
         * \param ptr pointer memory at least len long to write to
         * \param len the maximum amount to read
         * \return between 0 and len on success, if 0 eof should
         * be checked to see if we are at the end of the stream
         * \throws StreamException on error
         */
        unsigned Read(void *ptr, unsigned len) throw(StreamException);

        /**
         * Attempt to write some data to the stream.
         * \param ptr pointer to memory containing len bytes to write
         * \param len the maximum number of bytes to try to write
         * \return between 0 and len on success.
         * \throws StreamException on error
         */
        unsigned Write(const void *ptr, unsigned len) throw(StreamException);

        /**
         * Close this stream.
         * \throws StreamException on error
         */
        void Close() throw(StreamException) { descriptor->Close(); }

        bool Eof() const throw() { return descriptor->Eof(); }
        bool Closed() const throw() { return descriptor->Closed(); }
        operator bool() const throw() { return *descriptor; }
        DescriptorPtr Fd() throw() { return descriptor; }

    private:
        DescriptorPtr descriptor;
    };
}

#endif


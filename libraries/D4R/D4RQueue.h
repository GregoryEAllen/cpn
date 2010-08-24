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
 * \author John Bridgman
 */
#ifndef D4R_QUEUE_H
#define D4R_QUEUE_H
#pragma once
#include <tr1/memory>
namespace D4R {

    using std::tr1::shared_ptr;

    class Node;

    class QueueBase {
    public:
        QueueBase();
        virtual ~QueueBase();

        void SetReaderNode(shared_ptr<Node> n);
        void SetWriterNode(shared_ptr<Node> n);

    protected:
        /**
         * reader ===> writer
         * ReadBlock requires that you already hold the lock
         * and if it is reentrant then a single unlock will
         * release the lock.
         */
        void ReadBlock();

        /**
         * writer ===> reader
         * WriteBlock requires that you already hold the lock
         * and if it is reentrant then a single unlock will
         * release the lock.
         */
        void WriteBlock(unsigned qsize);

        virtual bool ReadBlocked() = 0;
        virtual bool WriteBlocked() = 0;

        virtual void Detect() = 0;

    public:
        virtual void Lock() const = 0;
        virtual void Unlock() const = 0;

        virtual void SignalReaderTagChanged();
        virtual void SignalWriterTagChanged();
    private:
        QueueBase(const QueueBase&);
        QueueBase &operator=(const QueueBase&);

    protected:
        virtual void Wait() = 0;
        virtual void Signal() = 0;

        shared_ptr<Node> reader;
        shared_ptr<Node> writer;
        bool readtagchanged;
        bool writetagchanged;
        bool incomm;
    };

}
#endif

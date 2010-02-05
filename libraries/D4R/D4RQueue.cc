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
#include "D4RQueue.h"
#include "D4RNode.h"
#include "D4RDeadlockException.h"
#include "AutoLock.h"
#include "AutoUnlock.h"
#include "Assert.h"

namespace D4R {

    QueueBase::QueueBase()
        : reader(0),
        writer(0),
        readtagchanged(false),
        writetagchanged(false)
    {}

    QueueBase::~QueueBase() {}

    void QueueBase::SetReaderNode(Node *n) {
        Sync::AutoLock<QueueBase> al(*this);
        reader = n;
        Signal();
    }

    void QueueBase::SetWriterNode(Node *n) {
        Sync::AutoLock<QueueBase> al(*this);
        writer = n;
        Signal();
    }

    void QueueBase::ReadBlock() {
        if (!writer) {
            while (ReadBlocked() && !writer) {
                Wait();
            }
        }
        if (!ReadBlocked()) { return; }
        writetagchanged = false;
        {
            AutoUnlock<QueueBase> au(*this);
            reader->Block(writer->GetPublicTag(), -1);
        }
        while (ReadBlocked()) {
            if (writetagchanged) {
                writetagchanged = false;
                bool detect;
                {
                    AutoUnlock<QueueBase> au(*this);
                    detect = reader->Transmit(writer->GetPublicTag());
                }
                if (detect) {
                    throw DeadlockException("True deadlock detected");
                }
            } else {
                Wait();
            }
        }
    }

    void QueueBase::WriteBlock(unsigned qsize) {
        if (!reader) {
            while (WriteBlocked() && !reader) {
                Wait();
            }
        }
        if (!WriteBlocked()) { return; }
        readtagchanged = false;
        {
            AutoUnlock<QueueBase> au(*this);
            writer->Block(reader->GetPublicTag(), qsize);
        }
        while (WriteBlocked()) {
            if (readtagchanged) {
                readtagchanged = false;
                bool detect;
                {
                    AutoUnlock<QueueBase> au(*this);
                    detect = writer->Transmit(reader->GetPublicTag()); 
                }
                if (detect) { Detect(); }
            } else {
                Wait();
            }
        }
    }

    void QueueBase::SignalReaderTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        readtagchanged = true;
        Signal();
    }

    void QueueBase::SignalWriterTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        writetagchanged = true;
        Signal();
    }

}


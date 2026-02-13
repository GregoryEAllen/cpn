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

#if _DEBUG
#include <stdio.h>
#define DEBUG(fmt, ...) fprintf(stderr, fmt, ## __VA_ARGS__)
#else
#define DEBUG(fmt, ...)
#endif

template <typename T>
class ScopeSetter {
public:
    ScopeSetter(T &val_, T set) : val(val_), old(val)
    {
        val = set;
    }
    ~ScopeSetter() { val = old; }
    T &val;
    T old;
};

namespace D4R {

    QueueBase::QueueBase()
        : readtagchanged(false),
        writetagchanged(false),
        incomm(false)
    {}

    QueueBase::~QueueBase() {}

    void QueueBase::SetReaderNode(shared_ptr<Node> n) {
        AutoLock<QueueBase> al(*this);
        reader = n;
        Signal();
    }

    void QueueBase::SetWriterNode(shared_ptr<Node> n) {
        AutoLock<QueueBase> al(*this);
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
        try {
            while (incomm) { Wait(); }
            ScopeSetter<bool> ss(incomm, true);
            AutoUnlock<QueueBase> au(*this);
            reader->Block(writer->GetPublicTag(), -1);
        } catch (...) { Signal(); throw; }
        Signal();
        while (ReadBlocked()) {
            if (writetagchanged) {
                writetagchanged = false;
                bool detect;
                try {
                    while (incomm) { Wait(); }
                    ScopeSetter<bool> ss(incomm, true);
                    AutoUnlock<QueueBase> au(*this);
                    detect = reader->Transmit(writer->GetPublicTag());
                } catch (...) { Signal(); throw; }
                Signal();
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
        try {
            while (incomm) { Wait(); }
            ScopeSetter<bool> ss(incomm, true);
            AutoUnlock<QueueBase> au(*this);
            writer->Block(reader->GetPublicTag(), qsize);
        } catch (...) { Signal(); throw; }
        Signal();
        while (WriteBlocked()) {
            if (readtagchanged) {
                readtagchanged = false;
                bool detect;
                try {
                    while (incomm) { Wait(); }
                    ScopeSetter<bool> ss(incomm, true);
                    AutoUnlock<QueueBase> au(*this);
                    detect = writer->Transmit(reader->GetPublicTag()); 
                } catch (...) { Signal(); throw; }
                Signal();
                if (detect) { Detect(); }
            } else {
                Wait();
            }
        }
    }

    void QueueBase::SignalReaderTagChanged() {
        AutoLock<QueueBase> al(*this);
        UnlockedSignalReaderTagChanged();
    }

    void QueueBase::UnlockedSignalReaderTagChanged() {
        DEBUG("%s: (%llu -> %llu)\n", __PRETTY_FUNCTION__, (writer ? writer->GetPrivateTag().Key() : 0), (reader ? reader->GetPrivateTag().Key() : 0));
        readtagchanged = true;
        Signal();
    }

    void QueueBase::SignalWriterTagChanged() {
        AutoLock<QueueBase> al(*this);
        UnlockedSignalWriterTagChanged();
    }

    void QueueBase::UnlockedSignalWriterTagChanged() {
        DEBUG("%s: (%llu -> %llu)\n", __PRETTY_FUNCTION__, (writer ? writer->GetPrivateTag().Key() : 0), (reader ? reader->GetPrivateTag().Key() : 0));
        writetagchanged = true;
        Signal();
    }

}


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
            if (!ReadBlocked()) {
                return;
            }
        }
        readtagchanged = false;
        {
            AutoUnlock<QueueBase> aul(*this);
            reader->Block(writer->GetTag(), -1);
        }
        while (ReadBlocked()) {
            if (readtagchanged) {
                readtagchanged = false;
                AutoUnlock<QueueBase> aul(*this);
                if (reader->Transmit(writer->GetTag())) {
                    Detect(false);
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
            if (!WriteBlocked()) {
                return;
            }
        }
        writetagchanged = false;
        {
            AutoUnlock<QueueBase> aul(*this);
            writer->Block(reader->GetTag(), qsize);
        }
        while (WriteBlocked()) {
            if (writetagchanged) {
                writetagchanged = false;
                AutoUnlock<QueueBase> aul(*this);
                if (writer->Transmit(reader->GetTag())) {
                    Detect(true);
                }
            } else {
                Wait();
            }
        }
    }

    void QueueBase::SignalTagChanged() {
        Sync::AutoLock<QueueBase> al(*this);
        writetagchanged = true;
        readtagchanged = true;
        Signal();
    }

}


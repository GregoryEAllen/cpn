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
 * \brief Implementation for the QueueBase.
 * \author John Bridgman
 */
#include "QueueBase.h"
#include "QueueDatatypes.h"
#include <stdio.h>

namespace CPN {
    QueueBase::QueueBase() : shutdown(false), datatype(TypeName<void>()) {
    }

    QueueBase::~QueueBase() {}

    void QueueBase::SetReaderMessageHandler(ReaderMessageHandler *rmhan) {
        Sync::AutoReentrantLock arl(lock);
        SetSubReaderHandler(rmhan);
        cond.Signal();
    }

    ReaderMessageHandler *QueueBase::GetReaderMessageHandler() {
        return this;
    }

    void QueueBase::ClearReaderMessageHandler() {
        Sync::AutoReentrantLock arl(lock);
        SetSubReaderHandler(0);
        shutdown = true;
        cond.Signal();
    }

    void QueueBase::SetWriterMessageHandler(WriterMessageHandler *wmhan) {
        Sync::AutoReentrantLock arl(lock);
        SetSubWriterHandler(wmhan);
        cond.Signal();
    }

    WriterMessageHandler *QueueBase::GetWriterMessageHandler() {
        return this;
    }

    void QueueBase::ClearWriterMessageHandler() {
        Sync::AutoReentrantLock arl(lock);
        SetSubReaderHandler(0);
        shutdown = true;
        cond.Signal();
    }

    bool QueueBase::CheckRMH() {
        Sync::AutoReentrantLock arl(lock);
        while (GetSubReaderHandler() == 0 && !shutdown) {
            cond.Wait(lock);
        }
        return !shutdown;
    }

    bool QueueBase::CheckWMH() {
        Sync::AutoReentrantLock arl(lock);
        while (GetSubWriterHandler() == 0 && !shutdown) {
            cond.Wait(lock);
        }
        return !shutdown;
    }

    void QueueBase::LogState() {
        printf("Regular queue: size: %u count: %u thresh: %u\n",
                QueueLength(), Count(), MaxThreshold());
    }
}


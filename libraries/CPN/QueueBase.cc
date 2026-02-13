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
#include "Exceptions.h"
#include "QueueAttr.h"
#include "KernelBase.h"
#include "Context.h"
#include <sstream>
#include <string.h>

namespace CPN {


    QueueBase::QueueBase(KernelBase *k, const SimpleQueueAttr &attr)
        : readerkey(attr.GetReaderKey()),
        writerkey(attr.GetWriterKey()),
        readshutdown(false),
        writeshutdown(false),
        readrequest(0),
        writerequest(0),
        enqueuethresh(0),
        dequeuethresh(0),
        indequeue(false),
        inenqueue(false),
        kernel(k),
        useD4R(kernel->UseD4R()),
        logger(kernel->GetContext().get(), Logger::DEBUG),
        datatype(attr.GetDatatype())
    {
        std::ostringstream oss;
        oss << "Queue(" << writerkey << ", " << readerkey << ")";
        logger.Name(oss.str());
    }

    QueueBase::~QueueBase() {}

    const void *QueueBase::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        kernel->CheckTerminated();
        AutoLock<QueueBase> al(*this);
        if (indequeue) { ASSERT(dequeuethresh >= thresh); }
        else { dequeuethresh = thresh; }
        while (true) {
            const void *ptr = InternalGetRawDequeuePtr(thresh, chan);
            if (ptr || writeshutdown) {
                if (ptr) { indequeue = true; }
                return ptr;
            }
            if (readshutdown) { throw BrokenQueueException(readerkey); }
            if (thresh > UnlockedMaxThreshold() && kernel->GrowQueueMaxThreshold()) {
                //printf("Grow(%u, %u)\n", 2*thresh, thresh);
                UnlockedGrow(2*thresh, thresh);
                Signal();
            } else if (WriteBlocked() && kernel->GrowQueueMaxThreshold()) {
                UnlockedGrow(writerequest + thresh, thresh);
                Signal();
            } else {
                readrequest = thresh;
                WaitForData();
                readrequest = 0;
            }
        }
    }

    void QueueBase::Dequeue(unsigned count) {
        AutoLock<QueueBase> al(*this);
        dequeuethresh = 0;
        indequeue = false;
        if (readshutdown) { throw BrokenQueueException(readerkey); }
        InternalDequeue(count);
        NotifyFreespace();
    }

    bool QueueBase::RawDequeue(void* data, unsigned count, unsigned numChans, unsigned chanStride) {
        const void *src = GetRawDequeuePtr(count, 0);
        char *dest = (char*)data;
        if (!src) { return false; }
        memcpy(dest, src, count);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            src = GetRawDequeuePtr(count, chan);
            ASSERT(src);
            dest += chanStride;
            memcpy(dest, src, count);
        }
        Dequeue(count);
        return true;
    }

    bool QueueBase::RawDequeue(void *data, unsigned count) {
        return RawDequeue(data, count, 1, 0);
    }

    void *QueueBase::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        kernel->CheckTerminated();
        AutoLock<QueueBase> al(*this);
        if (inenqueue) { ASSERT(enqueuethresh >= thresh); }
        else { enqueuethresh = thresh; }
        bool grown = false;
        while (true) {
            void *ptr = InternalGetRawEnqueuePtr(thresh, chan);
            if (ptr) {
                inenqueue = true;
                return ptr;
            }
            if (readshutdown || writeshutdown) { throw BrokenQueueException(writerkey); }
            if (thresh > UnlockedMaxThreshold() && kernel->GrowQueueMaxThreshold()) {
                //printf("Grow(%u, %u)\n", 2*thresh, thresh);
                UnlockedGrow(2*thresh, thresh);
                Signal();
            } else if (!grown && ReadBlocked() && kernel->GrowQueueMaxThreshold()) {
                UnlockedGrow(readrequest + thresh, thresh);
                Signal();
                grown = true;
            } else {
                writerequest = thresh;
                WaitForFreespace();
                writerequest = 0;
            }
        }
    }

    void QueueBase::Enqueue(unsigned count) {
        AutoLock<QueueBase> al(*this);
        enqueuethresh = 0;
        inenqueue = false;
        if (writeshutdown) { throw BrokenQueueException(writerkey); }
        InternalEnqueue(count);
        NotifyData();
    }

    void QueueBase::RawEnqueue(const void *data, unsigned count, unsigned numChans, unsigned chanStride) {
        void *dest = GetRawEnqueuePtr(count, 0);
        const char *src = (char*)data;
        memcpy(dest, src, count);
        for (unsigned chan = 1; chan < numChans; ++chan) {
            dest = GetRawEnqueuePtr(count, chan);
            src += chanStride;
            memcpy(dest, src, count);
        }
        Enqueue(count);
    }


    void QueueBase::RawEnqueue(const void* data, unsigned count) {
        return RawEnqueue(data, count, 1, 0);
    }

    unsigned QueueBase::NumChannels() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedNumChannels();
    }

    unsigned QueueBase::Count() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedCount();
    }

    bool QueueBase::Empty() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedEmpty();
    }

    unsigned QueueBase::Freespace() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedFreespace();
    }

    bool QueueBase::Full() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedFull();
    }

    unsigned QueueBase::MaxThreshold() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedMaxThreshold();
    }

    unsigned QueueBase::QueueLength() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedQueueLength();
    }

    unsigned QueueBase::EnqueueChannelStride() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedEnqueueChannelStride();
    }

    unsigned QueueBase::DequeueChannelStride() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedDequeueChannelStride();
    }

    void QueueBase::Grow(unsigned queueLen, unsigned maxThresh) {
        AutoLock<QueueBase> al(*this);
        UnlockedGrow(queueLen, maxThresh);
    }

    void QueueBase::ShutdownReader() {
        AutoLock<QueueBase> al(*this);
        UnlockedShutdownReader();
    }

    void QueueBase::UnlockedShutdownReader() {
        readshutdown = true;
        Signal();
    }

    void QueueBase::ShutdownWriter() {
        AutoLock<QueueBase> al(*this);
        UnlockedShutdownWriter();
    }

    void QueueBase::UnlockedShutdownWriter() {
        writeshutdown = true;
        Signal();
    }

    unsigned QueueBase::NumEnqueued() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedNumEnqueued();
    }

    unsigned QueueBase::NumDequeued() const {
        AutoLock<const QueueBase> al(*this);
        return UnlockedNumDequeued();
    }

    void QueueBase::WaitForData() {
        if (useD4R) {
            ReadBlock();
        } else {
            while (ReadBlocked()) {
                cond.Wait(lock);
            }
        }
    }

    bool QueueBase::ReadBlocked() {
        kernel->CheckTerminated();
        return UnlockedCount() < readrequest && !(readshutdown || writeshutdown);
    }

    void QueueBase::NotifyData() {
        if (UnlockedCount() >= readrequest) {
            cond.Broadcast();
        }
    }

    void QueueBase::WaitForFreespace() {
        if (useD4R) {
            WriteBlock(UnlockedQueueLength());
        } else {
            while (WriteBlocked()) {
                cond.Wait(lock);
            }
        }
    }

    bool QueueBase::WriteBlocked() {
        kernel->CheckTerminated();
        return UnlockedFreespace() < writerequest && !(readshutdown || writeshutdown);
    }

    void QueueBase::NotifyFreespace() {
        if (UnlockedFreespace() >= writerequest) {
            cond.Broadcast();
        }
    }

    void QueueBase::NotifyTerminate() {
        AutoLock<QueueBase> al(*this);
        cond.Broadcast();
    }

    void QueueBase::Detect() {
        unsigned size = kernel->CalculateGrowSize(UnlockedCount(), writerequest);
        logger.Debug("Detect: Grow(%u, %u)", size, writerequest);
        UnlockedGrow(size, writerequest);
        logger.Debug("New size: (%u, %u)", UnlockedQueueLength(), UnlockedMaxThreshold());
    }

    unsigned QueueBase::ReadRequest() {
        AutoLock<QueueBase> al(*this);
        return readrequest;
    }

    unsigned QueueBase::WriteRequest() {
        AutoLock<QueueBase> al(*this);
        return writerequest;
    }

    bool QueueBase::IsReaderShutdown() {
        AutoLock<QueueBase> al(*this);
        return readshutdown;
    }

    bool QueueBase::IsWriterShutdown() {
        AutoLock<QueueBase> al(*this);
        return writeshutdown;
    }

    void QueueBase::LogState() {
        logger.Error("Printing state (w:%llu r:%llu)", readerkey, writerkey);
        logger.Error("size: %u, maxthresh: %u count: %u free: %u",
                UnlockedQueueLength(), UnlockedMaxThreshold(), UnlockedCount(), UnlockedFreespace());
        logger.Error("readrequest: %u, writerequest: %u numenqueued: %u, numdequeued: %u",
                readrequest, writerequest, UnlockedNumEnqueued(), UnlockedNumDequeued());
        if (indequeue) {
            logger.Error("Indequeue (thresh: %u)", dequeuethresh);
        }
        if (inenqueue) {
            logger.Error("Inenqueue (thresh: %u)", enqueuethresh);
        }
        if (readshutdown) {
            logger.Error("Reader shutdown");
        }
        if (writeshutdown) {
            logger.Error("Writer shutdown");
        }
    }

    QueueReleaser::~QueueReleaser() {}
}


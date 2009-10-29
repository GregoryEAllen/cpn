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

#include "SocketEndpoint.h"
#include "Assert.h"
#include "ToString.h"

namespace CPN {

    SocketEndpoint::SocketEndpoint(Key_t readerkey_, Key_t writerkey_, Mode_t mode_,
            KernelMessageHandler *kmh_, unsigned size, unsigned maxThresh, unsigned numChans)
        : logger(kmh_->GetLogger(), Logger::INFO),
        queue(size, maxThresh, numChans),
        status(INIT),
        mode(mode_),
        writerkey(writerkey_),
        readerkey(readerkey_),
        kmh(kmh_)
    {
        logger.Name(ToString("SocketEndpoint(m:%s, r:%lu, w: %lu)",
                    mode == READ ? "r" : "w", readerkey, writerkey));
    }

    SocketEndpoint::Status_t SocketEndpoint::GetStatus() const {
        Sync::AutoReentrantLock arl(lock);
        return status;
    }

    void SocketEndpoint::Shutdown() {
    }

    double SocketEndpoint::CheckStatus() {
        return -1;
    }

    const void* SocketEndpoint::GetRawDequeuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock arl(lock);
        return queue.GetRawDequeuePtr(thresh, chan);
    }

    void SocketEndpoint::Dequeue(unsigned count) {
        Sync::AutoReentrantLock arl(lock);
        return queue.Dequeue(count);
    }

    bool SocketEndpoint::RawDequeue(void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock arl(lock);
        return queue.RawDequeue(data, count, numChans, chanStride);
    }

    bool SocketEndpoint::RawDequeue(void* data, unsigned count) {
        Sync::AutoReentrantLock arl(lock);
        return queue.RawDequeue(data, count);
    }

    void* SocketEndpoint::GetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        Sync::AutoReentrantLock arl(lock);
        return queue.GetRawEnqueuePtr(thresh, chan);
    }

    void SocketEndpoint::Enqueue(unsigned count) {
        Sync::AutoReentrantLock arl(lock);
        return queue.Enqueue(count);
    }

    bool SocketEndpoint::RawEnqueue(const void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock arl(lock);
        return queue.RawEnqueue(data, count, numChans, chanStride);
    }

    bool SocketEndpoint::RawEnqueue(const void* data, unsigned count) {
        Sync::AutoReentrantLock arl(lock);
        return queue.RawEnqueue(data, count);
    }

    unsigned SocketEndpoint::NumChannels() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.NumChannels();
    }

    unsigned SocketEndpoint::Count() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.Count();
    }

    bool SocketEndpoint::Empty() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.Empty();
    }

    unsigned SocketEndpoint::Freespace() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.Freespace();
    }

    bool SocketEndpoint::Full() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.Full();
    }

    unsigned SocketEndpoint::MaxThreshold() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.MaxThreshold();
    }

    unsigned SocketEndpoint::QueueLength() const {
        Sync::AutoReentrantLock arl(lock);
        return queue.QueueLength();
    }

    void SocketEndpoint::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoReentrantLock arl(lock);
        return queue.Grow(queueLen, maxThresh);
    }



    void SocketEndpoint::RMHEnqueue(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::RMHEndOfWriteQueue(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::RMHWriteBlock(Key_t writerkey, Key_t readerkey, unsigned requested) {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::RMHTagChange(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
    }


    void SocketEndpoint::WMHDequeue(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::WMHEndOfReadQueue(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::WMHReadBlock(Key_t readerkey, Key_t writerkey, unsigned requested) {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::WMHTagChange(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
    }


    void SocketEndpoint::OnRead() {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::OnWrite() {
        Sync::AutoReentrantLock arl(lock);
    }

    void SocketEndpoint::OnError() {
        Sync::AutoReentrantLock arl(lock);
        // Error on socket.
    }

    void SocketEndpoint::OnHup() {
        Sync::AutoReentrantLock arl(lock);
        // If I understand correctly this will be called if
        // poll detects that if we try to write we would get EPIPE
    }

    void SocketEndpoint::OnInval() {
        Sync::AutoReentrantLock arl(lock);
        // Our file descriptor is invalid
    }

    void SocketEndpoint::WriteBytes(const iovec *iov, unsigned iovcnt) {
        int numwritten = Writev(iov, iovcnt);
#ifndef NDEBUG
        int total = 0;
        for (unsigned i = 0; i < iovcnt; ++i) {
            total += iov[i].iov_cnt;
        }
        ASSERT(numwritten == total, "Writev did not completely write data.");
#endif
    }
}


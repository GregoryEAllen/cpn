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

    SocketEndpoint(Key_t readerkey_, Key_t writerkey_, Mode_t mode_,
            KernelMessageHandler *kmh_)
        : logger(kmh_->GetLogger(), Logger::INFO),
        status(INIT),
        mode(mode_), readerkey(readerkey_), writerkey(writerkey_), kmh(kmh_)
    {
        logger.Name(ToString("SocketEndpoint(m:%s, r:%lu, w: %lu)",
                    mode == READ ? "r" : "w", readerkey, writerkey));
    }

    SocketEndpoint::Status_t SocketEndpoint::GetStatus() const {
        Sync::AutoReentrantLock arl(lock);
        return status;
    }

    const void* GetRawDequeuePtr(unsigned thresh, unsigned chan=0) {
        Sync::AutoReentrantLock arl(lock);
    }

    void Dequeue(unsigned count) {
        Sync::AutoReentrantLock arl(lock);
    }

    bool RawDequeue(void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock arl(lock);
    }

    bool RawDequeue(void* data, unsigned count) {
        Sync::AutoReentrantLock arl(lock);
    }

    void* GetRawEnqueuePtr(unsigned thresh, unsigned chan=0) {
        Sync::AutoReentrantLock arl(lock);
    }

    void Enqueue(unsigned count) {
        Sync::AutoReentrantLock arl(lock);
    }

    bool RawEnqueue(const void* data, unsigned count,
            unsigned numChans, unsigned chanStride) {
        Sync::AutoReentrantLock arl(lock);
    }

    bool RawEnqueue(const void* data, unsigned count) {
        Sync::AutoReentrantLock arl(lock);
    }

    unsigned NumChannels() const {
        Sync::AutoReentrantLock arl(lock);
    }

    unsigned Count() const {
        Sync::AutoReentrantLock arl(lock);
    }

    bool Empty() const {
        Sync::AutoReentrantLock arl(lock);
    }

    unsigned Freespace() const {
        Sync::AutoReentrantLock arl(lock);
    }

    bool Full() const {
        Sync::AutoReentrantLock arl(lock);
    }

    unsigned MaxThreshold() const {
        Sync::AutoReentrantLock arl(lock);
    }

    unsigned QueueLength() const {
        Sync::AutoReentrantLock arl(lock);
    }

    void Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoReentrantLock arl(lock);
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

}


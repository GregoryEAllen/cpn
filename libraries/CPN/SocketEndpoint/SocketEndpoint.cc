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
        Readable(true);
        Writeable(false);
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
        ASSERT(mode == WRITE);
        InternCheckState();
    }

    void SocketEndpoint::RMHEndOfWriteQueue(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == WRITE);
        writeshutdown = true;
        InternCheckStatus();
    }

    void SocketEndpoint::RMHWriteBlock(Key_t writerkey, Key_t readerkey, unsigned requested) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == WRITE);
        blockRequest = requested;
        pendingBlock = true;
        InternCheckStatus();
    }

    void SocketEndpoint::RMHTagChange(Key_t writerkey, Key_t readerkey) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == WRITE);
        ASSERT(false, "unimplemented");
    }


    void SocketEndpoint::WMHDequeue(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == READ);
        pendingDequeue = true;
        InternCheckStatus();
    }

    void SocketEndpoint::WMHEndOfReadQueue(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == READ);
        readshutdown = true;
        InternCheckStatus();
    }

    void SocketEndpoint::WMHReadBlock(Key_t readerkey, Key_t writerkey, unsigned requested) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == READ);
        blockRequest = requested;
        pendingBlock = true;
        InternCheckStatus();
    }

    void SocketEndpoint::WMHTagChange(Key_t readerkey, Key_t writerkey) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(mode == READ);
        ASSERT(false, "unimplemented");
    }


    void SocketEndpoint::OnRead() {
        Sync::AutoReentrantLock arl(lock);
        if (!Good()) { return; }
        bool loop = true;
        while (loop) {
            unsigned numtoread = 0;
            void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
            unsigned numread = Recv(ptr, numtoread, false);
            if (numread == 0) {
                if (!Good()) {
                    // Eof
                }
                loop = false;
            } else {
                PacketDecoder::ReleaseDecoderBytes(numread);
            }
        }
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

    void SocketEndpoint::EnqueuePacket(const Packet &packet) {
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        readcount += packet.Count();
        std::vector<iovec> iovs;
        for (unsigned i = 0; i < packet.NumChannels(); ++i) {
            iovec iov;
            iov.iov_base = queue.GetRawEnqueuePtr(packet.Count(), i);
            ASSERT(iov.iov_base, "Internal throttle failed!");
            iov.iov_len = packet.Count();
        }
        unsigned numread = Readv(&iovs[0], iovs.size());
        // Need to figure out what to do when this fails.
        ASSERT(numread == packet.DataLength());
        queue.Enqueue(packet.Count());
        ReaderMessageHandler::RMHEnqueue(writerkey, readerkey);
        InternCheckStatus();
    }

    void SocketEndpoint::DequeuePacket(const Packet &packet) {
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        writecount -= packet.Count();
        WriterMessageHandler::WMHDequeue(readerkey, writerkey);
        InternCheckStatus();
    }

    void SocketEndpoint::ReadBlockPacket(const Packet &packet) {
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        WriterMessageHandler::WMHReadBlock(readerkey, writerkey, packet.Requested());
        InternCheckStatus();
    }

    void SocketEndpoint::WriteBlockPacket(const Packet &packet) {
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        ReaderMessageHandler::RMHWriteBlock(writerkey, readerkey, packet.Requested());
        InternCheckStatus();
    }

    void SocketEndpoint::EndOfWritePacket(const Packet &packet) {
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        writeshutdown = true;
        // This signals proper end of data, there will be no more
        // data coming across this socket, next read should return 0
        ReaderMessageHandler::RMHEndOfWriteQueue(writerkey, readerkey);
        InternCheckStatus();
    }

    void SocketEndpoint::EndOfReadPacket(const Packet &packet) {
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        readshutdown = true;
        WriterMessageHandler::WMHEndOfReadQueue(readerkey, writerkey);
        InternCheckStatus();
    }

    void SocketEndpoint::IDReaderPacket(const Packet &packet) {
        ASSERT(false, "Shouldn't receive an ID packet once the connection is successful");
    }

    void SocketEndpoint::IDWriterPacket(const Packet &packet) {
        ASSERT(false, "Shouldn't receive an ID packet once the connection is successful");
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

    void SocketEndpoint::SendWakeup() {
        kmh->SendWakeup();
    }

    void SocketEndpoint::InternCheckStatus() {
        if (Closed()) {
            if (status != DEAD) {
                while (true) {
                    if (connection) {
                        if (connection->Done()) {
                            FileHandler::FD(connection->Get());
                            connection.reset();
                        } else { return; }
                    } else if (Closed()) {
                        if (mode == READ) {
                            connection = kmh->GetReaderDescriptor(readerkey, writerkey);
                        } else {
                            connection = kmh->GetWriterDescriptor(readerkey, writerkey);
                        }
                    } else { break; }
                }
            }
        }

        if (mode == READ) {
            // In read mode
 
            if (pendingDequeue) {
                SendDequeue();
            }

            if (pendingBlock) {
                SendReadBlock();
            }

            // If we have a read shutdown we need to send the 
            // end of read queue message and shutdown the write
            // side of the socket.
            if (readshutdown) {
                if (!sendEnd) { SendEndOfRead(); }
            }

            // if we have a write shutdown we can go ahead and shutdown
            // the read side of our socket and send a end of reader
            // down the socket if not already sent and start actually
            // shutting down
            if (writeshutdown) {
                if (!sendEnd) { SendEndOfRead(); }
                Close();
                status = DEAD;
            }

        } else {
            ASSERT(mode == WRITE);

            // In write mode
            if (!queue.Empty()) {
                if (!EnqueueBlocked()) {
                    SendEnqueue();
                }
            }

            if (pendingBlock) {
                OnRead();
                if (blockRequested > queue.Count()) {
                    SendWriteBlock();
                }
            }

            // if we have a read shutdown we ensure we have written
            // a writer end and die. There will be no more enqueues
            // and there is no more data to be read.
            if (readshutdown) {
                if (!sentEnd) { SendEndOfWrite(); }
                Close();
                status = DEAD;
            }

            // If we have a write shutdown we need to finish sending any data we
            // have and then send across the writer end and wait for the confirming
            // reader end
            if (writeshutdown) {
                if ( queue.Empty() && !sentEnd ) {
                    SendEndOfWrite();
                }
            }
        }
    }

    bool SocketEndpoint::EnqueueBlocked() {
        return (2 * writecount) > queue.QueueLength();
    }

    void SocketEndpoint::SetupPacketDefaults(Packet &packet) {
        if (mode == WRITE) { packet.SourceKey(writerkey).DestinationKey(readerkey); }
        else { packet.SourceKey(readerkey).DestinationKey(writerkey); }
        packet.Mode(mode).Status(status).NumChannels(queue.NumChannels());
    }

    void SocketEndpoint::SendEnqueue() {
        unsigned count = queue.Count();
        const unsigned maxthresh = queue.MaxThreshold();
        const unsigned expectedfree = queue.QueueLength() - writecount;
        if (count > maxthresh) { count = maxthresh; }
        if (count > expectedfree) { count = expectedfree; }
        unsigned datalength = count * queue.NumChannels();
        Packet packet(datalength, PACKET_ENQUEUE);
        SetupPacketDefaults(packet);
        packet.Count(count).BytesQueued(queue.Count());
        writecount += count;
        PacketEncoder::SendEnqueue(packet, this);
    }

    void SocketEndpoint::SendWriteBlock() {
        Packet packet(PACKET_WRITEBLOCK);
        SetupPacketDefaults(packet);
        packet.Requested(blockRequest);
        PacketEncoder::SendPacket(packet);
        pendingBlock = false;
    }

    void SocketEndpoint::SendEndOfWrite() {
        Packet packet(PACKET_ENDOFWRITE);
        SetupPacketDefaults(packet);
        PacketEncoder::SendPacket(packet);
        sentEnd = true;
        SockHandler::ShutdownWrite();
    }

    void SocketEndpoint::SendDequeue() {
        Packet packet(PACKET_DEQUEUE);
        SetupPacketDefaults(packet);
        unsigned count = readcount - queue.Count();
        packet.Count(count);
        PacketEncoder::SendPacket(packet);
        readcount -= count;
        pendingDequeue = false;
    }

    void SocketEndpoint::SendReadBlock() {
        Packet packet(PACKET_READBLOCK);
        SetupPacketDefaults(packet);
        packet.Requested(blockRequest);
        PacketEncoder::SendPacket(packet);
        pendingBlock = false;
    }

    void SocketEndpoint::SendEndOfRead() {
        Packet packet(PACKET_ENDOFREAD);
        SetupPacketDefaults(packet);
        PacketEncoder::SendPacket(packet);
        sentEnd = true;
        SockHandler::ShutdownWrite();
    }
}


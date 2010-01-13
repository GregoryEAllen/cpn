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
#include "QueueAttr.h"
#include "Assert.h"
#include "ToString.h"
#include "ErrnoException.h"

#if 0
#define FUNC_TRACE logger.Trace("%s", __PRETTY_FUNCTION__)
#else
#define FUNC_TRACE
#endif

struct BoolGuard {
public:
    BoolGuard(bool &v) : val(v) { val = true; }
    ~BoolGuard() { val = false; }
private:
    bool &val;
};

namespace CPN {

    SocketEndpoint::SocketEndpoint(shared_ptr<Database> db, Mode_t mode_,
            KernelBase *kmh_, const SimpleQueueAttr &attr)
        : QueueBase(db, attr),
        logger(kmh_->GetLogger(), Logger::INFO),
        queue(attr.GetLength(), attr.GetMaxThreshold(), attr.GetNumChannels()),
        status(LIVE),
        mode(mode_),
        kmh(kmh_),
        writecount(0),
        readcount(0),
        pendingDequeue(false),
        pendingBlock(false),
        sentEnd(false),
        inread(false),
        incheckstatus(false)
    {
        Writeable(false);
        logger.Name(ToString("SocketEndpoint(m:%s, r:%llu, w: %llu)",
                    mode == READ ? "r" : "w", readerkey, writerkey));
    }

    SocketEndpoint::Status_t SocketEndpoint::GetStatus() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return status;
    }

    void SocketEndpoint::Shutdown() {
        Sync::AutoLock<QueueBase> arl(*this);
        FUNC_TRACE;
        // force us into a shutdown state
        if (connection) {
            connection->Cancel();
            connection.reset();
        }
        if (mode == READ) {
            readshutdown = true;
        } else {
            writeshutdown = true;
        }
        queue.Clear();
        status = DIEING;
        InternCheckStatus();
    }

    double SocketEndpoint::CheckStatus() {
        Sync::AutoLock<QueueBase> arl(*this);
        InternCheckStatus();

        // this is arbitrary
        if (status == LIVE) {
            if (Closed()) {
                // 30s
                return 30;
            }
        } else if (status == DIEING) {
            return 30;
        }
        return -1;
    }

    const void *SocketEndpoint::InternalGetRawDequeuePtr(unsigned thresh, unsigned chan) {
        return queue.GetRawDequeuePtr(thresh, chan);
    }

    void SocketEndpoint::InternalDequeue(unsigned count) {
        queue.Dequeue(count);
        pendingDequeue = true;
        InternCheckStatus();
    }

    void *SocketEndpoint::InternalGetRawEnqueuePtr(unsigned thresh, unsigned chan) {
        return queue.GetRawEnqueuePtr(thresh, chan);
    }

    void SocketEndpoint::InternalEnqueue(unsigned count) {
        queue.Enqueue(count);
        InternCheckStatus();
    }

    unsigned SocketEndpoint::NumChannels() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.NumChannels();
    }

    unsigned SocketEndpoint::Count() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.Count();
    }

    bool SocketEndpoint::Empty() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.Empty();
    }

    unsigned SocketEndpoint::Freespace() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.Freespace();
    }

    bool SocketEndpoint::Full() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.Full();
    }

    unsigned SocketEndpoint::MaxThreshold() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.MaxThreshold();
    }

    unsigned SocketEndpoint::QueueLength() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.QueueLength();
    }

    void SocketEndpoint::Grow(unsigned queueLen, unsigned maxThresh) {
        Sync::AutoLock<QueueBase> arl(*this);
        return queue.Grow(queueLen, maxThresh);
    }


    void SocketEndpoint::WaitForFreespace(unsigned request) {
        Sync::AutoLock<QueueBase> arl(*this);
        ASSERT(mode == WRITE);
        pendingBlock = true;
        InternCheckStatus();
        QueueBase::WaitForFreespace(request);
    }

    void SocketEndpoint::WaitForData(unsigned request) {
        Sync::AutoLock<QueueBase> arl(*this);
        ASSERT(mode == READ);
        pendingBlock = true;
        InternCheckStatus();
        QueueBase::WaitForData(request);
    }

    void SocketEndpoint::ShutdownReader() {
        Sync::AutoLock<QueueBase> al(*this);
        QueueBase::ShutdownReader();
        InternCheckStatus();
    }

    void SocketEndpoint::ShutdownWriter() {
        Sync::AutoLock<QueueBase> al(*this);
        QueueBase::ShutdownWriter();
        InternCheckStatus();
    }

    void SocketEndpoint::OnRead() {
        Sync::AutoLock<QueueBase> arl(*this);
        // Don't allow recursive calls to OnRead
        // Can happen because *Packet functions call
        // InternCheckStatus
        if (inread) { return; }
        BoolGuard guard(inread);

        try {
            while (Good()) {
                unsigned numtoread = 0;
                void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
                unsigned numread = Recv(ptr, numtoread, false);
                if (numread == 0) {
                    if (Eof()) {
                        // Eof
                        logger.Debug("Read EOF");
                        //ASSERT(readshutdown || writeshutdown, "Read EOF before shutdown");
                    }
                    break;
                } else {
                    PacketDecoder::ReleaseDecoderBytes(numread);
                }
            }
        } catch (const ErrnoException &e) {
            logger.Error("Exception on read (%d): %s", e.Error(), e.what());
        }
    }

    void SocketEndpoint::OnWrite() {
        Sync::AutoLock<QueueBase> arl(*this);
        logger.Error("%s", __PRETTY_FUNCTION__);
    }

    void SocketEndpoint::OnError() {
        Sync::AutoLock<QueueBase> arl(*this);
        // Error on socket.
        logger.Error("%s", __PRETTY_FUNCTION__);
    }

    void SocketEndpoint::OnHup() {
        Sync::AutoLock<QueueBase> arl(*this);
        // If I understand correctly this will be called if
        // poll detects that if we try to write we would get EPIPE
        logger.Debug("Hangup received");
    }

    void SocketEndpoint::OnInval() {
        Sync::AutoLock<QueueBase> arl(*this);
        // Our file descriptor is invalid
        logger.Debug("%s", __PRETTY_FUNCTION__);
        ASSERT(Closed());
    }

    bool SocketEndpoint::Readable() const {
        Sync::AutoLock<QueueBase> arl(*this);
        return Good();
    }

    void SocketEndpoint::EnqueuePacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        std::vector<iovec> iovs;
        for (unsigned i = 0; i < packet.NumChannels(); ++i) {
            iovec iov;
            iov.iov_base = queue.GetRawEnqueuePtr(packet.Count(), i);
            ASSERT(iov.iov_base, "Internal throttle failed!");
            iov.iov_len = packet.Count();
            iovs.push_back(iov);
        }
        unsigned numread = Readv(&iovs[0], iovs.size());
        // Need to figure out what to do when this fails.
        ASSERT(numread == packet.DataLength());
        queue.Enqueue(packet.Count());
        readcount += packet.Count();
        NotifyData();
    }

    void SocketEndpoint::DequeuePacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        writecount -= packet.Count();
        InternCheckStatus();
    }

    void SocketEndpoint::ReadBlockPacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        InternCheckStatus();
    }

    void SocketEndpoint::WriteBlockPacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        InternCheckStatus();
    }

    void SocketEndpoint::EndOfWritePacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT( (packet.Mode() == WRITE) &&
        (packet.SourceKey() == writerkey) &&
        (packet.DestinationKey() == readerkey)
        );
        ASSERT(mode == READ);
        ASSERT(!writeshutdown);
        QueueBase::ShutdownWriter();
        InternCheckStatus();
    }

    void SocketEndpoint::EndOfReadPacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT( (packet.Mode() == READ) &&
        (packet.SourceKey() == readerkey) &&
        (packet.DestinationKey() == writerkey)
        );
        ASSERT(mode == WRITE);
        ASSERT(!readshutdown);
        QueueBase::ShutdownReader();
        InternCheckStatus();
    }

    void SocketEndpoint::IDReaderPacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT(false, "Shouldn't receive an ID packet once the connection is successful");
    }

    void SocketEndpoint::IDWriterPacket(const Packet &packet) {
        FUNC_TRACE;
        ASSERT(false, "Shouldn't receive an ID packet once the connection is successful");
    }

    void SocketEndpoint::WriteBytes(const iovec *iov, unsigned iovcnt) {
        int numwritten = Writev(iov, iovcnt);
#ifndef NDEBUG
        int total = 0;
        for (unsigned i = 0; i < iovcnt; ++i) {
            total += iov[i].iov_len;
        }
        ASSERT(numwritten == total, "Writev did not completely write data.");
#endif
    }

    void SocketEndpoint::InternCheckStatus() {
        // return if recursive call
        if (incheckstatus) { return; }
        BoolGuard guard(incheckstatus);
        if (status == DEAD) { return; }

        try {

            if (Eof() && !(readshutdown || writeshutdown)) {
                logger.Error("Eof detected but not shutdown!");
            }

            if (Closed()) {
                if (status == DIEING) {
                    status = DEAD;
                    return;
                }
                if (status == LIVE) {
                    while (true) {
                        if (connection) {
                            if (connection->Done()) {
                                ASSERT(Closed());
                                FileHandler::Reset();
                                FileHandler::FD(connection->Get());
                                connection.reset();
                                logger.Debug("Connection established");
                            } else { return; }
                        } else if (Closed()) {
                            logger.Debug("Getting new connection");
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
                    OnRead();
                    if (readrequest > queue.Count()) {
                        SendReadBlock();
                    } else {
                        pendingBlock = false;
                    }
                }

                // If we have a read shutdown we need to send the 
                // end of read queue message and shutdown the write
                // side of the socket.
                if (readshutdown) {
                    if (!sentEnd) { SendEndOfRead(); }
                }

                // if we have a write shutdown we can go ahead and shutdown
                // the write side of our socket and send a end of reader
                // down the socket if not already sent and start actually
                // shutting down
                if (writeshutdown) {
                    if (!sentEnd) { SendEndOfRead(); }
                    status = DEAD;
                    logger.Debug("Closing connection");
                    Close();
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
                    if (writerequest > queue.Freespace()) {
                        SendWriteBlock();
                    } else {
                        pendingBlock = false;
                    }
                }

                // if we have a read shutdown we ensure we have written
                // a writer end and die. There will be no more enqueues
                // and there is no more data to be read.
                if (readshutdown) {
                    if (!sentEnd) { SendEndOfWrite(); }
                    status = DEAD;
                    logger.Debug("Closing connection");
                    Close();
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
        } catch (const ErrnoException &e) {
            logger.Error("Exception (%d): %s", e.Error(), e.what());
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
        FUNC_TRACE;
        ASSERT(!Closed());
        unsigned count = queue.Count();
        const unsigned maxthresh = queue.MaxThreshold();
        const unsigned expectedfree = queue.QueueLength() - writecount;
        if (count > maxthresh) { count = maxthresh; }
        if (count > expectedfree) { count = expectedfree; }
        unsigned datalength = count * queue.NumChannels();
        Packet packet(datalength, PACKET_ENQUEUE);
        SetupPacketDefaults(packet);
        packet.Count(count).BytesQueued(queue.Count());
        PacketEncoder::SendEnqueue(packet, queue);
        writecount += count;
        QueueBase::NotifyData();
    }

    void SocketEndpoint::SendWriteBlock() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_WRITEBLOCK);
        SetupPacketDefaults(packet);
        packet.Requested(writerequest);
        PacketEncoder::SendPacket(packet);
        pendingBlock = false;
    }

    void SocketEndpoint::SendEndOfWrite() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_ENDOFWRITE);
        SetupPacketDefaults(packet);
        PacketEncoder::SendPacket(packet);
        sentEnd = true;
        SockHandler::ShutdownWrite();
    }

    void SocketEndpoint::SendDequeue() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_DEQUEUE);
        SetupPacketDefaults(packet);
        unsigned count = readcount - queue.Count();
        packet.Count(count);
        PacketEncoder::SendPacket(packet);
        readcount -= count;
        pendingDequeue = false;
    }

    void SocketEndpoint::SendReadBlock() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_READBLOCK);
        SetupPacketDefaults(packet);
        packet.Requested(readrequest);
        PacketEncoder::SendPacket(packet);
        pendingBlock = false;
    }

    void SocketEndpoint::SendEndOfRead() {
        FUNC_TRACE;
        ASSERT(!Closed());
        Packet packet(PACKET_ENDOFREAD);
        SetupPacketDefaults(packet);
        PacketEncoder::SendPacket(packet);
        sentEnd = true;
        SockHandler::ShutdownWrite();
    }

    void SocketEndpoint::LogState() {
        logger.Debug("Printing state (w:%llu r:%llu)", readerkey, writerkey);
        switch (status) {
            case LIVE:
                logger.Debug("State live");
                break;
            case DEAD:
                logger.Debug("State dead");
                break;
            case DIEING:
                logger.Debug("State dieing");
                break;
            default:
                ASSERT(false);
        }
        if (Closed()) {
            logger.Debug("Not Connected");
            if (connection) {
                logger.Debug("Pending connection (%p)", connection.get());
                if (connection->Done()) {
                    logger.Debug("Pending done");
                }
            }
        } else {
            logger.Debug("Connected");
            if (Eof()) {
                logger.Debug("Read eof");
            }
        }
        if (mode == READ) {
            logger.Debug("Readcount %u (in queue %u)", readcount, queue.Count());
            if (pendingDequeue) {
                logger.Debug("Dequeue pending");
            }
            if (sentEnd) {
                logger.Debug("End of read sent.");
            }
        } else {
            logger.Debug("Writecount %u", writecount);
            if (!queue.Empty()) {
                logger.Debug("Enqueue pending (count %u)", queue.Count());
                if (EnqueueBlocked()) {
                    logger.Debug("Enqueue blocked");
                }
            }
            if (sentEnd) {
                logger.Debug("End of write sent.");
            }
        }
        if (pendingBlock) {
            if (mode == READ) {
                logger.Debug("Pending block request %u", readrequest);
            } else {
                logger.Debug("Pending block request %u", writerequest);
            }
        }
        if (readshutdown) {
            logger.Debug("Reader shutdown");
        }
        if (writeshutdown) {
            logger.Debug("Writer shutdown");
        }
    }
}


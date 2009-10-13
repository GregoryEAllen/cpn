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

#include "StreamEndpoint.h"
#include "QueueBase.h"

#include "Assert.h"

namespace CPN {

    StreamEndpoint::StreamEndpoint(KernelMessageHandler *kernMsgHan, Key_t rkey, Key_t wkey, Mode_t m)
        : queuelock(0), writecount(0),
        readcount(0),
        rmh(0), wmh(0), kmh(kernMsgHan),
        readerkey(rkey),
        writerkey(wkey),
        mode(m),
        shuttingdown(false)
    {
        PacketDecoder::Enable(false);
    }

    void StreamEndpoint::RMHEnqueue(Key_t wkey, Key_t rkey) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(!shuttingdown, "Enqueue on a shutdown queue!");
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        CheckBlockedEnqueues();
        WriteSome();
        Wakeup();
    }


    void StreamEndpoint::WMHDequeue(Key_t rkey, Key_t wkey) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        unsigned numchans = queue->NumChannels();
        // Difference between how many bytes we think are in the
        // queue and how many are actually there
        unsigned length = readcount - queue->Count();
        encoder.SendDequeue(length, numchans);
        readcount -= length;
        WriteSome();
        Wakeup();
    }

    void StreamEndpoint::WMHReadBlock(Key_t rkey, Key_t wkey, unsigned requested) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(!shuttingdown, "Block on a shutdown queue!");
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        encoder.SendReadBlock(requested);
        WriteSome();
        Wakeup();
    }

    void StreamEndpoint::RMHWriteBlock(Key_t wkey, Key_t rkey, unsigned requested) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(!shuttingdown, "Block on a shutdown queue!");
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        CheckBlockedEnqueues();
        encoder.SendWriteBlock(requested);
        WriteSome();
        Wakeup();
    }

    void StreamEndpoint::RMHEndOfWriteQueue(Key_t wkey, Key_t rkey) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        // send the message on and start our shutdown
        shuttingdown = true;
        encoder.SendEndOfWriteQueue();
        WriteSome();
        Wakeup();
    }

    void StreamEndpoint::WMHEndOfReadQueue(Key_t rkey, Key_t wkey) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        // send the message on and start our shutdown
        shuttingdown = true;
        encoder.SendEndOfReadQueue();
        WriteSome();
        Wakeup();
    }

    void StreamEndpoint::RMHTagChange(Key_t wkey, Key_t rkey) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(wkey == writerkey && rkey == readerkey);
        ASSERT(mode == WRITE);
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::WMHTagChange(Key_t rkey, Key_t wkey) {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        ASSERT(rkey == readerkey && wkey == writerkey);
        ASSERT(mode == READ);
        ASSERT(false, "Unimplemented");
    }

    bool StreamEndpoint::ReadReady() {
        Sync::AutoReentrantLock arl(lock);
        return PacketDecoder::Enabled();
    }

    bool StreamEndpoint::WriteReady() {
        Sync::AutoReentrantLock arl(lock);
        if (!encoder.BytesReady()) {
            if (mode == WRITE && queue) {
                arl.Unlock();
                Sync::AutoReentrantLock qarl(*queuelock);
                arl.Lock();
                if (!EnqueueBlocked()) {
                    return WriteEnqueue();
                }
            }
            return false;
        }
        return true;
    }

    void StreamEndpoint::ReadSome() {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        if (!descriptor) { return; }
        Async::Stream stream(descriptor);
        unsigned numtoread = 0;
        unsigned numread = 0;
        bool loop = true;
        while (loop) {
            void *ptr = PacketDecoder::GetDecoderBytes(numtoread);
            numread = stream.Read(ptr, numtoread);
            if (0 == numread) {
                if  (!stream) {
                    // The other end closed the connection!!
                    stream.Close();
                    descriptor.reset();
                }
                loop = false;
            } else {
                PacketDecoder::ReleaseDecoderBytes(numread);
            }
        }
    }

    void StreamEndpoint::WriteSome() {
        Sync::AutoReentrantLock qarl(*queuelock);
        Sync::AutoReentrantLock arl(lock);
        if (!descriptor) { return; }
        Async::Stream stream(descriptor);
        while (true) {
            unsigned towrite = 0;
            const void *ptr = encoder.GetEncodedBytes(towrite);
            if (0 == towrite) {
                if (mode == WRITE) {
                    if (!EnqueueBlocked()) {
                        if (WriteEnqueue()) {
                            continue;
                        } else if (queue->Empty()
                                && !encoder.BytesReady()
                                && shuttingdown) {
                            // Nothing left to write
                            stream.Close();
                            descriptor.reset();
                        }
                    }
                }
                break;
            }
            unsigned numwritten = stream.Write(ptr, towrite);
            if (numwritten == 0) {
                break;
            } else {
                encoder.ReleaseEncodedBytes(numwritten);
            }
        }
    }

    void StreamEndpoint::OnError(int err) {
        descriptor.reset();
        if (Shuttingdown()) {
            SignalDeath();
        }
    }

    // All of these Receive functions will end up beign called in the
    // context of ReadSome.

    void StreamEndpoint::ReceivedEnqueue(void *data, unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        ASSERT(mode == READ);
        // We want to send the enqueue message after we have actually enqueued the data
        ENSURE(queue->RawEnqueue(data, length, numchannels, length),
                "Enqueue throttling over the socket failed.");
        readcount += length;
        rmh->RMHEnqueue(writerkey, readerkey);
    }

    void StreamEndpoint::ReceivedDequeue(unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        ASSERT(mode == WRITE);
        writecount -= length;
        CheckBlockedEnqueues();
        wmh->WMHDequeue(readerkey, writerkey);
    }

    void StreamEndpoint::ReceivedReadBlock(unsigned requested) {
        ASSERT(mode == WRITE);
        wmh->WMHReadBlock(readerkey, writerkey, requested);
    }

    void StreamEndpoint::ReceivedWriteBlock(unsigned requested) {
        ASSERT(mode == READ);
        rmh->RMHWriteBlock(writerkey, readerkey, requested);
    }

    void StreamEndpoint::ReceiveEndOfWriteQueue() {
        ASSERT(mode == READ);
        shuttingdown = true;
        rmh->RMHEndOfWriteQueue(writerkey, readerkey);
    }

    void StreamEndpoint::ReceiveEndOfReadQueue() {
        ASSERT(mode == WRITE);
        shuttingdown = true;
        wmh->WMHEndOfReadQueue(readerkey, writerkey);
    }

    void StreamEndpoint::ReceivedReaderID(uint64_t rkey, uint64_t wkey) {
        ASSERT(mode == WRITE);
        ASSERT(readerkey == rkey);
        ASSERT(writerkey == wkey);
    }

    void StreamEndpoint::ReceivedWriterID(uint64_t wkey, uint64_t rkey) {
        ASSERT(mode == READ);
        ASSERT(readerkey == rkey);
        ASSERT(writerkey == wkey);
    }

    void StreamEndpoint::CheckBlockedEnqueues() {
        while (!EnqueueBlocked() && WriteEnqueue()) {
            // Would it be a good idea to call WriteSome here?
        }
    }

    bool StreamEndpoint::WriteEnqueue() {
        ASSERT(mode == WRITE);
        // Send as much as we can, but no more than maxthreshold
        unsigned maxthresh = queue->MaxThreshold();
        unsigned amount = queue->Count();
        if (amount > maxthresh) { amount = maxthresh; }
        if (amount == 0) { return false; }
        unsigned numchans = queue->NumChannels();
        std::vector<const void*> data(numchans, 0);
        for (unsigned chan = 0; chan < numchans; ++chan) {
            data[chan] = queue->GetRawDequeuePtr(amount, chan);
            ASSERT(data[chan], "No data in the queue, inconsistant queue state.");
        }
        encoder.SendEnqueue(&data[0], amount, numchans);
        queue->Dequeue(amount);
        writecount += amount;
        return true;
    }

    bool StreamEndpoint::EnqueueBlocked() {
        return 2*writecount >= queue->QueueLength();
    }

    void StreamEndpoint::SetDescriptor(Async::DescriptorPtr desc) {
        Sync::AutoReentrantLock arl(lock);
        ASSERT(!descriptor);
        descriptor = desc;
        descriptor->ConnectReadable(sigc::mem_fun(this, &StreamEndpoint::ReadReady));
        descriptor->ConnectWriteable(sigc::mem_fun(this, &StreamEndpoint::WriteReady));
        descriptor->ConnectOnRead(sigc::mem_fun(this, &StreamEndpoint::ReadSome));
        descriptor->ConnectOnWrite(sigc::mem_fun(this, &StreamEndpoint::WriteSome));
        descriptor->ConnectOnError(sigc::mem_fun(this, &StreamEndpoint::OnError));
    }

    void StreamEndpoint::ResetDescriptor() {
        Sync::AutoReentrantLock arl(lock);
        descriptor.reset();
    }

    void StreamEndpoint::SetQueue(shared_ptr<QueueBase> q) {
        Sync::AutoReentrantLock qarl(q->GetLock());
        Sync::AutoReentrantLock arl(lock);
        queuelock = &q->GetLock();
        queue = q;
        rmh = queue->GetReaderMessageHandler();
        wmh = queue->GetWriterMessageHandler();
        if (mode == READ) {
            q->SetWriterMessageHandler(this);
        } else if (mode == WRITE) {
            q->SetReaderMessageHandler(this);
        } else {
            ASSERT(false, "Invaid stream endpoint mode.");
        }
        PacketDecoder::Enable(true);
        Wakeup();
    }

    bool StreamEndpoint::Shuttingdown() const {
        Sync::AutoReentrantLock arl(lock);
        return shuttingdown;
    }

    void StreamEndpoint::SignalDeath() {
        if (mode == READ) {
            kmh->StreamDead(readerkey);
        } else if (mode == WRITE) {
            kmh->StreamDead(writerkey);
        } else {
            ASSERT(false, "Invalid stream endpoint mode.");
        }
    }

    void StreamEndpoint::Wakeup() {
        if (encoder.BytesReady()) {
            kmh->SendWakeup();
        }
    }

    void StreamEndpoint::RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors) {
        if (descriptor) {
            descriptors.push_back(descriptor);
        } else if (Shuttingdown()) {
            SignalDeath();
        } else if (mode == WRITE && pendingconn.expired()) {
            // Writer will be setup to create a new connection.
            pendingconn = kmh->CreateNewQueueStream(readerkey, writerkey);
        }
    }

    Key_t StreamEndpoint::GetKey() const {
        if (mode == READ) {
            return readerkey;
        } else if (mode == WRITE) {
            return writerkey;
        } else {
            ASSERT(false, "Invalid stream endpoint mode.");
        }
    }

    void StreamEndpoint::PrintState() {
        printf("Printing state for StreamEndpoint w: %lu r: %lu in mode %s\n",
                readerkey, writerkey, mode == READ ? "read" : "write");
        printf("Readcount %u, Writecount %u\n", readcount, writecount);
        if (shuttingdown) {
            printf("Currently shutting down\n");
        }
        if (encoder.BytesReady()) {
            printf("%u Bytes in encoder\n", encoder.NumBytes());
        } else {
            printf("Encoder is empty\n");
        }
        if (descriptor) {
            printf("Descriptor set and %s\n", *descriptor ? "open" : "closed");
        } else {
            printf("Descriptor not set\n");
        }
        if (queue) {
            printf("Queue set with %u bytes in it\n", queue->Count());
        } else {
            printf("Queue not set\n");
        }
        if (PacketDecoder::Enabled()) {
            printf("Decoder is enabled with %u bytes\n", PacketDecoder::NumBytes());
        } else {
            printf("Decoder is disabled with %u bytes\n", PacketDecoder::NumBytes());
        }
    }
}


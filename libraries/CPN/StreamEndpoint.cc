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

namespace CPN {

    StreamEndpoint::StreamEndpoint()
        : readstate(READ_HEADER),
        totalread(0),
        totaltoread(sizeof(readheader)),
        readchan(0),
        writecount(0)
    {
    }

    void StreamEndpoint::ProcessMessage(NodeEnqueue *msg) {
        if (!Blocked() && blockedenqueues.empty()) {
            WriteEnqueue(msg->shared_from_this());
        } else {
            blockedenqueues.push_back(msg->shared_from_this());
            CheckBlockedEnqueues();
        }
    }
    
    void StreamEndpoint::WriteEnqueue(NodeEnqueuePtr msg) {
        // Setup an enqueue packet
        unsigned amount = msg->Amount();
        unsigned numchans = queue->NumChannels();
        StreamPacketHeader spacket;
        InitStreamPacket(&spacket, amount * numchans, PACKET_ENQUEUE);
        spacket.enqueue.amount = amount;
        spacket.enqueue.numchannels = numchans;
        writecount += amount;
        writeq.Put((char*)&spacket, sizeof(StreamPacketHeader));
        for (unsigned chan = 0; chan < numchans; ++chan) {
            const char *ptr = (const char*)queue->GetRawDequeuePtr(amount, chan);
            ASSERT(ptr, "Enqueue message received but no data is in the queue.");
            writeq.Put(ptr, amount);
        }
        queue->Dequeue(amount);
    }

    void StreamEndpoint::ProcessMessage(NodeDequeue *msg) {
        // Setup a dequeue packet
        unsigned amount = msg->Amount();
        unsigned numchans = queue->NumChannels();
        StreamPacketHeader spacket;
        InitStreamPacket(&spacket, 0, PACKET_DEQUEUE);
        spacket.dequeue.amount = amount;
        spacket.dequeue.numchannels = numchans;
        writeq.Put((char*)&spacket, sizeof(StreamPacketHeader));
    }

    void StreamEndpoint::ProcessMessage(NodeReadBlock *msg) {
        // Setup a read blocked packet
        unsigned requested = msg->Requested();
        StreamPacketHeader spacket;
        InitStreamPacket(&spacket, 0, PACKET_READBLOCK);
        spacket.readblock.requested = requested;
        writeq.Put((char*)&spacket, sizeof(StreamPacketHeader));
    }

    void StreamEndpoint::ProcessMessage(NodeWriteBlock *msg) {
        // Setup a write blocked packet
        unsigned requested = msg->Requested();
        StreamPacketHeader spacket;
        InitStreamPacket(&spacket, 0, PACKET_WRITEBLOCK);
        spacket.writeblock.requested = requested;
        writeq.Put((char*)&spacket, sizeof(StreamPacketHeader));
    }

    void StreamEndpoint::ProcessMessage(NodeEndOfWriteQueue *msg) {
        // TODO send the message on and start our shutdown
    }

    void StreamEndpoint::ProcessMessage(NodeEndOfReadQueue *msg) {
        // TODO send the message on and start our shutdown
    }

    bool StreamEndpoint::ReadReady() {
        return true;
    }

    bool StreamEndpoint::WriteReady() {
        return writeq.Size() > 0;
    }

    void StreamEndpoint::ReadSome() {
        Async::Stream stream(descriptor);
        unsigned numtoread = 0;
        unsigned numread = 0;
        char *ptr = 0;
        bool loop = true;
        while (loop) {
            switch (readstate) {
            case READ_HEADER:
                ptr = (char*)&readheader;
                totaltoread = sizeof(readheader);
            break;
            case READ_DATA:
                ptr = (char*)queue->GetRawEnqueuePtr(totaltoread - totalread, readchan);
                ASSERT(ptr, "Throttle failed no room in queue.");
            break;
            default: ASSERT(false, "Unreachable");
            }
            numtoread = totaltoread - totalread;
            numread = stream.Read(ptr + totalread, numtoread);
            if (0 == numread) {
                if  (!stream) {
                    // The other end closed the connection!!
                    stream.Close();
                }
                loop = false;
            }
            totalread += numread;
            if (totalread < totaltoread) {
                //nothing read more
            } else if (totalread == totaltoread) {
                switch (readstate) {
                case READ_HEADER:
                    // TODO do error recovery rather than abort
                    ASSERT(ValidStreamPacket(&readheader), "Invalid packet!?!?");
                    // Reset state.
                    totalread = 0;
                    readchan = 0;
                    // deserialize the packet and reset/change state
                    DecomposePacketHeader();
                    break;
                case READ_DATA:
                    ++readchan;
                    if (readchan >= queue->NumChannels()) {
                        // Enqueue the data and send the enqueue message.
                        queue->Enqueue(totaltoread);
                        msgtoendpoint->Put(nextmessage);
                        nextmessage.reset();
                        readstate = READ_HEADER;
                        readchan = 0;
                        totaltoread = sizeof(readheader);
                    }
                    totalread = 0;
                    break;
                default: ASSERT(false, "Unreachable");
                }
            } else { //if (totalread > totaltoread)
                ASSERT(false, "Unreachable");
            }
        }
    }

    void StreamEndpoint::WriteSome() {
        Async::Stream stream(descriptor);
        while (true) {
            unsigned towrite = 0;
            char *ptr = writeq.AllocateGet(writeq.Size(), towrite);
            if (0 == towrite) break;
            unsigned numwritten = stream.Write(ptr, towrite);
            if (numwritten == 0) {
                break;
            } else {
                writeq.ReleaseGet(numwritten);
            }
        }
    }

    void StreamEndpoint::DecomposePacketHeader() {
        switch (readheader.base.dataType) {
        case PACKET_ENQUEUE:
            ProcessEnqueuePacket();
            break;
        case PACKET_DEQUEUE:
            ProcessDequeuePacket();
            break;
        case PACKET_READBLOCK:
            ProcessReadblockPacket();
            break;
        case PACKET_WRITEBLOCK:
            ProcessWriteblockPacket();
            break;
        default:
            // We got a packet type we don't know what to do with
            // fail messily for now
            ASSERT(false, "Unknown packet");
        }
    }

    void StreamEndpoint::ProcessEnqueuePacket() {
        readstate = READ_DATA;
        totaltoread = readheader.enqueue.amount;
        ASSERT(readheader.enqueue.numchannels == queue->NumChannels(),
                "Endpoints disagree on number many channels.");
        // We want to send the enqueue message after we have actually enqueued the data
        nextmessage = NodeEnqueue::Create(totaltoread);
    }

    void StreamEndpoint::ProcessDequeuePacket() {
        ASSERT(readheader.dequeue.numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        writecount -= readheader.dequeue.amount;
        CheckBlockedEnqueues();
        msgtoendpoint->Put(NodeDequeue::Create(readheader.dequeue.amount));
    }

    void StreamEndpoint::ProcessReadblockPacket() {
        msgtoendpoint->Put(NodeReadBlock::Create(readheader.readblock.requested));
    }

    void StreamEndpoint::ProcessWriteblockPacket() {
        msgtoendpoint->Put(NodeWriteBlock::Create(readheader.writeblock.requested));
    }

    void StreamEndpoint::CheckBlockedEnqueues() {
        while (!Blocked() && !blockedenqueues.empty()) {
            WriteEnqueue(blockedenqueues.front());
            blockedenqueues.pop_front();
        }
    }

    bool StreamEndpoint::Blocked() {
        return 2*writecount >= queue->QueueLength();
    }

    void StreamEndpoint::SetDescriptor(Async::DescriptorPtr desc) {
        descriptor = desc;
        descriptor->ConnectReadable(sigc::mem_fun(this, &StreamEndpoint::ReadReady));
        descriptor->ConnectWriteable(sigc::mem_fun(this, &StreamEndpoint::WriteReady));
        descriptor->ConnectOnRead(sigc::mem_fun(this, &StreamEndpoint::ReadSome));
        descriptor->ConnectOnWrite(sigc::mem_fun(this, &StreamEndpoint::WriteSome));
    }

    void StreamEndpoint::SetQueue( shared_ptr<QueueBase> q,
                shared_ptr<MsgPut<NodeMessagePtr> > mfs) {
        msgtoendpoint = mfs;
        queue = q;
    }
}


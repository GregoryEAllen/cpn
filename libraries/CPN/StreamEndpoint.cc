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
        : writecount(0)
    {
        PacketDecoder::Enable(false);
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
        unsigned amount = msg->Amount();
        unsigned numchans = queue->NumChannels();
        std::vector<const void*> data(numchans, 0);
        for (unsigned chan = 0; chan < numchans; ++chan) {
            data[chan] = queue->GetRawDequeuePtr(amount, chan);
            ASSERT(data[chan], "No data in the queue, inconsistant queue state.");
        }
        encoder.SendEnqueue(&data[0], amount, numchans);
        queue->Dequeue(amount);
        writecount += amount;
    }

    void StreamEndpoint::ProcessMessage(NodeDequeue *msg) {
        unsigned amount = msg->Amount();
        unsigned numchans = queue->NumChannels();
        encoder.SendDequeue(amount, numchans);
    }

    void StreamEndpoint::ProcessMessage(NodeReadBlock *msg) {
        encoder.SendReadBlock(msg->Requested());
    }

    void StreamEndpoint::ProcessMessage(NodeWriteBlock *msg) {
        encoder.SendWriteBlock(msg->Requested());
    }

    void StreamEndpoint::Shutdown() {
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::ProcessMessage(NodeEndOfWriteQueue *msg) {
        // TODO send the message on and start our shutdown
        ASSERT(false, "Unimplemented");
    }

    void StreamEndpoint::ProcessMessage(NodeEndOfReadQueue *msg) {
        // TODO send the message on and start our shutdown
        ASSERT(false, "Unimplemented");
    }

    bool StreamEndpoint::ReadReady() {
        return PacketDecoder::Enabled();
    }

    bool StreamEndpoint::WriteReady() {
        return encoder.BytesReady();
    }

    void StreamEndpoint::ReadSome() {
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
                }
                loop = false;
            } else {
                PacketDecoder::ReleaseDecoderBytes(numread);
            }
        }
    }

    void StreamEndpoint::WriteSome() {
        Async::Stream stream(descriptor);
        while (encoder.BytesReady()) {
            unsigned towrite = 0;
            const void *ptr = encoder.GetEncodedBytes(towrite);
            if (0 == towrite) break;
            unsigned numwritten = stream.Write(ptr, towrite);
            if (numwritten == 0) {
                break;
            } else {
                encoder.ReleaseEncodedBytes(numwritten);
            }
        }
    }

    void StreamEndpoint::ReceivedEnqueue(void *data, unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number many channels.");
        // We want to send the enqueue message after we have actually enqueued the data
        ENSURE(queue->RawEnqueue(data, length, numchannels, length),
                "Enqueue throttling over the socket failed.");
        msgtoendpoint->Put(NodeEnqueue::Create(length));
    }

    void StreamEndpoint::ReceivedDequeue(unsigned length, unsigned numchannels) {
        ASSERT(numchannels == queue->NumChannels(),
                "Endpoints disagree on number of channels.");
        writecount -= length;
        CheckBlockedEnqueues();
        msgtoendpoint->Put(NodeDequeue::Create(length));
    }

    void StreamEndpoint::ReceivedReadBlock(unsigned requested) {
        msgtoendpoint->Put(NodeReadBlock::Create(requested));
    }

    void StreamEndpoint::ReceivedWriteBlock(unsigned requested) {
        msgtoendpoint->Put(NodeWriteBlock::Create(requested));
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
        PacketDecoder::Enable(true);
    }
}


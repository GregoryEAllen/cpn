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
 * \brief Implementation for kernel functions
 * \author John Bridgman
 */

#include "Kernel.h"

#include "NodeFactory.h"
#include "NodeBase.h"

#include "SimpleQueue.h"
#include "ThresholdQueue.h"

#include "Database.h"

#include "ReaderStream.h"
#include "WriterStream.h"
#include "KernelStream.h"
#include "CPNStream.h"
#include "UnknownStream.h"

#include "AutoBuffer.h"
#include "Assert.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <stdexcept>

//#define KERNEL_FUNC_TRACE
#ifdef KERNEL_FUNC_TRACE
#define FUNCBEGIN printf("%s\n",__PRETTY_FUNCTION__)
#else
#define FUNCBEGIN
#endif

namespace CPN {

    Kernel::Kernel(const KernelAttr &kattr)
        : lock(),
        status(INITIALIZED, &lock),
        kernelname(kattr.GetName()),
        hostkey(0),
        nodemap(),
        garbagenodes(),
        database(kattr.GetDatabase())
    {
        FUNCBEGIN;
        Async::SockAddrList addrlist = Async::SocketAddress::CreateIP(
                kattr.GetHostName().c_str(),
                kattr.GetServName().c_str());
        listener = Async::ListenSocket::Create(addrlist);

        if (!database) {
            database = Database::Local();
        }
        listener->ConnectReadable(sigc::mem_fun(this, &Kernel::True));
        listener->ConnectOnRead(sigc::mem_fun(this, &Kernel::ListenRead));

        hostkey = database->SetupHost(kattr);
        Async::StreamSocket::CreatePair(wakeuplisten, wakeupwriter);
        wakeuplisten->ConnectOnRead(sigc::mem_fun(this, &Kernel::WakeupReader));
        wakeuplisten->ConnectReadable(sigc::mem_fun(this, &Kernel::True));
        // Start up and don't finish until actually started.
        Pthread::Start();
        status.CompareAndWait(INITIALIZED);
    }

    Kernel::~Kernel() {
        FUNCBEGIN;
        Terminate();
        Wait();
        database->DestroyHostKey(hostkey);
        while (!unknownstreams.Empty()) {
            delete unknownstreams.PopFront();
        }
    }

    void Kernel::Wait() {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock);
        KernelStatus_t s = status.Get();
        while (s != DONE) {
            s = status.CompareAndWait(s);
        }
    }

    void Kernel::Terminate() {
        FUNCBEGIN;
        status.CompareAndPost(RUNNING, TERMINATE);
        SendWakeup();
    }

    Key_t Kernel::CreateNode(const NodeAttr &attr) {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock, false);
        Key_t nodekey = database->CreateNodeKey(attr.GetName());
        NodeAttr nodeattr = attr;
        nodeattr.SetKey(nodekey);
        // check the host the node should go on and send
        // to that particular host
        arlock.Lock();
        if (nodeattr.GetHost().empty() ||
                nodeattr.GetHost() == kernelname) {
            InternalCreateNode(nodeattr);
        } else {
            arlock.Unlock();
            Key_t key = database->WaitForHostSetup(nodeattr.GetHost());
            arlock.Lock();
            nodeattr.SetHostKey(key);
            SendCreateNode(key, nodeattr);
        }
        return nodekey;
    }

    void Kernel::WaitNodeTerminate(const std::string &nodename) {
        FUNCBEGIN;
        database->WaitForNodeEnd(nodename);
    }

    void Kernel::WaitNodeStart(const std::string &nodename) {
        FUNCBEGIN;
        database->WaitForNodeStart(nodename);
    }

    void Kernel::CreateQueue(const QueueAttr &qattr) {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock, false);
        // Normalize the QueueAttr into a SimpleQueueAttr
        // This gets rid of the names and translates to IDs
        SimpleQueueAttr attr = qattr;
        if (attr.GetReaderKey() == 0) {
            if (attr.GetReaderNodeKey() == 0) {
                if (qattr.GetReaderNode().empty()) {
                    throw std::invalid_argument(
                            "The reader side must be specified with the reader key"
                            " or the reader name and node key or the reader name and"
                            " node name.");
                }
                attr.SetReaderNodeKey(database->WaitForNodeStart(qattr.GetReaderNode()));
            }
            if (qattr.GetReaderPort().empty()) {
                throw std::invalid_argument("Ether the port key or port name must be specified.");
            }
            attr.SetReaderKey(database->GetCreateReaderKey(attr.GetReaderNodeKey(),
                        qattr.GetReaderPort()));
        } else if (attr.GetReaderNodeKey() == 0) {
            attr.SetReaderNodeKey(database->GetReaderNode(attr.GetReaderKey()));
        }

        if (attr.GetWriterKey() == 0) {
            if (attr.GetWriterNodeKey() == 0) {
                if (qattr.GetWriterNode().empty()) {
                    throw std::invalid_argument(
                            "The writer side must be specified with the writer key"
                            " or the writer name and node key or the writer name and"
                            " node name.");
                }
                attr.SetWriterNodeKey(database->WaitForNodeStart(qattr.GetWriterNode()));
            }
            if (qattr.GetWriterPort().empty()) {
                throw std::invalid_argument("Ether the port key or port name must be specified.");
            }
            attr.SetWriterKey(database->GetCreateWriterKey(attr.GetWriterNodeKey(),
                        qattr.GetWriterPort()));
        } else if (attr.GetWriterNodeKey() == 0) {
            attr.SetWriterNodeKey(database->GetWriterNode(attr.GetWriterKey()));
        }

        database->ConnectEndpoints(attr.GetWriterKey(), attr.GetReaderKey());

        Key_t readerhost = database->GetNodeHost(attr.GetReaderNodeKey());
        Key_t writerhost = database->GetNodeHost(attr.GetWriterNodeKey());

        if (readerhost == writerhost) {
            if (readerhost == hostkey) {
                CreateLocalQueue(attr);
            } else {
                // Send a message to the other host to create a local queue
                SendCreateQueue(readerhost, attr);
            }
        } else if (readerhost == hostkey) {
            // Create the reader end here and queue up a message
            // to the writer host that they need to create an endpoint
            CreateReaderEndpoint(attr);
            SendCreateWriter(writerhost, attr);
        } else if (writerhost == hostkey) {
            // Create the writer end here and queue up a message to
            // the reader host that they need to create an endpoint
            CreateWriterEndpoint(attr);
            SendCreateReader(readerhost, attr);
        } else {
            // Queue up a message to both the reader and writer host
            // to create endpoints
            SendCreateWriter(writerhost, attr);
            SendCreateReader(readerhost, attr);
        }
    }

    void Kernel::SendCreateWriter(Key_t writerhost, const SimpleQueueAttr &attr) {
    }
    void Kernel::SendCreateReader(Key_t readerhost, const SimpleQueueAttr &attr) {
    }
    void Kernel::SendCreateQueue(Key_t rwhost, const SimpleQueueAttr &attr) {
    }
    void Kernel::SendCreateNode(Key_t nhost, const NodeAttr &attr) {
    }

    void Kernel::CreateReaderEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        shared_ptr<QueueBase> queue = MakeQueue(attr);
        shared_ptr<NodeBase> readernode = nodemap[attr.GetReaderNodeKey()];
        ASSERT(readernode);
        readernode->GetNodeMessageHandler()->
            CreateReader(attr.GetReaderKey(), attr.GetWriterKey(), queue);

        shared_ptr<ReaderStream> stream;
        StreamMap::iterator entry = streammap.find(attr.GetReaderKey());
        if (entry == streammap.end()) {
            stream = shared_ptr<ReaderStream>(
                    new ReaderStream(this, attr.GetReaderKey(),
                        attr.GetWriterKey()));
            streammap.insert(std::make_pair(attr.GetReaderKey(), stream));
        } else {
            stream = dynamic_pointer_cast<ReaderStream>(entry->second);
        }
        stream->SetQueue(queue);
    }

    void Kernel::CreateWriterEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        shared_ptr<QueueBase> queue = MakeQueue(attr);
        shared_ptr<NodeBase> writernode = nodemap[attr.GetWriterNodeKey()];
        ASSERT(writernode);
        writernode->GetNodeMessageHandler()->
            CreateWriter(attr.GetReaderKey(), attr.GetWriterKey(), queue);

        shared_ptr<WriterStream> stream;
        StreamMap::iterator entry = streammap.find(attr.GetWriterKey());
        if (entry == streammap.end()) {
            stream = shared_ptr<WriterStream>(
                    new WriterStream(this, attr.GetWriterKey(),
                        attr.GetReaderKey()));
            streammap.insert(std::make_pair(attr.GetWriterKey(), stream));
        } else {
            stream = dynamic_pointer_cast<WriterStream>(entry->second);
        }
        stream->SetQueue(queue);
    }

    void Kernel::CreateLocalQueue(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);

        shared_ptr<QueueBase> queue = MakeQueue(attr);

        shared_ptr<NodeBase> readernode = nodemap[attr.GetReaderNodeKey()];
        ASSERT(readernode);
        readernode->GetNodeMessageHandler()->
            CreateReader(attr.GetReaderKey(), attr.GetWriterKey(), queue);

        shared_ptr<NodeBase> writernode = nodemap[attr.GetWriterNodeKey()];
        ASSERT(writernode);
        writernode->GetNodeMessageHandler()->
            CreateWriter(attr.GetReaderKey(), attr.GetWriterKey(), queue);
    }

    shared_ptr<QueueBase> Kernel::MakeQueue(const SimpleQueueAttr &attr) {
        shared_ptr<QueueBase> queue;
        switch (attr.GetHint()) {
        case QUEUEHINT_THRESHOLD:
            queue = shared_ptr<QueueBase>(new ThresholdQueue(attr.GetLength(),
                attr.GetMaxThreshold(), attr.GetNumChannels()));
            break;
        default:
            queue = shared_ptr<QueueBase>(new SimpleQueue(attr.GetLength(),
                attr.GetMaxThreshold(), attr.GetNumChannels()));
            break;
        }
        return queue;
    }

    void Kernel::Logf(int level, const char* const fmt, ...) {
        FUNCBEGIN;
        // This code was taken from an example of how
        // to use vsnprintf in the unix man pages.
        va_list ap;
        AutoBuffer buff(100);
        while (1) {
            /* Try to print in the allocated space. */
            va_start(ap, fmt);
            int n = vsnprintf((char*)buff.GetBuffer(), buff.GetSize(), fmt, ap);
            va_end(ap);
            /* If that worked, return the string. */
            if (n > -1 && unsigned(n) < buff.GetSize()) {
                std::string ret = (char*)buff.GetBuffer();
                Log(level, ret);
                return;
            }
            /* Else try again with more space. */
            if (n > -1) { /* glibc 2.1 */ /* precisely what is needed */
                buff.ChangeSize(n+1);
            }
            else { /* glibc 2.0 */ /* twice the old size */
                buff.ChangeSize(buff.GetSize()*2);
            }
        }
    }

    void Kernel::Log(int level, const std::string &msg) {
        FUNCBEGIN;
        database->Log(level, msg);
    }

    void Kernel::InternalCreateNode(NodeAttr &nodeattr) {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock);
        nodeattr.SetDatabase(database);
        database->AffiliateNodeWithHost(hostkey, nodeattr.GetKey());
        shared_ptr<NodeFactory> factory = CPNGetNodeFactory(nodeattr.GetTypeName());
        if (!factory) {
            throw std::invalid_argument("No such node type " + nodeattr.GetTypeName());
        }
        shared_ptr<NodeBase> node = factory->Create(*this, nodeattr);
        nodemap.insert(std::make_pair(nodeattr.GetKey(), node));
        node->Start();
    }

    void Kernel::NodeTerminated(Key_t key) {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock);
        database->DestroyNodeKey(key);
        shared_ptr<NodeBase> node = nodemap[key];
        nodemap.erase(key);
        garbagenodes.push_back(node);
        SendWakeup();
    }

    void Kernel::ClearGarbage() {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock);
        garbagenodes.clear();
    }

    void Kernel::HandleMessages() {
        Sync::AutoReentrantLock arlock(lock);
        /*
        std::list<KernelMessagePtr>::iterator itr;
        itr = msgqueue.begin();
        while (itr != msgqueue.end()) {
            KernelMessagePtr msg = *itr;
            // Look at the destination of the message.
            if (msg->GetDestinationKey() == hostkey) {
                // Is it us? process it.
                msg->DispatchOn(this);
                itr = msgqueue.erase(itr);
            } else {
                // if not us lookup to see if we have a control
                StreamMap::iterator entry = streammap.find(msg->GetDestinationKey());
                if (entry == streammap.end()) {
                    // if we dont have one create one and keep the message around
                    ++itr;
                } else {
                    shared_ptr<KernelStream> kernelstream =
                        dynamic_pointer_cast<KernelStream>(entry->second);
                    if (kernelstream->Connected()) {
                        // if we have one forward it
                        msg->DispatchOn(kernelstream.get());
                        itr = msgqueue.erase(itr);
                    } else {
                        // It is waiting to be connected keep the message around
                        ++itr;
                    }
                }
            }
        }
        */
    }

    void *Kernel::EntryPoint() {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock, false);
        status.CompareAndPost(INITIALIZED, RUNNING);
        while (status.Get() == RUNNING) {
            ClearGarbage();
            HandleMessages();
            std::vector<Async::DescriptorPtr> descriptors;
            descriptors.push_back(wakeuplisten);
            descriptors.push_back(listener);

            for (StreamMap::iterator itr = streammap.begin();
                    itr != streammap.end(); ++itr) {
                itr->second->RunOneIteration();
                itr->second->RegisterDescriptor(descriptors);
            }

            ENSURE(Async::Descriptor::Poll(descriptors, -1) > 0);
        }
        // Close the listen port
        listener.reset();
        // Tell everybody that is left to die.
        arlock.Lock();
        for (NodeMap::iterator itr = nodemap.begin();
                itr != nodemap.end(); ++itr) {
            itr->second->Shutdown();
        }
        while (!nodemap.empty()) {
            arlock.Unlock();
            std::vector<Async::DescriptorPtr> descriptors;
            descriptors.push_back(wakeuplisten);
            ENSURE(Async::Descriptor::Poll(descriptors, -1) > 0);
            arlock.Lock();
        }
        arlock.Unlock();
        status.Post(DONE);
        return 0;
    }

    void Kernel::WakeupReader() {
        FUNCBEGIN;
        Async::Stream stream(wakeuplisten);
        // read data out of the stream until it's empty
        // the buffer size is random
        char buffer[256];
        while (stream.Read(buffer, sizeof(buffer)) > 0);
        ASSERT(stream, "EOF on the wakeup pipe!!!");
    }

    void Kernel::SendWakeup() {
        FUNCBEGIN;
        Async::Stream stream(wakeupwriter);
        char c = 0;
        while(stream.Write(&c, sizeof(c)) != sizeof(c));
    }

    void Kernel::ListenRead() {
        Async::SockPtr sock = listener->Accept();
        unknownstreams.PushFront(new UnknownStream(sock, this));
    }

    void Kernel::CreateWriter(Key_t src, Key_t dst, const SimpleQueueAttr &attr) {
        CreateWriterEndpoint(attr);
    }
    void Kernel::CreateReader(Key_t src, Key_t dst, const SimpleQueueAttr &attr) {
        CreateReaderEndpoint(attr);
    }
    void Kernel::CreateQueue(Key_t src, Key_t dst, const SimpleQueueAttr &attr) {
        CreateLocalQueue(attr);
    }
    void Kernel::CreateNode(Key_t src, Key_t dst, const NodeAttr &attr) {
        NodeAttr nodeattr(attr);
        InternalCreateNode(nodeattr);
    }

    void Kernel::StreamDead(Key_t streamkey) {
        streammap.erase(streamkey);
    }

    void Kernel::SetReaderDescriptor(Key_t readerkey, Async::DescriptorPtr desc) {
        StreamMap::iterator entry = streammap.find(readerkey);
        if (entry == streammap.end()) {
        }
    }

    void Kernel::SetWriterDescriptor(Key_t writerkey, Async::DescriptorPtr desc) {
        StreamMap::iterator entry = streammap.find(writerkey);
        if (entry == streammap.end()) {
        }
    }

    void Kernel::NewKernelStream(Key_t kernelkey, Async::DescriptorPtr desc) {
        StreamMap::iterator entry = streammap.find(kernelkey);
        if (entry == streammap.end()) {
        }
    }
}


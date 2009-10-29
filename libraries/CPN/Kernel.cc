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

#include "SocketAddress.h"
#include "FileHandler.h"

#include "Assert.h"
#include "Logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include <stdexcept>
#include <cassert>

//#define KERNEL_FUNC_TRACE
#ifdef KERNEL_FUNC_TRACE
#define FUNCBEGIN printf("%s begin %s\n",__PRETTY_FUNCTION__, kernelname.c_str())
#define FUNCEND printf("%s end %s\n",__PRETTY_FUNCTION__, kernelname.c_str())
#else
#define FUNCBEGIN
#define FUNCEND
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
        SockAddrList addrlist = SocketAddress::CreateIP(
                kattr.GetHostName().c_str(),
                kattr.GetServName().c_str());
        listener.Connect(addrlist);

        if (!database) {
            database = Database::Local();
        }
        logger.Output(database.get());
        logger.LogLevel(database->LogLevel());
        logger.Name(kernelname);

        SocketAddress addr;
        addr.SetFromSockName(listener.FD());
        hostkey = database->SetupHost(kernelname, addr.GetHostName(), addr.GetServName(), this);

        logger.Info("New kernel, listening on %s:%s", addr.GetHostName().c_str(), addr.GetServName().c_str());
        // Start up and don't finish until actually started.
        Pthread::Start();
        status.CompareAndWait(INITIALIZED);
    }

    Kernel::~Kernel() {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        Terminate();
        Wait();
        database->DestroyHostKey(hostkey);
        assert(status.Get() == DONE);
        assert(nodemap.empty());
        assert(garbagenodes.empty());
        assert(streammap.empty());
        assert(unknownstreams.empty());
        FUNCEND;
    }

    void Kernel::Wait() {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        KernelStatus_t s = status.Get();
        while (s != DONE) {
            s = status.CompareAndWait(s);
        }
        FUNCEND;
    }

    void Kernel::Terminate() {
        FUNCBEGIN;
        status.CompareAndPost(RUNNING, TERMINATE);
        wakeuphandler.SendWakeup();
    }

    Key_t Kernel::CreateNode(const NodeAttr &attr) {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock, false);
        ASSERT(status.Get() == RUNNING);
        Key_t nodekey = database->CreateNodeKey(attr.GetName());
        NodeAttr nodeattr = attr;
        nodeattr.SetKey(nodekey);
        // check the host the node should go on and send
        // to that particular host
        arlock.Lock();
        if (nodeattr.GetHost().empty() ||
                nodeattr.GetHost() == kernelname) {
            nodeattr.SetHostKey(hostkey);
            InternalCreateNode(nodeattr);
        } else {
            arlock.Unlock();
            Key_t key = database->WaitForHostSetup(nodeattr.GetHost());
            nodeattr.SetHostKey(key);
            database->SendCreateNode(key, nodeattr);
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
        ASSERT(status.Get() == RUNNING);
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
                database->SendCreateQueue(readerhost, attr);
            }
        } else if (readerhost == hostkey) {
            // Create the reader end here and queue up a message
            // to the writer host that they need to create an endpoint
            CreateReaderEndpoint(attr);
            database->SendCreateWriter(writerhost, attr);
        } else if (writerhost == hostkey) {
            // Create the writer end here and queue up a message to
            // the reader host that they need to create an endpoint
            CreateWriterEndpoint(attr);
            database->SendCreateReader(readerhost, attr);
        } else {
            // Queue up a message to both the reader and writer host
            // to create endpoints
            database->SendCreateWriter(writerhost, attr);
            database->SendCreateReader(readerhost, attr);
        }
    }

    void Kernel::CreateReaderEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);

        shared_ptr<QueueBase> queue;

        shared_ptr<NodeBase> readernode = nodemap[attr.GetReaderNodeKey()];
        ASSERT(readernode);

        readernode->GetNodeMessageHandler()->
            CreateReader(attr.GetReaderKey(), attr.GetWriterKey(), queue);
    }

    void Kernel::CreateWriterEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);

        shared_ptr<NodeBase> writernode = nodemap[attr.GetWriterNodeKey()];
        ASSERT(writernode);

        writernode->GetNodeMessageHandler()->
            CreateWriter(attr.GetReaderKey(), attr.GetWriterKey(), queue);
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

    void Kernel::InternalCreateNode(NodeAttr &nodeattr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(status.Get() == RUNNING);
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
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(status.Get() != DONE);
        database->DestroyNodeKey(key);
        shared_ptr<NodeBase> node = nodemap[key];
        nodemap.erase(key);
        garbagenodes.push_back(node);
        wakeuphandler.SendWakeup();
    }

    void Kernel::ClearGarbage() {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        garbagenodes.clear();
    }

    void *Kernel::EntryPoint() {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock, false);
        status.CompareAndPost(INITIALIZED, RUNNING);
        while (status.Get() == RUNNING) {
            ClearGarbage();
            Poll();
        }
        // Close the listen port
        listener.Close();
        // Tell everybody that is left to die.
        arlock.Lock();
        for (NodeMap::iterator itr = nodemap.begin();
                itr != nodemap.end(); ++itr) {
            itr->second->Shutdown();
        }
        for (EndpointMap::iterator itr = endpointmap.begin();
                itr != endpointmap.end(); ++itr) {
            itr->second->Shutdown();
        }
        // Wait for all nodes to end and all endpoints
        // to finish sending data
        while (!nodemap.empty()) {
            arlock.Unlock();
            ClearGarbage();
            Poll();
            arlock.Lock();
        }
        arlock.Unlock();
        ClearGarbage();
        status.Post(DONE);
        FUNCEND;
        return 0;
    }

    void Kernel::Poll() {
        Sync::AutoReentrantLock arlock(lock, false);
        double timeout = static_cast<unsigned>(-1); // a long time, arbitrary
        std::vector<FileHandler*> filehandlers;
        if (!listener.Closed()) { filehandlers.push_back(&listener); }
        if (!wakeuphandler.Closed()) { filehandlers.push_back(&wakeuphandler); }

        arlock.Lock();
        for (EndpointMap::iterator itr = endpointmap.begin();
                itr != endpointmap.end(); ++itr) {
            double time = itr->second->CheckStatus();
            if (time < timeout) { timeout = time; }
            if (!itr->second->Closed()) {
                filehandlers.push_back(itr->second.get());
            }
        }
        arl.Unlock();

        FileHandler::Poll(&filehandlers[0], filehandlers.size(), timeout);
    }

    void Kernel::CreateWriter(Key_t dst, const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(dst == hostkey);
        CreateWriterEndpoint(attr);
    }
    void Kernel::CreateReader(Key_t dst, const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(dst == hostkey);
        CreateReaderEndpoint(attr);
    }
    void Kernel::CreateQueue(Key_t dst, const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(dst == hostkey);
        CreateLocalQueue(attr);
    }
    void Kernel::CreateNode(Key_t dst, const NodeAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(dst == hostkey);
        NodeAttr nodeattr(attr);
        InternalCreateNode(nodeattr);
    }

    void Kernel::StreamDead(Key_t streamkey) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
    }

    void Kernel::SetReaderDescriptor(Key_t readerkey, Key_t writerkey, Async::DescriptorPtr desc) {
        FUNCBEGIN;
    }

    void Kernel::SetWriterDescriptor(Key_t writerkey, Key_t readerkey, Async::DescriptorPtr desc) {
        FUNCBEGIN;
    }

    weak_ptr<UnknownStream> Kernel::CreateNewQueueStream(Key_t readerkey, Key_t writerkey) {
        FUNCBEGIN;
    }

}


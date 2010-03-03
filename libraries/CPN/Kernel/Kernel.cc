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

#include "Exceptions.h"

#include "NodeFactory.h"
#include "NodeBase.h"

#include "ThresholdQueue.h"

#include "Database.h"

#include "RemoteQueue.h"

#include "SocketAddress.h"

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
        database(kattr.GetDatabase())
    {
        FUNCBEGIN;
        if (!database) {
            database = Database::Local();
        }
        logger.Output(database.get());
        logger.LogLevel(database->LogLevel());
        logger.Name(kernelname);

        SockAddrList addrlist = SocketAddress::CreateIP(kattr.GetHostName(),
                kattr.GetServName());
        server.reset(new ConnectionServer(addrlist, database));

        SocketAddress addr = server->GetAddress();
        hostkey = database->SetupHost(kernelname, addr.GetHostName(), addr.GetServName(), this);

        logger.Info("New kernel, listening on %s:%s", addr.GetHostName().c_str(), addr.GetServName().c_str());
        // Start up and don't finish until actually started.
        Pthread::Start();
        status.CompareAndWait(INITIALIZED);
    }

    Kernel::~Kernel() {
        FUNCBEGIN;
        Terminate();
        Wait();
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
        database->Terminate();
    }

    void Kernel::NotifyTerminate() {
        FUNCBEGIN;
        if (status.CompareAndPost(RUNNING, TERMINATE)) {
            server->Wakeup();
        }
    }

    Key_t Kernel::CreateNode(const NodeAttr &attr) {
        FUNCBEGIN;
        Sync::AutoReentrantLock arlock(lock);
        Key_t ourkey = hostkey;
        arlock.Unlock();

        NodeAttr nodeattr = attr;

        if (nodeattr.GetHostKey() == 0) {
            Key_t key = 0;
            if (nodeattr.GetHost().empty()) {
                key = ourkey;
            } else {
                key = database->WaitForHostStart(nodeattr.GetHost());
            }
            nodeattr.SetHostKey(key);
        }
        Key_t nodekey = database->CreateNodeKey(nodeattr.GetHostKey(), nodeattr.GetName());
        nodeattr.SetKey(nodekey);

        // check the host the node should go on and send
        // to that particular host
        if (nodeattr.GetHostKey() == ourkey) {
            InternalCreateNode(nodeattr);
        } else {
            database->SendCreateNode(nodeattr.GetHostKey(), nodeattr);
        }
        return nodekey;
    }

    void Kernel::WaitNodeTerminate(const std::string &nodename) {
        FUNCBEGIN;
        database->WaitForNodeEnd(nodename);
    }

    void Kernel::WaitForAllNodeEnd() {
        database->WaitForAllNodeEnd();
    }

    void Kernel::WaitNodeStart(const std::string &nodename) {
        FUNCBEGIN;
        database->WaitForNodeStart(nodename);
    }

    void Kernel::CreateQueue(const QueueAttr &qattr) {
        FUNCBEGIN;
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

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    database,
                    RemoteQueue::READ,
                    server.get(),
                    attr
                    ));


        NodeMap::iterator entry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        entry->second->CreateReader(endp);
    }

    void Kernel::CreateWriterEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    database,
                    RemoteQueue::WRITE,
                    server.get(),
                    attr
                    ));

        NodeMap::iterator entry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        entry->second->CreateWriter(endp);
    }

    void Kernel::CreateLocalQueue(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);

        shared_ptr<QueueBase> queue;
        queue = shared_ptr<QueueBase>(new ThresholdQueue(database, attr));

        NodeMap::iterator entry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(entry != nodemap.end(), "Tried to connect a queue to a node that doesn't exist.");
        entry->second->CreateReader(queue);

        entry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(entry != nodemap.end(), "Tried to connect a queue to a node that doesn't exist.");
        entry->second->CreateWriter(queue);
    }

    void Kernel::InternalCreateNode(NodeAttr &nodeattr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(status.Get() == RUNNING);
        nodeattr.SetDatabase(database);
        NodeFactory *factory = database->GetNodeFactory(nodeattr.GetTypeName());
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
        if (status.Get() == DONE) {
            logger.Warn("Nodes running after shutdown");
        } else {
            NodeMap::iterator entry = nodemap.find(key);
            ASSERT(entry != nodemap.end());
            shared_ptr<NodeBase> node = entry->second;
            nodemap.erase(entry);
            garbagenodes.push_back(node);
            SendWakeup();
        }
        cond.Signal();
    }

    void Kernel::ClearGarbage() {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        garbagenodes.clear();
    }

    void *Kernel::EntryPoint() {
        FUNCBEGIN;
        status.CompareAndPost(INITIALIZED, RUNNING);
        try {
            database->SignalHostStart(hostkey);
            while (status.Get() == RUNNING) {
                ClearGarbage();
                server->Poll();
            }
            server->Close();
            Sync::AutoReentrantLock arlock(lock);
            NodeMap::iterator nitr = nodemap.begin();
            while (nitr != nodemap.end()) {
                (nitr++)->second->NotifyTerminate();
            }
            // Wait for all nodes to end
            while (!nodemap.empty()) {
                garbagenodes.clear();
                cond.Wait(lock);
            }
        } catch (const ShutdownException &e) {
            logger.Warn("Kernel forced shutdown");
        }
        ClearGarbage();
        database->DestroyHostKey(hostkey);
        status.Post(DONE);
        FUNCEND;
        return 0;
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

}


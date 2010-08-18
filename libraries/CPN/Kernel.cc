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
#include "ErrnoException.h"
#include "PthreadFunctional.h"

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
        status(INITIALIZED),
        kernelname(kattr.GetName()),
        hostkey(0),
        database(kattr.GetDatabase()),
        useremote(kattr.GetRemoteEnabled())
    {
        FUNCBEGIN;
        thread.reset(CreatePthreadFunctional(this, &Kernel::EntryPoint));
        if (thread->Error() != 0) {
            throw ErrnoException("Could not create thread", thread->Error());
        }
        if (!database) {
            database = Database::Local();
        }
        if (database->RequireRemote()) {
            useremote = true;
        }
        logger.Output(database.get());
        logger.LogLevel(database->LogLevel());
        logger.Name(kernelname);

        if (useremote) {
            SockAddrList addrlist = SocketAddress::CreateIP(kattr.GetHostName(),
                    kattr.GetServName());
            server.reset(new ConnectionServer(addrlist, database));

            SocketAddress addr = server->GetAddress();
            hostkey = database->SetupHost(kernelname, addr.GetHostName(), addr.GetServName(), this);
            remotequeueholder.reset(new RemoteQueueHolder());

            logger.Info("New kernel, listening on %s:%s", addr.GetHostName().c_str(), addr.GetServName().c_str());
        } else {
            hostkey = database->SetupHost(kernelname, this);
            logger.Info("New kernel");
        }
        // Start up and don't finish until actually started.
        thread->Start();
        status.CompareAndWait(INITIALIZED);
    }

    Kernel::~Kernel() {
        FUNCBEGIN;
        Terminate();
        Wait();
        thread->Join();
        FUNCEND;
    }

    void Kernel::Wait() {
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
            SendWakeup();
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
                ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
                // Send a message to the other host to create a local queue
                database->SendCreateQueue(readerhost, attr);
            }
        } else if (readerhost == hostkey) {
            ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
            // Create the reader end here and queue up a message
            // to the writer host that they need to create an endpoint
            CreateReaderEndpoint(attr);
            database->SendCreateWriter(writerhost, attr);
        } else if (writerhost == hostkey) {
            ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
            // Create the writer end here and queue up a message to
            // the reader host that they need to create an endpoint
            CreateWriterEndpoint(attr);
            database->SendCreateReader(readerhost, attr);
        } else {
            ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
            // Queue up a message to both the reader and writer host
            // to create endpoints
            database->SendCreateWriter(writerhost, attr);
            database->SendCreateReader(readerhost, attr);
        }
    }

    void Kernel::CreateReaderEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    database,
                    RemoteQueue::READ,
                    server.get(),
                    remotequeueholder.get(),
                    attr
                    ));


        NodeMap::iterator entry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        shared_ptr<NodeBase> node = entry->second;
        remotequeueholder->AddQueue(endp);
        endp->Start();
        arlock.Unlock();
        node->CreateReader(endp);
    }

    void Kernel::CreateWriterEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    database,
                    RemoteQueue::WRITE,
                    server.get(),
                    remotequeueholder.get(),
                    attr
                    ));

        NodeMap::iterator entry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        shared_ptr<NodeBase> node = entry->second;
        remotequeueholder->AddQueue(endp);
        endp->Start();
        arlock.Unlock();
        node->CreateWriter(endp);
    }

    void Kernel::CreateLocalQueue(const SimpleQueueAttr &attr) {
        shared_ptr<QueueBase> queue;
        queue = shared_ptr<QueueBase>(new ThresholdQueue(database, attr));

        Sync::AutoReentrantLock arlock(lock);
        NodeMap::iterator readentry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(readentry != nodemap.end(), "Tried to connect a queue to a node that doesn't exist.");
        shared_ptr<NodeBase> readnode = readentry->second;

        NodeMap::iterator writeentry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(writeentry != nodemap.end(), "Tried to connect a queue to a node that doesn't exist.");
        shared_ptr<NodeBase> writenode = writeentry->second;
        arlock.Unlock();

        writenode->CreateWriter(queue);
        readnode->CreateReader(queue);
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

    void Kernel::SendWakeup() {
        Sync::AutoReentrantLock arlock(lock);
        if (useremote) {
            server->Wakeup();
        }
        cond.Signal();
    }

    void *Kernel::EntryPoint() {
        FUNCBEGIN;
        status.CompareAndPost(INITIALIZED, RUNNING);
        try {
            database->SignalHostStart(hostkey);
            if (useremote) {
                while (status.Get() == RUNNING) {
                    ClearGarbage();
                    remotequeueholder->Cleanup();
                    server->Poll();
                }
            } else {
                Sync::AutoReentrantLock arlock(lock);
                while (status.Get() == RUNNING) {
                    garbagenodes.clear();
                    cond.Wait(lock);
                }
            }
        } catch (const ShutdownException &e) {
            logger.Warn("Kernel forced shutdown");
        }
        if (useremote) {
            server->Close();
            remotequeueholder->Shutdown();
        }
        {
            Sync::AutoReentrantLock arlock(lock);
            NodeMap mapcopy = nodemap;
            arlock.Unlock();
            NodeMap::iterator nitr = mapcopy.begin();
            while (nitr != mapcopy.end()) {
                (nitr++)->second->NotifyTerminate();
            }
            arlock.Lock();
            // Wait for all nodes to end
            while (!nodemap.empty()) {
                garbagenodes.clear();
                cond.Wait(lock);
            }
        }
        database->DestroyHostKey(hostkey);
        status.Post(DONE);
        FUNCEND;
        return 0;
    }

    void Kernel::CreateWriter(Key_t dst, const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        ASSERT(dst == hostkey);
        CreateWriterEndpoint(attr);
    }
    void Kernel::CreateReader(Key_t dst, const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        ASSERT(dst == hostkey);
        CreateReaderEndpoint(attr);
    }
    void Kernel::CreateQueue(Key_t dst, const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        ASSERT(dst == hostkey);
        CreateLocalQueue(attr);
    }
    void Kernel::CreateNode(Key_t dst, const NodeAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        ASSERT(dst == hostkey);
        NodeAttr nodeattr(attr);
        InternalCreateNode(nodeattr);
    }

    void Kernel::LogState() {
        // Note that this function does not aquire the lock...
        // this is because it is meant to be called from the debugger while
        // the application is halted and may already be in the lock or not.
        std::string statename;
        switch (status.Get()) {
        case INITIALIZED:
            statename = "initialized";
            break;
        case RUNNING:
            statename = "running";
            break;
        case TERMINATE:
            statename = "terminated";
            break;
        case DONE:
            statename = "done";
            break;
        }
        logger.Error("Kernel %s (%llu) in state %s", kernelname.c_str(), hostkey, statename.c_str());
        logger.Error("Active nodes: %u, Garbage nodes: %u", nodemap.size(), garbagenodes.size());
        if (useremote) {
            server->LogState();
        }
        NodeMap::iterator node = nodemap.begin();
        while (node != nodemap.end()) {
            node->second->LogState();
            ++node;
        }
    }
}


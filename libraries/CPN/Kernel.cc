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
#include "ConnectionServer.h"
#include "RemoteQueueHolder.h"
#include "Pthread.h"
#include "RemoteQueue.h"
#include "SocketAddress.h"
#include "Assert.h"
#include "Logger.h"
#include "ErrnoException.h"
#include "PthreadFunctional.h"
#include <stdexcept>

//#define KERNEL_FUNC_TRACE
#ifdef KERNEL_FUNC_TRACE
#include <stdio.h>
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
        context(kattr.GetContext()),
        useremote(kattr.GetRemoteEnabled())
    {
        FUNCBEGIN;
        thread.reset(CreatePthreadFunctional(this, &Kernel::EntryPoint));
        if (!context) {
            context = Context::Local();
        }
        if (context->RequireRemote()) {
            useremote = true;
        }
        logger.Output(context.get());
        logger.LogLevel(context->LogLevel());
        logger.Name(kernelname);

        if (useremote) {
            SockAddrList addrlist = SocketAddress::CreateIP(kattr.GetHostName(),
                    kattr.GetServName());
            server.reset(new ConnectionServer(addrlist, context));

            SocketAddress addr = server->GetAddress();
            hostkey = context->SetupHost(kernelname, addr.GetHostName(), addr.GetServName(), this);
            remotequeueholder.reset(new RemoteQueueHolder());

            logger.Info("New kernel, listening on %s:%s", addr.GetHostName().c_str(), addr.GetServName().c_str());
        } else {
            hostkey = context->SetupHost(kernelname, this);
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
        context->Terminate();
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
                key = context->WaitForHostStart(nodeattr.GetHost());
            }
            nodeattr.SetHostKey(key);
        }
        Key_t nodekey = context->CreateNodeKey(nodeattr.GetHostKey(), nodeattr.GetName());
        nodeattr.SetKey(nodekey);

        // check the host the node should go on and send
        // to that particular host
        if (nodeattr.GetHostKey() == ourkey) {
            InternalCreateNode(nodeattr);
        } else {
            context->SendCreateNode(nodeattr.GetHostKey(), nodeattr);
        }
        return nodekey;
    }

    Key_t Kernel::CreatePseudoNode(const std::string &nodename) {
        Sync::AutoReentrantLock arlock(lock);
        Key_t ourkey = hostkey;
        arlock.Unlock();
        Key_t nodekey = context->CreateNodeKey(ourkey, nodename);
        shared_ptr<PseudoNode> pnode;
        pnode.reset(new PseudoNode(nodename, nodekey, context));
        arlock.Lock();
        nodemap.insert(std::make_pair(nodekey, pnode));
        arlock.Unlock();
        context->SignalNodeStart(nodekey);
        return nodekey;
    }

    Key_t Kernel::GetPseudoNode(const std::string &nodename) {
        Key_t nodekey = context->GetNodeKey(nodename);
        Sync::AutoReentrantLock arlock(lock);
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid PseudoNode.");
        }
        return nodekey;
    }

    shared_ptr<QueueWriter> Kernel::GetPseudoWriter(Key_t key, const std::string &portname) {
        Sync::AutoReentrantLock arlock(lock);
        NodeMap::iterator entry = nodemap.find(key);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid PseudoNode.");
        }
        shared_ptr<PseudoNode> pnode = entry->second;
        arlock.Unlock();
        return pnode->GetWriter(portname);
    }

    shared_ptr<QueueReader> Kernel::GetPseudoReader(Key_t key, const std::string &portname) {
        Sync::AutoReentrantLock arlock(lock);
        NodeMap::iterator entry = nodemap.find(key);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid PseudoNode.");
        }
        shared_ptr<PseudoNode> pnode = entry->second;
        arlock.Unlock();
        return pnode->GetReader(portname);
    }

    void Kernel::DestroyPseudoNode(Key_t key) {
        Sync::AutoReentrantLock arlock(lock);
        NodeMap::iterator entry = nodemap.find(key);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid PseudoNode.");
        }
        NodeTerminated(key);
    }

    void Kernel::WaitNodeTerminate(const std::string &nodename) {
        FUNCBEGIN;
        context->WaitForNodeEnd(nodename);
    }

    void Kernel::WaitForAllNodeEnd() {
        context->WaitForAllNodeEnd();
    }

    void Kernel::WaitNodeStart(const std::string &nodename) {
        FUNCBEGIN;
        context->WaitForNodeStart(nodename);
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
                attr.SetReaderNodeKey(context->WaitForNodeStart(qattr.GetReaderNode()));
            }
            if (qattr.GetReaderPort().empty()) {
                throw std::invalid_argument("Ether the port key or port name must be specified.");
            }
            attr.SetReaderKey(context->GetCreateReaderKey(attr.GetReaderNodeKey(),
                        qattr.GetReaderPort()));
        } else if (attr.GetReaderNodeKey() == 0) {
            attr.SetReaderNodeKey(context->GetReaderNode(attr.GetReaderKey()));
        }

        if (attr.GetWriterKey() == 0) {
            if (attr.GetWriterNodeKey() == 0) {
                if (qattr.GetWriterNode().empty()) {
                    throw std::invalid_argument(
                            "The writer side must be specified with the writer key"
                            " or the writer name and node key or the writer name and"
                            " node name.");
                }
                attr.SetWriterNodeKey(context->WaitForNodeStart(qattr.GetWriterNode()));
            }
            if (qattr.GetWriterPort().empty()) {
                throw std::invalid_argument("Ether the port key or port name must be specified.");
            }
            attr.SetWriterKey(context->GetCreateWriterKey(attr.GetWriterNodeKey(),
                        qattr.GetWriterPort()));
        } else if (attr.GetWriterNodeKey() == 0) {
            attr.SetWriterNodeKey(context->GetWriterNode(attr.GetWriterKey()));
        }

        context->ConnectEndpoints(attr.GetWriterKey(), attr.GetReaderKey());

        Key_t readerhost = context->GetNodeHost(attr.GetReaderNodeKey());
        Key_t writerhost = context->GetNodeHost(attr.GetWriterNodeKey());

        if (readerhost == writerhost) {
            if (readerhost == hostkey) {
                CreateLocalQueue(attr);
            } else {
                ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
                // Send a message to the other host to create a local queue
                context->SendCreateQueue(readerhost, attr);
            }
        } else if (readerhost == hostkey) {
            ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
            // Create the reader end here and queue up a message
            // to the writer host that they need to create an endpoint
            CreateReaderEndpoint(attr);
            context->SendCreateWriter(writerhost, attr);
        } else if (writerhost == hostkey) {
            ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
            // Create the writer end here and queue up a message to
            // the reader host that they need to create an endpoint
            CreateWriterEndpoint(attr);
            context->SendCreateReader(readerhost, attr);
        } else {
            ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");
            // Queue up a message to both the reader and writer host
            // to create endpoints
            context->SendCreateWriter(writerhost, attr);
            context->SendCreateReader(readerhost, attr);
        }
    }

    void Kernel::CreateReaderEndpoint(const SimpleQueueAttr &attr) {
        Sync::AutoReentrantLock arlock(lock);
        ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    context,
                    RemoteQueue::READ,
                    server.get(),
                    remotequeueholder.get(),
                    attr
                    ));


        NodeMap::iterator entry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        shared_ptr<PseudoNode> node = entry->second;
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
                    context,
                    RemoteQueue::WRITE,
                    server.get(),
                    remotequeueholder.get(),
                    attr
                    ));

        NodeMap::iterator entry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        shared_ptr<PseudoNode> node = entry->second;
        remotequeueholder->AddQueue(endp);
        endp->Start();
        arlock.Unlock();
        node->CreateWriter(endp);
    }

    void Kernel::CreateLocalQueue(const SimpleQueueAttr &attr) {
        shared_ptr<QueueBase> queue;
        queue = shared_ptr<QueueBase>(new ThresholdQueue(context, attr));

        Sync::AutoReentrantLock arlock(lock);
        NodeMap::iterator readentry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(readentry != nodemap.end(), "Tried to connect a queue to a node that doesn't exist.");
        shared_ptr<PseudoNode> readnode = readentry->second;

        NodeMap::iterator writeentry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(writeentry != nodemap.end(), "Tried to connect a queue to a node that doesn't exist.");
        shared_ptr<PseudoNode> writenode = writeentry->second;
        arlock.Unlock();

        writenode->CreateWriter(queue);
        readnode->CreateReader(queue);
    }

    void Kernel::InternalCreateNode(NodeAttr &nodeattr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(status.Get() == RUNNING);
        NodeFactory *factory = context->GetNodeFactory(nodeattr.GetTypeName());
        if (!factory) {
            throw std::invalid_argument("No such node type " + nodeattr.GetTypeName());
        }
        shared_ptr<NodeBase> node = factory->Create(*this, nodeattr);
        nodemap.insert(std::make_pair(nodeattr.GetKey(), node));
        node->Start();
    }

    void Kernel::NodeTerminated(Key_t key) {
        context->SignalNodeEnd(key);
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        if (status.Get() == DONE) {
            logger.Warn("Nodes running after shutdown");
        } else {
            NodeMap::iterator entry = nodemap.find(key);
            ASSERT(entry != nodemap.end());
            shared_ptr<PseudoNode> node = entry->second;
            nodemap.erase(entry);
            garbagenodes.push_back(node);
            SendWakeup();
        }
        cond.Signal();
    }

    void Kernel::ClearGarbage() {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        while (!garbagenodes.empty()) {
            garbagenodes.back()->Shutdown();
            garbagenodes.pop_back();
        }
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
            context->SignalHostStart(hostkey);
            if (useremote) {
                while (status.Get() == RUNNING) {
                    ClearGarbage();
                    remotequeueholder->Cleanup();
                    server->Poll();
                }
            } else {
                Sync::AutoReentrantLock arlock(lock);
                while (status.Get() == RUNNING) {
                    ClearGarbage();
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
                ClearGarbage();
                cond.Wait(lock);
            }
        }
        ClearGarbage();
        context->SignalHostEnd(hostkey);
        status.Post(DONE);
        FUNCEND;
        return 0;
    }

    void Kernel::RemoteCreateWriter(SimpleQueueAttr attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        CreateWriterEndpoint(attr);
    }
    void Kernel::RemoteCreateReader(SimpleQueueAttr attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        CreateReaderEndpoint(attr);
    }
    void Kernel::RemoteCreateQueue(SimpleQueueAttr attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        CreateLocalQueue(attr);
    }
    void Kernel::RemoteCreateNode(NodeAttr attr) {
        Sync::AutoReentrantLock arlock(lock);
        FUNCBEGIN;
        ASSERT(useremote);
        InternalCreateNode(attr);
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


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
        : status(INITIALIZED),
        kernelname(kattr.GetName()),
        hostkey(0),
        context(kattr.GetContext()),
        useremote(kattr.GetRemoteEnabled()),
        nodecond_signal(false),
        useD4R(kattr.UseD4R()),
        swallowbrokenqueue(kattr.SwallowBrokenQueueExceptions()),
        growmaxthresh(kattr.GrowQueueMaxThreshold())
    {
        FUNCBEGIN;
        nodeloader.LoadSharedLib(kattr.GetSharedLibs());
        nodeloader.LoadNodeList(kattr.GetNodeLists());
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

    bool Kernel::IsTerminated() {
        return context->IsTerminated();
    }

    void Kernel::CheckTerminated() {
        context->CheckTerminated();
    }

    void Kernel::NotifyTerminate() {
        FUNCBEGIN;
        if (status.CompareAndPost(RUNNING, TERMINATE)) {
            SendWakeup();
        }
    }

    Key_t Kernel::CreateNode(const NodeAttr &attr) {
        FUNCBEGIN;

        NodeAttr nodeattr = attr;

        if (nodeattr.GetHostKey() == 0) {
            Key_t key = 0;
            if (nodeattr.GetHost().empty()) {
                key = hostkey;
            } else {
                key = context->WaitForHostStart(nodeattr.GetHost());
            }
            nodeattr.SetHostKey(key);
        }
        Key_t nodekey = context->CreateNodeKey(nodeattr.GetHostKey(), nodeattr.GetName());
        nodeattr.SetKey(nodekey);

        // check the host the node should go on and send
        // to that particular host
        if (nodeattr.GetHostKey() == hostkey) {
            InternalCreateNode(nodeattr);
        } else {
            context->SendCreateNode(nodeattr.GetHostKey(), nodeattr);
        }
        return nodekey;
    }

    class ExternalEndpoint : public PseudoNode {
    public:
        ExternalEndpoint(const std::string &name, Key_t nodekey, shared_ptr<Context> context,
                bool iswriter_)
            : PseudoNode(name, nodekey, context), iswriter(iswriter_) {}
        virtual ~ExternalEndpoint() {}

        bool IsWriter() const { return iswriter; }
    private:
        bool iswriter;
    };

    void Kernel::CreateExternalReader(const std::string &name) {
        CreateExternalEndpoint(name, false);
    }

    void Kernel::CreateExternalWriter(const std::string &name) {
        CreateExternalEndpoint(name, true);
    }

    void Kernel::CreateExternalEndpoint(const std::string &name, bool iswriter) {
        Key_t nodekey = context->CreateNodeKey(hostkey, name);
        shared_ptr<PseudoNode> pnode;
        pnode.reset(new ExternalEndpoint(name, nodekey, context, iswriter));
        Sync::AutoReentrantLock arlock(nodelock);
        nodemap.insert(std::make_pair(nodekey, pnode));
        arlock.Unlock();
        context->SignalNodeStart(nodekey);
    }

    shared_ptr<QueueWriter> Kernel::GetExternalOQueue(const std::string &name) {
        Key_t key = context->GetNodeKey(name);
        Sync::AutoReentrantLock arlock(nodelock);
        NodeMap::iterator entry = nodemap.find(key);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid external reader.");
        }
        shared_ptr<ExternalEndpoint> pnode = dynamic_pointer_cast<ExternalEndpoint>(entry->second);
        arlock.Unlock();
        if (pnode && pnode->IsWriter()) {
            return pnode->GetOQueue(name);
        }
        throw std::invalid_argument("Not a valid external reader.");
    }

    shared_ptr<QueueReader> Kernel::GetExternalIQueue(const std::string &name) {
        Key_t key = context->GetNodeKey(name);
        Sync::AutoReentrantLock arlock(nodelock);
        NodeMap::iterator entry = nodemap.find(key);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid external writer.");
        }
        shared_ptr<ExternalEndpoint> pnode = dynamic_pointer_cast<ExternalEndpoint>(entry->second);
        arlock.Unlock();
        if (pnode && !pnode->IsWriter()) {
            return pnode->GetIQueue(name);
        }
        throw std::invalid_argument("Not a valid external writer.");
    }

    void Kernel::DestroyExternalEndpoint(const std::string &name) {
        Key_t key = context->GetNodeKey(name);
        Sync::AutoReentrantLock arlock(nodelock);
        NodeMap::iterator entry = nodemap.find(key);
        if (entry == nodemap.end() || !entry->second->IsPurePseudo()) {
            throw std::invalid_argument("Not a valid external endpoint.");
        }
        arlock.Unlock();
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

        context->ConnectEndpoints(attr.GetWriterKey(), attr.GetReaderKey(), qattr.GetName());

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
        ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    this,
                    RemoteQueue::READ,
                    server.get(),
                    remotequeueholder.get(),
                    attr
                    ));


        Sync::AutoReentrantLock arlock(nodelock);
        NodeMap::iterator entry = nodemap.find(attr.GetReaderNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        shared_ptr<PseudoNode> node = entry->second;
        arlock.Unlock();
        remotequeueholder->AddQueue(endp);
        endp->Start();
        node->CreateReader(endp);
    }

    void Kernel::CreateWriterEndpoint(const SimpleQueueAttr &attr) {
        ASSERT(useremote, "Cannot create remote queue without enabling remote operations.");

        shared_ptr<RemoteQueue> endp;
        endp = shared_ptr<RemoteQueue>(
                new RemoteQueue(
                    this,
                    RemoteQueue::WRITE,
                    server.get(),
                    remotequeueholder.get(),
                    attr
                    ));

        Sync::AutoReentrantLock arlock(nodelock);
        NodeMap::iterator entry = nodemap.find(attr.GetWriterNodeKey());
        ASSERT(entry != nodemap.end(), "Node not found!?");
        shared_ptr<PseudoNode> node = entry->second;
        arlock.Unlock();
        remotequeueholder->AddQueue(endp);
        endp->Start();
        node->CreateWriter(endp);
    }

    void Kernel::CreateLocalQueue(const SimpleQueueAttr &attr) {
        shared_ptr<QueueBase> queue;
        queue = shared_ptr<QueueBase>(new ThresholdQueue(this, attr));

        Sync::AutoReentrantLock arlock(nodelock);
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
        Sync::AutoReentrantLock arlock(nodelock);
        FUNCBEGIN;
        ASSERT(status.Get() == RUNNING);
        NodeFactory *factory = GetNodeFactory(nodeattr.GetTypeName());
        if (!factory) {
            throw std::invalid_argument("No such node type " + nodeattr.GetTypeName());
        }
        shared_ptr<NodeBase> node = factory->Create(*this, nodeattr);
        nodemap.insert(std::make_pair(nodeattr.GetKey(), node));
        node->Start();
    }

    void Kernel::NodeTerminated(Key_t key) {
        context->SignalNodeEnd(key);
        Sync::AutoReentrantLock arlock(nodelock);
        FUNCBEGIN;
        if (status.Get() == DONE) {
            logger.Warn("Nodes running after shutdown");
        } else {
            NodeMap::iterator entry = nodemap.find(key);
            ASSERT(entry != nodemap.end());
            shared_ptr<PseudoNode> node = entry->second;
            nodemap.erase(entry);
            {
                Sync::AutoReentrantLock arlock(garbagelock);
                garbagenodes.push_back(node);
            }
            SendWakeup();
        }
        nodecond.Signal();
        nodecond_signal = true;
    }

    void Kernel::ClearGarbage() {
        Sync::AutoReentrantLock arlock(garbagelock);
        FUNCBEGIN;
        while (!garbagenodes.empty()) {
            garbagenodes.back()->Shutdown();
            garbagenodes.pop_back();
        }
    }

    void Kernel::SendWakeup() {
        Sync::AutoReentrantLock arlock(nodelock);
        if (useremote) {
            server->Wakeup();
        }
        nodecond.Signal();
        nodecond_signal = true;
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
                ClearGarbage();
                Sync::AutoReentrantLock arlock(nodelock);
                while (status.Get() == RUNNING) {
                    if (!nodecond_signal) {
                        nodecond.Wait(nodelock);
                        nodecond_signal = false;
                    }
                    arlock.Unlock();
                    ClearGarbage();
                    arlock.Lock();
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
            Sync::AutoReentrantLock arlock(nodelock);
            NodeMap mapcopy = nodemap;
            arlock.Unlock();
            NodeMap::iterator nitr = mapcopy.begin();
            while (nitr != mapcopy.end()) {
                (nitr++)->second->NotifyTerminate();
            }
            ClearGarbage();
            arlock.Lock();
            // Wait for all nodes to end
            while (!nodemap.empty()) {
                if (!nodecond_signal) {
                    nodecond.Wait(nodelock);
                    nodecond_signal = false;
                }
                arlock.Unlock();
                ClearGarbage();
                arlock.Lock();
            }
        }
        ClearGarbage();
        context->SignalHostEnd(hostkey);
        status.Post(DONE);
        FUNCEND;
        return 0;
    }

    void Kernel::RemoteCreateWriter(SimpleQueueAttr attr) {
        FUNCBEGIN;
        ASSERT(useremote);
        CreateWriterEndpoint(attr);
    }
    void Kernel::RemoteCreateReader(SimpleQueueAttr attr) {
        FUNCBEGIN;
        ASSERT(useremote);
        CreateReaderEndpoint(attr);
    }
    void Kernel::RemoteCreateQueue(SimpleQueueAttr attr) {
        FUNCBEGIN;
        ASSERT(useremote);
        CreateLocalQueue(attr);
    }
    void Kernel::RemoteCreateNode(NodeAttr attr) {
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


    bool Kernel::UseD4R() {
        Sync::AutoReentrantLock al(datalock);
        return useD4R;
    }

    bool Kernel::UseD4R(bool u) {
        Sync::AutoReentrantLock al(datalock);
        return useD4R = u;
    }

    bool Kernel::SwallowBrokenQueueExceptions() {
        Sync::AutoReentrantLock al(datalock);
        return swallowbrokenqueue;
    }

    bool Kernel::SwallowBrokenQueueExceptions(bool sbqe) {
        Sync::AutoReentrantLock al(datalock);
        return swallowbrokenqueue = sbqe;
    }

    bool Kernel::GrowQueueMaxThreshold() {
        Sync::AutoReentrantLock al(datalock);
        return growmaxthresh;
    }

    bool Kernel::GrowQueueMaxThreshold(bool grow) {
        Sync::AutoReentrantLock al(datalock);
        return growmaxthresh = grow;
    }

}


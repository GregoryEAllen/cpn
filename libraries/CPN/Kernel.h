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
 * \brief Definition for the kernel object.
 * \author John Bridgman
 */

#ifndef CPN_KERNEL_H
#define CPN_KERNEL_H
#pragma once

#include "CPNCommon.h"
#include "KernelAttr.h"
#include "NodeAttr.h"
#include "QueueAttr.h"

#include "Message.h"

#include "ReentrantLock.h"
#include "StatusHandler.h"

#include "ListenSockHandler.h"
#include "WakeupHandler.h"
#include "KernelConnectionHandler.h"

#include "Logger.h"

#include "Pthread.h"

#include <string>
#include <map>
#include <vector>
#include <deque>
#include <list>

namespace CPN {

    /**
     * \brief The Kernel declaration.
     *
     * The purpose of the kernel object is to keep track
     * of all the queues and nodes on a particular host,
     * ensure that they are instantiated and destroyed
     * correctly and to provide a unified interface to
     * the user of the process network.
     */
    class CPN_API Kernel : private Pthread, private KernelMessageHandler {
        enum KernelStatus_t {
            INITIALIZED, RUNNING, TERMINATE, DONE
        };
    public:

        /**
         * Construct a new kernel object with the given attributes.
         * The Kernel starts processing immediately.
         */
        Kernel(const KernelAttr &kattr);
        ~Kernel();

        /**
         * Waits until the main loop terminates. Will not happen until
         * a terminate signal or Terminate is called.
         * Use WaitNodeTerminate if you wish to wait for the nodes to be
         * done.
         */
        void Wait();

        /**
         * For the kernel to terminate.
         * Returns immediately, use wait to wait
         * for completion.
         */
        void Terminate();

        /**
         * Create a new node.
         *
         * The memory pointed to by argsize may be copied up to
         * argsize bytes into a location that the new node can access.
         *
         * \param attr The NodeAttr that describes the new node to create.
         * \return the new node's key
         * \throws KernelShutdownException if Wait completed or Terminate
         * has been called.
         */
        Key_t CreateNode(const NodeAttr &attr);

        /**
         * Check for the existance of the given node. Returns
         * when it detects that the given node does not exist.
         * \param nodename the name of the node to wait for
         * \throws ShutdownException if the kernel has shutdown
         */
        void WaitNodeTerminate(const std::string &nodename);
        void WaitNodeStart(const std::string &nodename);

        // Attempt to connect the given endpoints together using the given attributes
        // If the endpoints are of the wrong type
        // this function will fail. Only one side has to free the connection for
        // the other port to become disconnected and available.
        // If the endpoints do not exist create them. If
        // the endpoints are already connected fail.
        void CreateQueue(const QueueAttr &attr);
       
        const LoggerOutput *GetLogger() const { return &logger; }

        /** 
         * \return the name of this kernel.
         */
        const std::string &GetName() const { return kernelname; }

        Key_t GetKey() const { return hostkey; }

        shared_ptr<Database> GetDatabase() const { return database; }

        void NodeTerminated(Key_t key);
    private:
        // Not copyable
        Kernel(const Kernel&);
        Kernel &operator=(const Kernel&);

        void CreateReaderEndpoint(const SimpleQueueAttr &attr);
        void CreateWriterEndpoint(const SimpleQueueAttr &attr);
        void CreateLocalQueue(const SimpleQueueAttr &attr);
        shared_ptr<QueueBase> MakeQueue(const SimpleQueueAttr &attr);
        void InternalCreateNode(NodeAttr &nodeattr);
        void ClearGarbage();

        void *EntryPoint();

        void Poll(double timeout);
        void SendWakeup() { wakeuphandler.SendWakeup(); }

        void CreateWriter(Key_t dst, const SimpleQueueAttr &attr);
        void CreateReader(Key_t dst, const SimpleQueueAttr &attr);
        void CreateQueue(Key_t dst, const SimpleQueueAttr &attr);
        void CreateNode(Key_t dst, const NodeAttr &attr);

        shared_ptr<Future<int> > GetReaderDescriptor(Key_t readerkey, Key_t writerkey);
        shared_ptr<Future<int> > GetWriterDescriptor(Key_t readerkey, Key_t writerkey);

        void LogEndpoints();

        Sync::ReentrantLock lock;
        Sync::StatusHandler<KernelStatus_t> status;
        const std::string kernelname;
        Key_t hostkey;
        Logger logger;
        shared_ptr<Database> database;

        WakeupHandler wakeuphandler;
        KernelConnectionHandler connhandler;

        typedef std::map<Key_t, shared_ptr<NodeBase> > NodeMap;
        typedef std::vector< shared_ptr<NodeBase> > NodeList;
        NodeMap nodemap;
        NodeList garbagenodes;

        typedef std::vector<shared_ptr<SocketEndpoint> > EndpointList;
        EndpointList endpoints;
    };
}
#endif

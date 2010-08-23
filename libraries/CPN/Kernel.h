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

#include "KernelBase.h"
#include "ConnectionServer.h"
#include "RemoteQueueHolder.h"

#include "ReentrantLock.h"
#include "StatusHandler.h"

#include "Logger.h"

#include "Pthread.h"

#include <string>
#include <map>
#include <vector>

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
    class CPN_API Kernel : public KernelBase {
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
        /** \brief Waits until there are no running nodes.
         */
        void WaitForAllNodeEnd();
        /** \brief Wait for the given node to start
         * \param nodename the name of the node to wait for
         */
        void WaitNodeStart(const std::string &nodename);

        /** \brief Create a new queue
         * \param attr the attribute to use to create the queu
         * \see QueueAttr
         */
        void CreateQueue(const QueueAttr &attr);
       
        LoggerOutput *GetLogger() { return &logger; }

        /** 
         * \return the name of this kernel.
         */
        const std::string &GetName() const { return kernelname; }

        /** \return the unique key for this kernel
         */
        Key_t GetKey() const { return hostkey; }

        /** \return the database this kernel is using
         */
        shared_ptr<Database> GetDatabase() const { return database; }

        /** \brief Called by the node in the cleanup routine.
         */
        void NodeTerminated(Key_t key);

        /**
         * These functions are called by the database to create things
         * remotely. Should not be called by anyone but the database!
         * \{
         */
        void NotifyTerminate();
        void RemoteCreateWriter(SimpleQueueAttr attr);
        void RemoteCreateReader(SimpleQueueAttr attr);
        void RemoteCreateQueue(SimpleQueueAttr attr);
        void RemoteCreateNode(NodeAttr attr);
        /**
         * \}
         */

    private:
        // Not copyable
        Kernel(const Kernel&);
        Kernel &operator=(const Kernel&);

        void CreateReaderEndpoint(const SimpleQueueAttr &attr);
        void CreateWriterEndpoint(const SimpleQueueAttr &attr);
        void CreateLocalQueue(const SimpleQueueAttr &attr);
        void InternalCreateNode(NodeAttr &nodeattr);
        void ClearGarbage();

        void *EntryPoint();

        void SendWakeup();

        // Function to be called from gdb
        void LogState();

        Sync::ReentrantLock lock;
        Sync::StatusHandler<KernelStatus_t> status;
        Sync::ReentrantCondition cond;
        auto_ptr<Pthread> thread;
        const std::string kernelname;
        Key_t hostkey;
        Logger logger;
        shared_ptr<Database> database;
        auto_ptr<ConnectionServer> server;
        auto_ptr<RemoteQueueHolder> remotequeueholder;
        bool useremote;

        typedef std::map<Key_t, shared_ptr<NodeBase> > NodeMap;
        typedef std::vector< shared_ptr<NodeBase> > NodeList;
        NodeMap nodemap;
        NodeList garbagenodes;
    };
}
#endif

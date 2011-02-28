//=============================================================================
//  Computational Process Networks class library
//  Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//  This library is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  The GNU Public License is available in the file LICENSE, or you
//  can write to the Free Software Foundation, Inc., 59 Temple Place -
//  Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//  World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \brief The Context abstract data type.
 * \author John Bridgman
 */
#ifndef CPNDB_CONTEXT_H
#define CPNDB_CONTEXT_H
#pragma once
#include "CPNCommon.h"
#include "Logger.h"
#include <string>




namespace CPN {

    /**
     * \brief The CPN::Context abstraction that holds all the global state
     * for the process network.
     *
     * All methods may through a CPN::ShutdownException.
     *
     * Note that all key values should be unique across all objects.
     * That is to say that even though a node and a kernel are of different types
     * none of there keys should ever be equal.
     *
     * Note that the only function here that should be called outside
     * of the library (i.e. by the user of the library) are the log level functions
     * the Get functions and the Wait functions and the Terminate functions.
     */
    class CPN_API Context : public LoggerOutput {
    public:

        /** \brief Create a local context.
         * \return a new local context.
         */
        static shared_ptr<Context> Local();

        virtual ~Context();

        /** \brief Called by the Kernel when it has successfully set it self up.
         * This gives the Context a way to notify the Kernel of events and
         * lets other Kernels look up the connection information for this Kernel.
         * \param name the kernel name
         * \param hostname the hostname to use to connect to this kernel
         * \param servname the service name the kernel is listening on
         * \param kernel callback reference
         * \return the unique key for the new kernel
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t SetupHost(const std::string &name, const std::string &hostname,
                const std::string &servname, KernelBase *kernel) = 0;
        /** \brief Called by the kernel when it is not in remote mode.
         * \param name the kernel name
         * \param kernel callback reference
         * \return the unique key for the new kernel
         */
        virtual Key_t SetupHost(const std::string &name, KernelBase *kernel) = 0;
        /** \param host the name of the host
         * \return the key for the given hostname.
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetHostKey(const std::string &host) = 0;
        /** \param hostkey the key to the host
         * \return the name for the host
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual std::string GetHostName(Key_t hostkey) = 0;
        /** \brief obtain the connection information for the given host
         * \param hostkey the unique id for the host
         * \param hostname (output) string to be filled with the hostname
         * \param servname (output) string to be filled with the service name
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual void GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname) = 0;
        /** \brief Signal to the Context that the given host is dead.
         * \param hostkey id of the host that died
         * \throw std::invalid_argument
         */
        virtual void SignalHostEnd(Key_t hostkey) = 0;
        /** \brief Does not return until the given host has started.
         * \param host the name of the host (the key may not be known yet)
         * \throw ShutdownException
         */
        virtual Key_t WaitForHostStart(const std::string &host) = 0;
        /** \brief Signal to the context that the given host has started
         * \param hostkey the id for the host
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual void SignalHostStart(Key_t hostkey) = 0;

        /** \brief Tell a given host that it needs to create a queue write end.
         * \param hostkey the id of the host
         * \param attr the queue attribute
         * \throw ShutdownException
         */
        virtual void SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr) = 0;
        /** \brief Tell a given host that it needs to create a queue read end.
         * \param hostkey the id of the host
         * \param attr the queue attribute
         * \throw ShutdownException
         */
        virtual void SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr) = 0;
        /** \brief Tell a given host that it needs to create a queue.
         * \param hostkey the id of the host
         * \param attr the queue attribute
         * \throw ShutdownException
         */
        virtual void SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr) = 0;
        /** \brief Tell a given host that it needs to create a node.
         * \param hostkey the id of the host
         * \param attr the node attribute
         * \throw ShutdownException
         */
        virtual void SendCreateNode(Key_t hostkey, const NodeAttr &attr) = 0;

        /** \brief Tell the context to allocate a new node key and data structure for
         * a node with nodename which is on hostkey.
         * \param hostkey the id of the kernel that the node will run on
         * \param nodename the name of the node
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t CreateNodeKey(Key_t hostkey, const std::string &nodename) = 0;
        /** \return the unique key associated with the given node name.
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetNodeKey(const std::string &nodename) = 0;
        /** \return the name associated with the given node key
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual std::string GetNodeName(Key_t nodekey) = 0;
        /** \param nodekey the unique key for the node
         * \return the key for the host the node is running on
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetNodeHost(Key_t nodekey) = 0;
        /** \brief Called by the node startup routine to indicate that the node has started.
         * \param nodekey the unique key for the node
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual void SignalNodeStart(Key_t nodekey) = 0;
        /** \brief Called by the node cleanup routine to indicate that the node has ended.
         * \param nodekey the unique key for the node
         * \throw std::invalid_argument
         */
        virtual void SignalNodeEnd(Key_t nodekey) = 0;

        /** \brief Waits until the node starts and returns the key, if the node is
         * already started returns the key
         * \param nodename the name of the node to wait for
         * \return the key for the node
         * \throw ShutdownException
         */
        virtual Key_t WaitForNodeStart(const std::string &nodename) = 0;
        /** \brief Waits for the given node to signal end
         * \param nodename the name of the node
         * \throw ShutdownException
         */
        virtual void WaitForNodeEnd(const std::string &nodename) = 0;
        /** \brief Convenience method which waits until there are no
         * nodes running. If no node have started then this will return immediately.
         * \throw ShutdownException
         */
        virtual void WaitForAllNodeEnd() = 0;


        /** \brief Get the key associated with the given endpoint for the given node.
         * Creates the information if it does not exist
         * \param nodekey the unique id for the node
         * \param portname the name of the endpoint.
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetCreateReaderKey(Key_t nodekey, const std::string &portname) = 0;
        /** \param portkey the unique id for the port
         * \return the key for the node this port is on
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetReaderNode(Key_t portkey) = 0;
        /** \param portkey the unique id for the port
         * \return the key for the host this port is on
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetReaderHost(Key_t portkey) = 0;
        /** \param portkey the unique id for the port
         * \return the name of the port
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual std::string GetReaderName(Key_t portkey) = 0;

        /** \see GetCreateReaderKey */
        virtual Key_t GetCreateWriterKey(Key_t nodekey, const std::string &portname) = 0;
        /** \see GetReaderNode */
        virtual Key_t GetWriterNode(Key_t portkey) = 0;
        /** \see GetReaderHost */
        virtual Key_t GetWriterHost(Key_t portkey) = 0;
        /** \see GetReaderName */
        virtual std::string GetWriterName(Key_t portkey) = 0;

        /** \brief Called by the kernel when a queue is created.
         * Note that the endpoints may have been created when the node requests them but
         * the queue may be created long after that.
         * \param writerkey the unique key for the writer endpoint
         * \param readerkey the unique key for the reader endpoint
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual void ConnectEndpoints(Key_t writerkey, Key_t readerkey, const std::string &qname) = 0;
        /** \param readerkey a unique reader key
         * \return the writer key associated with this reader key if there is one
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetReadersWriter(Key_t readerkey) = 0;
        /** \param writerkey a unique writer key
         * \return the reader key associated with this writer
         * \throw ShutdownException
         * \throw std::invalid_argument
         */
        virtual Key_t GetWritersReader(Key_t writerkey) = 0;

        /** \brief Signal to the Context that the network is terminating.
         * After this call most methods will throw a ShutdownException
         */
        virtual void Terminate() = 0;
        /** \return true if Terminate has been called
         */
        virtual bool IsTerminated() = 0;

        /** \brief Convenience method that checks IsTerminated and
         * if so throws a ShutdownException
         * \throw ShutdownException
         */
        void CheckTerminated();

        /** \brief Lets the kernel know that this context type requires remote activity.
         * This overrides the kernel option for remote activity.
         * Default value is false
         * \return true or false
         */
        virtual bool RequireRemote();

        /** \brief Calculate the new queue size when a queue needs to grow.
         * \return the new queue size
         */
        virtual unsigned CalculateGrowSize(unsigned currentsize, unsigned request) { return currentsize + request; }

    protected:
        Context();

    };

}

#endif


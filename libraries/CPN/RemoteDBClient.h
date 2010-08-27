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
#ifndef CPN_REMOTEDBCLIENT_H
#define CPN_REMOTEDBCLIENT_H
#pragma once

#include "RDBMT.h"
#include "Database.h"
#include "QueueAttr.h"
#include "NodeAttr.h"
#include "Pthread.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "Variant.h"
#include <list>
#include <tr1/memory>

namespace CPN {

    /**
     * \brief This abstract class is the remote database client.
     * 
     * An implementor only needs to override SendMessage to send the necessary
     * messages to the RemoteDBServer and then call DispatchMessage for the
     * replies from the server.
     */
    class RemoteDBClient : public Database {
    public:
        virtual ~RemoteDBClient();

        virtual int LogLevel() const;
        virtual int LogLevel(int level);
        virtual void Log(int level, const std::string &logmsg);

        virtual CPN::Key_t SetupHost(const std::string &name, const std::string &hostname,
                const std::string &servname, CPN::KernelBase *kernel);
        virtual CPN::Key_t SetupHost(const std::string &name, KernelBase *kernel) { ASSERT(false, "RemoteDBClient requires remote operation is turned on."); }
        virtual CPN::Key_t GetHostKey(const std::string &host);
        virtual std::string GetHostName(CPN::Key_t hostkey);
        virtual void GetHostConnectionInfo(CPN::Key_t hostkey, std::string &hostname, std::string &servname);
        virtual CPN::Key_t WaitForHostStart(const std::string &host);
        virtual void SignalHostStart(CPN::Key_t hostkey);
        virtual void SignalHostEnd(CPN::Key_t hostkey);

        virtual void SendCreateWriter(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr);
        virtual void SendCreateReader(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr);
        virtual void SendCreateQueue(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr);
        virtual void SendCreateNode(CPN::Key_t hostkey, const CPN::NodeAttr &attr);

        virtual CPN::Key_t CreateNodeKey(CPN::Key_t hostkey, const std::string &nodename);
        virtual CPN::Key_t GetNodeKey(const std::string &nodename);
        virtual std::string GetNodeName(CPN::Key_t nodekey);
        virtual CPN::Key_t GetNodeHost(CPN::Key_t nodekey);
        virtual void SignalNodeStart(CPN::Key_t nodekey);
        virtual void SignalNodeEnd(CPN::Key_t nodekey);

        virtual CPN::Key_t WaitForNodeStart(const std::string &nodename);
        virtual void WaitForNodeEnd(const std::string &nodename);
        virtual void WaitForAllNodeEnd();


        virtual CPN::Key_t GetCreateReaderKey(CPN::Key_t nodekey, const std::string &portname);
        virtual CPN::Key_t GetReaderNode(CPN::Key_t portkey);
        virtual CPN::Key_t GetReaderHost(CPN::Key_t portkey);
        virtual std::string GetReaderName(CPN::Key_t portkey);

        virtual CPN::Key_t GetCreateWriterKey(CPN::Key_t nodekey, const std::string &portname);
        virtual CPN::Key_t GetWriterNode(CPN::Key_t portkey);
        virtual CPN::Key_t GetWriterHost(CPN::Key_t portkey);
        virtual std::string GetWriterName(CPN::Key_t portkey);

        virtual void ConnectEndpoints(CPN::Key_t writerkey, CPN::Key_t readerkey);
        virtual CPN::Key_t GetReadersWriter(CPN::Key_t readerkey);
        virtual CPN::Key_t GetWritersReader(CPN::Key_t writerkey);

        virtual void Terminate();
        virtual bool IsTerminated();

        virtual bool RequireRemote();

        /**
         * Handle a message from the RemoteDBServer.
         * \param msg the message
         */
        void DispatchMessage(const Variant &msg);
    protected:
        RemoteDBClient();
        /**
         * Called by the functions to send a message to the
         * RemoteDBServer.
         * \param msg the message
         */
        virtual void SendMessage(const Variant &msg) = 0;
    private:

        /**
         * Struct to hold information for a call to the RemoteDBServer
         * which returns a result. This is used by RemoteCall.
         */
        struct WaiterInfo {
            WaiterInfo(unsigned wid) : waiterid(wid), signaled(false) {}
            unsigned waiterid;
            PthreadCondition cond;
            bool signaled;
            Variant msg;
        };

        /**
         * Struct used to hold information for when we wish to
         * listen to all incoming broadcasts.
         */
        struct GenericWaiter {
            PthreadCondition cond;
            std::list<Variant> messages;
        };
        typedef std::tr1::shared_ptr<GenericWaiter> GenericWaiterPtr;

        void SendQueueMsg(CPN::Key_t hostkey, RDBMT_t msgtype, const CPN::SimpleQueueAttr &attr);
        unsigned NewTranID();
        void AddWaiter(WaiterInfo *info);
        GenericWaiterPtr NewGenericWaiter();
        void InternalTerminate();
        void InternalCheckTerminated();
        void *TerminateThread();
        Variant RemoteCall(Variant msg);

        CPN::Key_t GetCreateEndpointKey(RDBMT_t msgtype, CPN::Key_t nodekey, const std::string &portname);
        Variant GetEndpointInfo(RDBMT_t msgtype, CPN::Key_t portkey);

        auto_ptr<Pthread> terminateThread;
        typedef std::map<unsigned, WaiterInfo*> WaiterMap;
        WaiterMap callwaiters;
        typedef std::list<std::tr1::weak_ptr<GenericWaiter> > WaiterList;
        WaiterList waiters;
        typedef std::map<CPN::Key_t, CPN::KernelBase*> KernelMap;
        KernelMap kernels;
        unsigned trancounter;
        bool shutdown;
        int loglevel;
    };
}
#endif

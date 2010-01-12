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
 * \brief An implemenation of Database for a local process specific
 * database
 * \author John Bridgman
 */

#ifndef CPN_LOCALDATABASE_H
#define CPN_LOCALDATABASE_H

#include "CPNCommon.h"
#include "Database.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <string>
#include <map>

namespace CPN {

    class LocalDatabase : public Database {
    public:
        typedef std::map< std::string, Key_t > NameMap;

        struct HostInfo {
            std::string name;
            std::string hostname;
            std::string servname;
            KernelBase *kmh;
            bool live;
            bool dead;
        };

        struct NodeInfo {
            std::string name;
            Key_t hostkey;
            bool started;
            NameMap readers;
            NameMap writers;
            bool dead;
        };

        struct PortInfo {
            std::string name;
            Key_t nodekey;
            Key_t opposingport;
            bool dead;
        };

        typedef std::map< Key_t, shared_ptr<HostInfo> > HostMap;
        typedef std::map< Key_t, shared_ptr<NodeInfo> > NodeMap;
        typedef std::map< Key_t, shared_ptr<PortInfo> > PortMap;

        LocalDatabase();
        virtual ~LocalDatabase();

        virtual void Log(int level, const std::string &msg);
        virtual int LogLevel() const;
        virtual int LogLevel(int level);

        virtual Key_t SetupHost(const std::string &name, const std::string &hostname,
                const std::string &servname, KernelBase *kmh);
        virtual Key_t GetHostKey(const std::string &host);
        virtual std::string GetHostName(Key_t hostkey);
        virtual void GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname);
        virtual void DestroyHostKey(Key_t hostkey);
        virtual Key_t WaitForHostStart(const std::string &host);
        virtual void SignalHostStart(Key_t hostkey);

        virtual void SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr);
        virtual void SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr);
        virtual void SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr);
        virtual void SendCreateNode(Key_t hostkey, const NodeAttr &attr);

        virtual Key_t CreateNodeKey(Key_t hostkey, const std::string &nodename);
        virtual Key_t GetNodeKey(const std::string &nodename);
        virtual std::string GetNodeName(Key_t nodekey);
        virtual Key_t GetNodeHost(Key_t nodekey);
        virtual void SignalNodeStart(Key_t nodekey);
        virtual void SignalNodeEnd(Key_t nodekey);
        virtual Key_t WaitForNodeStart(const std::string &nodename);
        virtual void WaitForNodeEnd(const std::string &nodename);
        virtual void WaitForAllNodeEnd();

        virtual Key_t GetCreateReaderKey(Key_t nodekey, const std::string &portname);
        virtual Key_t GetReaderNode(Key_t portkey);
        virtual Key_t GetReaderHost(Key_t portkey);
        virtual std::string GetReaderName(Key_t portkey);
        virtual void DestroyReaderKey(Key_t portkey);

        virtual Key_t GetCreateWriterKey(Key_t nodekey, const std::string &portname);
        virtual Key_t GetWriterNode(Key_t portkey);
        virtual Key_t GetWriterHost(Key_t portkey);
        virtual std::string GetWriterName(Key_t portkey);
        virtual void DestroyWriterKey(Key_t portkey);

        virtual void ConnectEndpoints(Key_t writerkey, Key_t readerkey);
        virtual Key_t GetReadersWriter(Key_t readerkey);
        virtual Key_t GetWritersReader(Key_t writerkey);

        virtual void Terminate();
        virtual bool IsTerminated();
    private:

        void InternalCheckTerminated();

        int loglevel;
        mutable PthreadMutex lock;
        PthreadCondition nodelivedead;
        PthreadCondition hostlivedead;
        NameMap hostnames;
        NameMap nodenames;
        HostMap hostmap;
        NodeMap nodemap;
        PortMap readports;
        PortMap writeports;
        unsigned numlivenodes;
        bool shutdown;

        Key_t NewKey() { return ++counter; }
        Key_t counter;
    };

}

#endif


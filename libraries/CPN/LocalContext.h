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
 * \brief An implemenation of Context for a local process specific
 * context
 * \author John Bridgman
 */

#ifndef CPN_LOCALCONTEXT_H
#define CPN_LOCALCONTEXT_H
#pragma once

#include "CPNCommon.h"
#include "Context.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <string>
#include <map>

namespace CPN {

    /** \brief A local implementation of the Context interface
     */
    class CPN_LOCAL LocalContext : public Context {
    public:
        typedef std::map< std::string, Key_t > NameMap;

        /** Struct to hold kernel information. */
        struct KernelInfo {
            std::string name;
            std::string hostname;
            std::string servname;
            KernelBase *kernel;
            bool live;
            bool dead;
            bool allowremote;
        };

        /** Struct to hold the node information */
        struct NodeInfo {
            std::string name;
            Key_t kernelkey;
            bool started;
            NameMap readers;
            NameMap writers;
            bool dead;
        };

        /** Struct to hold the endpoint information */
        struct PortInfo {
            std::string name;
            std::string qname;
            Key_t nodekey;
            Key_t opposingport;
            bool dead;
        };

        typedef std::map< Key_t, shared_ptr<KernelInfo> > KernelMap;
        typedef std::map< Key_t, shared_ptr<NodeInfo> > NodeMap;
        typedef std::map< Key_t, shared_ptr<PortInfo> > PortMap;

        LocalContext();
        virtual ~LocalContext();

        virtual void Log(int level, const std::string &msg);
        virtual int LogLevel() const;
        virtual int LogLevel(int level);

        virtual Key_t SetupKernel(const std::string &name, const std::string &hostname,
                const std::string &servname, KernelBase *kernel);
        virtual Key_t SetupKernel(const std::string &name, KernelBase *kernel);
        virtual Key_t GetKernelKey(const std::string &kernel);
        virtual std::string GetKernelName(Key_t kernelkey);
        virtual void GetKernelConnectionInfo(Key_t kernelkey, std::string &hostname, std::string &servname);
        virtual void SignalKernelEnd(Key_t kernelkey);
        virtual Key_t WaitForKernelStart(const std::string &kernel);
        virtual void SignalKernelStart(Key_t kernelkey);

        virtual void SendCreateWriter(Key_t kernelkey, const SimpleQueueAttr &attr);
        virtual void SendCreateReader(Key_t kernelkey, const SimpleQueueAttr &attr);
        virtual void SendCreateQueue(Key_t kernelkey, const SimpleQueueAttr &attr);
        virtual void SendCreateNode(Key_t kernelkey, const NodeAttr &attr);

        virtual Key_t CreateNodeKey(Key_t kernelkey, const std::string &nodename);
        virtual Key_t GetNodeKey(const std::string &nodename);
        virtual std::string GetNodeName(Key_t nodekey);
        virtual Key_t GetNodeKernel(Key_t nodekey);
        virtual void SignalNodeStart(Key_t nodekey);
        virtual void SignalNodeEnd(Key_t nodekey);
        virtual Key_t WaitForNodeStart(const std::string &nodename);
        virtual void WaitForNodeEnd(const std::string &nodename);
        virtual void WaitForAllNodeEnd();

        virtual Key_t GetCreateReaderKey(Key_t nodekey, const std::string &portname);
        virtual Key_t GetReaderNode(Key_t portkey);
        virtual Key_t GetReaderKernel(Key_t portkey);
        virtual std::string GetReaderName(Key_t portkey);

        virtual Key_t GetCreateWriterKey(Key_t nodekey, const std::string &portname);
        virtual Key_t GetWriterNode(Key_t portkey);
        virtual Key_t GetWriterKernel(Key_t portkey);
        virtual std::string GetWriterName(Key_t portkey);

        virtual void ConnectEndpoints(Key_t writerkey, Key_t readerkey, const std::string &qname);
        virtual Key_t GetReadersWriter(Key_t readerkey);
        virtual Key_t GetWritersReader(Key_t writerkey);

        virtual void Terminate();
        virtual bool IsTerminated();
    private:

        void InternalCheckTerminated();

        int loglevel;
        mutable PthreadMutex lock;
        PthreadCondition nodelivedead;
        PthreadCondition kernellivedead;
        NameMap kernelnames;
        NameMap nodenames;
        KernelMap kernelmap;
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


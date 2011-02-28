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

#include "LocalContext.h"
#include "KernelBase.h"
#include "KernelAttr.h"
#include "Exceptions.h"
#include "Assert.h"
#include <iostream>
#include <stdexcept>

namespace CPN {

    LocalContext::LocalContext()
        : loglevel(Logger::WARNING), numlivenodes(0), shutdown(false), counter(0)
    {}

    LocalContext::~LocalContext() {
    }

    void LocalContext::Log(int level, const std::string &msg) {
        PthreadMutexProtected pl(lock);
        if (level >= loglevel) {
            std::cerr << level << ":" << msg << std::endl;
        }
    }

    int LocalContext::LogLevel() const {
        PthreadMutexProtected pl(lock);
        return loglevel;
    }

    int LocalContext::LogLevel(int level) {
        PthreadMutexProtected pl(lock);
        return loglevel = level;
    }

    Key_t LocalContext::SetupHost(const std::string &name, const std::string &hostname,
            const std::string &servname, KernelBase *kernel) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        if (!kernel) { throw std::invalid_argument("Must have non null Kernel."); }
        if (hostnames.find(name) != hostnames.end()) {
           throw std::invalid_argument("Cannot create two kernels with the same name");
        }
        shared_ptr<HostInfo> hinfo = shared_ptr<HostInfo>(new HostInfo);
        hinfo->name = name;
        hinfo->hostname = hostname;
        hinfo->servname = servname;
        hinfo->kernel = kernel;
        hinfo->dead = false;
        hinfo->live = false;
        hinfo->allowremote = true;
        Key_t key = NewKey();
        hostmap.insert(std::make_pair(key, hinfo));
        hostnames.insert(std::make_pair(name, key));
        return key;
    }

    Key_t LocalContext::SetupHost(const std::string &name, KernelBase *kernel) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        if (!kernel) { throw std::invalid_argument("Must have non null Kernel."); }
        if (hostnames.find(name) != hostnames.end()) {
           throw std::invalid_argument("Cannot create two kernels with the same name");
        }
        shared_ptr<HostInfo> hinfo = shared_ptr<HostInfo>(new HostInfo);
        hinfo->name = name;
        hinfo->kernel = kernel;
        hinfo->dead = false;
        hinfo->live = false;
        hinfo->allowremote = false;
        Key_t key = NewKey();
        hostmap.insert(std::make_pair(key, hinfo));
        hostnames.insert(std::make_pair(name, key));
        return key;

    }

    Key_t LocalContext::GetHostKey(const std::string &host) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NameMap::iterator entry = hostnames.find(host);
        if (entry == hostnames.end()) {
            throw std::invalid_argument("No such host");
        }
        return entry->second;
    }

    std::string LocalContext::GetHostName(Key_t hostkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        return entry->second->name;
    }

    void LocalContext::GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        ASSERT(entry->second->allowremote, "Host does not have remote configured.");
        hostname = entry->second->hostname;
        servname = entry->second->servname;
    }

    void LocalContext::SignalHostEnd(Key_t hostkey) {
        PthreadMutexProtected pl(lock);
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        entry->second->dead = true;
        hostlivedead.Broadcast();
    }

    Key_t LocalContext::WaitForHostStart(const std::string &host) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        while (true) {
            NameMap::iterator entry = hostnames.find(host);
            if (entry != hostnames.end()) {
                HostMap::iterator hentry = hostmap.find(entry->second);
                if (hentry->second->live) {
                    return entry->second;
                }
            }
            hostlivedead.Wait(lock);
            InternalCheckTerminated();
        }
    }

    void LocalContext::SignalHostStart(Key_t hostkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        entry->second->live = true;
        hostlivedead.Broadcast();
    }

    void LocalContext::SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr) {
        KernelBase *kernel;
        {
            PthreadMutexProtected pl(lock);
            InternalCheckTerminated();
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kernel = hinfo->kernel;
        }
        ASSERT(kernel);
        kernel->RemoteCreateWriter(attr);
    }

    void LocalContext::SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr) {
        KernelBase *kernel;
        {
            PthreadMutexProtected pl(lock);
            InternalCheckTerminated();
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kernel = hinfo->kernel;
        }
        ASSERT(kernel);
        kernel->RemoteCreateReader(attr);
    }

    void LocalContext::SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr) {
        KernelBase *kernel;
        {
            PthreadMutexProtected pl(lock);
            InternalCheckTerminated();
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kernel = hinfo->kernel;
        }
        ASSERT(kernel);
        kernel->RemoteCreateQueue(attr);
    }

    void LocalContext::SendCreateNode(Key_t hostkey, const NodeAttr &attr) {
        KernelBase *kernel;
        {
            PthreadMutexProtected pl(lock);
            InternalCheckTerminated();
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kernel = hinfo->kernel;
        }
        ASSERT(kernel);
        kernel->RemoteCreateNode(attr);
    }

    Key_t LocalContext::CreateNodeKey(Key_t hostkey, const std::string &nodename) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NameMap::iterator nameentry = nodenames.find(nodename);
        Key_t key;
        if (nameentry == nodenames.end()) {
            shared_ptr<NodeInfo> ninfo = shared_ptr<NodeInfo>(new NodeInfo);
            ninfo->name = nodename;
            ninfo->started = false;
            ninfo->dead = false;
            ninfo->hostkey = hostkey;
            key = NewKey();
            nodenames.insert(std::make_pair(nodename, key));
            nodemap.insert(std::make_pair(key, ninfo));
        } else {
            throw std::invalid_argument("Node " + nodename + " already exists.");
        }
        return key;
    }

    Key_t LocalContext::GetNodeKey(const std::string &nodename) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NameMap::iterator nameentry = nodenames.find(nodename);
        if (nameentry == nodenames.end()) {
            throw std::invalid_argument("No such node");
        } else {
            return nameentry->second;
        }
    }

    std::string LocalContext::GetNodeName(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        }
        return entry->second->name;
    }

    void LocalContext::SignalNodeStart(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        } else {
            entry->second->started = true;
            ++numlivenodes;
            nodelivedead.Broadcast();
        }
    }

    void LocalContext::SignalNodeEnd(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        } else {
            entry->second->dead = true;
            --numlivenodes;
            nodelivedead.Broadcast();
        }
    }

    Key_t LocalContext::WaitForNodeStart(const std::string &nodename) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        while (true) {
            NameMap::iterator nameentry = nodenames.find(nodename);
            if (nameentry != nodenames.end()) {
                NodeMap::iterator entry = nodemap.find(nameentry->second);
                if (entry != nodemap.end()) {
                    if (entry->second->started) {
                        return entry->first;
                    }
                }
            }
            nodelivedead.Wait(lock);
            InternalCheckTerminated();
        }
    }

    void LocalContext::WaitForNodeEnd(const std::string &nodename) {
        PthreadMutexProtected pl(lock);
        while (!shutdown) {
            NameMap::iterator nameentry = nodenames.find(nodename);
            if (nameentry != nodenames.end()) {
                NodeMap::iterator entry = nodemap.find(nameentry->second);
                if (entry != nodemap.end()) {
                    if (entry->second->dead) {
                        ASSERT(entry->second->started, "Node died before it started!?");
                        return;
                    }
                }
            }
            nodelivedead.Wait(lock);
        }
    }

    void LocalContext::WaitForAllNodeEnd() {
        PthreadMutexProtected pl(lock);
        while (numlivenodes > 0 && !shutdown) {
            nodelivedead.Wait(lock);
        }
    }

    Key_t LocalContext::GetNodeHost(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        }
        return entry->second->hostkey;
    }

    Key_t LocalContext::GetCreateReaderKey(Key_t nodekey, const std::string &portname) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        }
        NameMap::iterator portentry = entry->second->readers.find(portname);
        if (portentry == entry->second->readers.end()) {
            Key_t key = NewKey();
            entry->second->readers.insert(std::make_pair(portname, key));
            shared_ptr<PortInfo> pinfo = shared_ptr<PortInfo>(new PortInfo);
            pinfo->name = portname;
            pinfo->nodekey = nodekey;
            pinfo->opposingport = 0;
            pinfo->dead = false;
            readports.insert(std::make_pair(key, pinfo));
            return key;
        } else {
            return portentry->second;
        }
    }
    Key_t LocalContext::GetReaderNode(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator entry = readports.find(portkey);
        if (entry == readports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->nodekey;
    }

    Key_t LocalContext::GetReaderHost(Key_t portkey) {
        Key_t nodekey = GetReaderNode(portkey);
        return GetNodeHost(nodekey);
    }

    std::string LocalContext::GetReaderName(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator entry = readports.find(portkey);
        if (entry == readports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->name;
    }

    Key_t LocalContext::GetCreateWriterKey(Key_t nodekey, const std::string &portname) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        }
        NameMap::iterator portentry = entry->second->writers.find(portname);
        if (portentry == entry->second->writers.end()) {
            Key_t key = NewKey();
            entry->second->writers.insert(std::make_pair(portname, key));
            shared_ptr<PortInfo> pinfo = shared_ptr<PortInfo>(new PortInfo);
            pinfo->name = portname;
            pinfo->nodekey = nodekey;
            pinfo->opposingport = 0;
            pinfo->dead = false;
            writeports.insert(std::make_pair(key, pinfo));
            return key;
        } else {
            return portentry->second;
        }
    }
    Key_t LocalContext::GetWriterNode(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator entry = writeports.find(portkey);
        if (entry == writeports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->nodekey;
    }

    Key_t LocalContext::GetWriterHost(Key_t portkey) {
        Key_t nodekey = GetWriterNode(portkey);
        return GetNodeHost(nodekey);
    }

    std::string LocalContext::GetWriterName(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator entry = writeports.find(portkey);
        if (entry == writeports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->name;
    }

    void LocalContext::ConnectEndpoints(Key_t writerkey, Key_t readerkey, const std::string &qname) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator writeentry = writeports.find(writerkey);
        if (writeentry == writeports.end()) {
            throw std::invalid_argument("Write port does not exist.");
        }
        PortMap::iterator readentry = readports.find(readerkey);
        if (readentry == readports.end()) {
            throw std::invalid_argument("Read port does not exist.");
        }

        writeentry->second->opposingport = readerkey;
        readentry->second->opposingport = writerkey;

        readentry->second->qname = qname;
        writeentry->second->qname = qname;
    }

    Key_t LocalContext::GetReadersWriter(Key_t readerkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator readentry = readports.find(readerkey);
        if (readentry == readports.end()) {
            throw std::invalid_argument("Read port does not exist.");
        }
        return readentry->second->opposingport;
    }

    Key_t LocalContext::GetWritersReader(Key_t writerkey) {
        PthreadMutexProtected pl(lock);
        InternalCheckTerminated();
        PortMap::iterator writeentry = writeports.find(writerkey);
        if (writeentry == writeports.end()) {
            throw std::invalid_argument("Write port does not exist.");
        }
        return writeentry->second->opposingport;
    }

    void LocalContext::Terminate() {
        HostMap mapcopy;
        {
            PthreadMutexProtected pl(lock);
            shutdown = true;
            nodelivedead.Broadcast();
            hostlivedead.Broadcast();
            mapcopy = hostmap;
        }
        HostMap::iterator itr = mapcopy.begin();
        while (itr != mapcopy.end()) {
            if (!itr->second->dead) {
                itr->second->kernel->NotifyTerminate();
            }
            ++itr;
        }
}

    bool LocalContext::IsTerminated() {
        PthreadMutexProtected pl(lock);
        return shutdown;
    }

    void LocalContext::InternalCheckTerminated() {
        if (shutdown) {
            throw ShutdownException();
        }
    }
}


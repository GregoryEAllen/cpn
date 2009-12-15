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

#include "LocalDatabase.h"
#include "KernelAttr.h"
#include "Assert.h"
#include <iostream>
#include <stdexcept>

namespace CPN {

    LocalDatabase::LocalDatabase() : loglevel(Logger::WARNING), numlivenodes(0), counter(0) {}
    LocalDatabase::~LocalDatabase() {}

    void LocalDatabase::Log(int level, const std::string &msg) const {
        PthreadMutexProtected pl(lock);
        if (level >= loglevel) {
            std::cout << level << ":" << msg << std::endl;
        }
    }

    int LocalDatabase::LogLevel() const {
        PthreadMutexProtected pl(lock);
        return loglevel;
    }

    int LocalDatabase::LogLevel(int level) {
        PthreadMutexProtected pl(lock);
        return loglevel = level;
    }

    Key_t LocalDatabase::SetupHost(const std::string &name, const std::string &hostname,
            const std::string &servname, KernelMessageHandler *kmh) {
        PthreadMutexProtected pl(lock);
        ASSERT(kmh, "Must have non null KernelMessageHandler.");
        ASSERT(hostnames.find(name) == hostnames.end(), "Names must be unique.");
        shared_ptr<HostInfo> hinfo = shared_ptr<HostInfo>(new HostInfo);
        hinfo->name = name;
        hinfo->hostname = hostname;
        hinfo->servname = servname;
        hinfo->kmh = kmh;
        hinfo->dead = false;
        hinfo->live = false;
        Key_t key = NewKey();
        hostmap.insert(std::make_pair(key, hinfo));
        hostnames.insert(std::make_pair(name, key));
        return key;
    }

    Key_t LocalDatabase::GetHostKey(const std::string &host) {
        PthreadMutexProtected pl(lock);
        NameMap::iterator entry = hostnames.find(host);
        if (entry == hostnames.end()) {
            throw std::invalid_argument("No such host");
        }
        return entry->second;
    }

    const std::string &LocalDatabase::GetHostName(Key_t hostkey) {
        PthreadMutexProtected pl(lock);
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        return entry->second->name;
    }

    void LocalDatabase::GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname) {
        PthreadMutexProtected pl(lock);
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        hostname = entry->second->hostname;
        servname = entry->second->servname;
    }

    void LocalDatabase::DestroyHostKey(Key_t hostkey) {
        PthreadMutexProtected pl(lock);
        hostmap[hostkey]->dead = true;
        hostlivedead.Broadcast();
    }

    Key_t LocalDatabase::WaitForHostStart(const std::string &host) {
        PthreadMutexProtected pl(lock);
        while (true) {
            NameMap::iterator entry = hostnames.find(host);
            if (entry != hostnames.end()) {
                HostMap::iterator hentry = hostmap.find(entry->second);
                if (hentry->second->live) {
                    return entry->second;
                }
            }
            hostlivedead.Wait(lock);
        }
    }

    void LocalDatabase::SignalHostStart(Key_t hostkey) {
        PthreadMutexProtected pl(lock);
        HostMap::iterator entry = hostmap.find(hostkey);
        if (entry == hostmap.end()) {
            throw std::invalid_argument("No such host");
        }
        entry->second->live = true;
        hostlivedead.Broadcast();
    }

    void LocalDatabase::SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr) {
        KernelMessageHandler *kmh;
        {
            PthreadMutexProtected pl(lock);
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kmh = hinfo->kmh;
        }
        ASSERT(kmh);
        kmh->CreateWriter(hostkey, attr);
    }

    void LocalDatabase::SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr) {
        KernelMessageHandler *kmh;
        {
            PthreadMutexProtected pl(lock);
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kmh = hinfo->kmh;
        }
        ASSERT(kmh);
        kmh->CreateReader(hostkey, attr);
    }

    void LocalDatabase::SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr) {
        KernelMessageHandler *kmh;
        {
            PthreadMutexProtected pl(lock);
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kmh = hinfo->kmh;
        }
        ASSERT(kmh);
        kmh->CreateQueue(hostkey, attr);
    }

    void LocalDatabase::SendCreateNode(Key_t hostkey, const NodeAttr &attr) {
        KernelMessageHandler *kmh;
        {
            PthreadMutexProtected pl(lock);
            shared_ptr<HostInfo> hinfo = hostmap[hostkey];
            kmh = hinfo->kmh;
        }
        ASSERT(kmh);
        kmh->CreateNode(hostkey, attr);
    }

    Key_t LocalDatabase::CreateNodeKey(Key_t hostkey, const std::string &nodename) {
        PthreadMutexProtected pl(lock);
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

    Key_t LocalDatabase::GetNodeKey(const std::string &nodename) {
        PthreadMutexProtected pl(lock);
        NameMap::iterator nameentry = nodenames.find(nodename);
        if (nameentry == nodenames.end()) {
            throw std::invalid_argument("No such node");
        } else {
            return nameentry->second;
        }
    }

    const std::string &LocalDatabase::GetNodeName(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        }
        return entry->second->name;
    }

    void LocalDatabase::SignalNodeStart(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        } else {
            entry->second->started = true;
            ++numlivenodes;
            nodelivedead.Broadcast();
        }
    }

    void LocalDatabase::SignalNodeEnd(Key_t nodekey) {
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

    Key_t LocalDatabase::WaitForNodeStart(const std::string &nodename) {
        PthreadMutexProtected pl(lock);
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
        }
    }

    void LocalDatabase::WaitForNodeEnd(const std::string &nodename) {
        PthreadMutexProtected pl(lock);
        while (true) {
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

    void LocalDatabase::WaitForAllNodeEnd() {
        PthreadMutexProtected pl(lock);
        while (numlivenodes > 0) {
            nodelivedead.Wait(lock);
        }
    }

    Key_t LocalDatabase::GetNodeHost(Key_t nodekey) {
        PthreadMutexProtected pl(lock);
        NodeMap::iterator entry = nodemap.find(nodekey);
        if (entry == nodemap.end()) {
            throw std::invalid_argument("No such node");
        }
        return entry->second->hostkey;
    }

    Key_t LocalDatabase::GetCreateReaderKey(Key_t nodekey, const std::string &portname) {
        PthreadMutexProtected pl(lock);
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
    Key_t LocalDatabase::GetReaderNode(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator entry = readports.find(portkey);
        if (entry == readports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->nodekey;
    }

    Key_t LocalDatabase::GetReaderHost(Key_t portkey) {
        Key_t nodekey = GetReaderNode(portkey);
        return GetNodeHost(nodekey);
    }

    const std::string &LocalDatabase::GetReaderName(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator entry = readports.find(portkey);
        if (entry == readports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->name;
    }

    void LocalDatabase::DestroyReaderKey(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator entry = readports.find(portkey);
        ASSERT(entry != readports.end());
        entry->second->dead = true;
    }

    Key_t LocalDatabase::GetCreateWriterKey(Key_t nodekey, const std::string &portname) {
        PthreadMutexProtected pl(lock);
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
    Key_t LocalDatabase::GetWriterNode(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator entry = writeports.find(portkey);
        if (entry == writeports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->nodekey;
    }

    Key_t LocalDatabase::GetWriterHost(Key_t portkey) {
        Key_t nodekey = GetWriterNode(portkey);
        return GetNodeHost(nodekey);
    }

    const std::string &LocalDatabase::GetWriterName(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator entry = writeports.find(portkey);
        if (entry == writeports.end()) {
            throw std::invalid_argument("No such port");
        }
        return entry->second->name;
    }

    void LocalDatabase::DestroyWriterKey(Key_t portkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator entry = writeports.find(portkey);
        ASSERT(entry != writeports.end());
        entry->second->dead = true;
    }

    void LocalDatabase::ConnectEndpoints(Key_t writerkey, Key_t readerkey) {
        PthreadMutexProtected pl(lock);
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
    }

    Key_t LocalDatabase::GetReadersWriter(Key_t readerkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator readentry = readports.find(readerkey);
        if (readentry == readports.end()) {
            throw std::invalid_argument("Read port does not exist.");
        }
        return readentry->second->opposingport;
    }

    Key_t LocalDatabase::GetWritersReader(Key_t writerkey) {
        PthreadMutexProtected pl(lock);
        PortMap::iterator writeentry = writeports.find(writerkey);
        if (writeentry == writeports.end()) {
            throw std::invalid_argument("Write port does not exist.");
        }
        return writeentry->second->opposingport;
    }

}


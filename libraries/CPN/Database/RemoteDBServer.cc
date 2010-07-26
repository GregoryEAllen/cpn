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

#include "RemoteDBServer.h"
#include "RDBMT.h"
#include "Assert.h"
#include "VariantToJSON.h"
#include <stdio.h>
#include <stdarg.h>

namespace CPN {

    RemoteDBServer::RemoteDBServer()
        : debuglevel(0), shutdown(false), numlivenodes(0), keycount(0)
    {
    }

    RemoteDBServer::~RemoteDBServer() {
    }

    void RemoteDBServer::dbprintf(int level, const char *fmt, ...) {
        if (debuglevel >= level) {
            va_list ap;
            va_start(ap, fmt);
            vprintf(fmt, ap);
            va_end(ap);
        }
    }

    void RemoteDBServer::DispatchMessage(const std::string &sender, const Variant &msg) {
        dbprintf(4, "msg:%s:%s\n", sender.c_str(), VariantToJSON(msg).c_str());
        if (IsTerminated()) {
            return;
        }
        RDBMT_t type = msg["type"].AsNumber<RDBMT_t>();
        switch (type) {
        case RDBMT_SETUP_HOST:
            SetupHost(sender, msg);
            break;
        case RDBMT_SIGNAL_HOST_START:
            SignalHostStart(msg);
            break;
        case RDBMT_DESTROY_HOST_KEY:
            DestroyHostKey(msg);
            break;
        case RDBMT_GET_HOST_INFO:
            GetHostInfo(sender, msg);
            break;
        case RDBMT_CREATE_WRITER:
        case RDBMT_CREATE_READER:
        case RDBMT_CREATE_QUEUE:
        case RDBMT_CREATE_NODE:
            RouteKernelMessage(msg);
            break;
        case RDBMT_CREATE_NODE_KEY:
            CreateNodeKey(sender, msg);
            break;
        case RDBMT_SIGNAL_NODE_START:
            SignalNodeStart(msg);
            break;
        case RDBMT_SIGNAL_NODE_END:
            SignalNodeEnd(msg);
            break;
        case RDBMT_GET_NODE_INFO:
            GetNodeInfo(sender, msg);
            break;
        case RDBMT_GET_NUM_NODE_LIVE:
            GetNumNodeLive(sender, msg);
            break;
        case RDBMT_GET_CREATE_READER_KEY:
            GetCreateEndpointKey(sender, msg);
            break;
        case RDBMT_DESTROY_READER_KEY:
            DestroyEndpointKey(msg);
            break;
        case RDBMT_GET_READER_INFO:
            GetEndpointInfo(sender, msg);
            break;
        case RDBMT_GET_CREATE_WRITER_KEY:
            GetCreateEndpointKey(sender, msg);
            break;
        case RDBMT_DESTROY_WRITER_KEY:
            DestroyEndpointKey(msg);
            break;
        case RDBMT_GET_WRITER_INFO:
            GetEndpointInfo(sender, msg);
            break;
        case RDBMT_CONNECT_ENDPOINTS:
            ConnectEndpoints(msg);
            break;
        case RDBMT_TERMINATE:
            Terminate();
            break;
        case RDBMT_LOG:
            LogMessage(sender + ":" + msg["msg"].AsString());
            break;
        default:
            ASSERT(false);
        }
    }

    void RemoteDBServer::Terminate() {
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_TERMINATE;
        BroadcastMessage(msg);
        shutdown = true;
    }

    void RemoteDBServer::SetupHost(const std::string &sender, const Variant &msg) {
        std::string name = msg["name"].AsString();
        if (hostmap.find(name) != hostmap.end()) {
            Variant reply(Variant::ObjectType);
            reply["msgid"] = msg["msgid"];
            reply["msgtype"] = "reply";
            reply["success"] = false;
            SendMessage(sender, reply);
            return;
        }
        Variant hostinfo(Variant::ObjectType);
        Key_t hostkey = NewKey();
        hostinfo["key"] = hostkey;
        hostinfo["name"] = name;
        hostinfo["hostname"] = msg["hostname"];
        hostinfo["servname"] = msg["servname"];
        hostinfo["live"] = false;
        hostinfo["type"] = "hostinfo";
        hostinfo["client"] = sender;
        datamap.insert(std::make_pair(hostkey, hostinfo));
        hostmap.insert(std::make_pair(name, hostkey));
        dbprintf(2, "Host %s created\n", name.c_str());
        Variant reply;
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["success"] = true;
        reply["hostinfo"] = hostinfo.Copy();
        SendMessage(sender, reply);
    }

    void RemoteDBServer::SignalHostStart(const Variant &msg) {
        Key_t hostkey = msg["key"].AsNumber<Key_t>();
        Variant hostinfo = datamap[hostkey];
        ASSERT(hostinfo["type"].AsString() == "hostinfo");
        hostinfo["live"] = true;
        dbprintf(2, "Host %s started\n", hostinfo["name"].AsString().c_str());
        Variant notice = NewBroadcastMessage();
        notice["hostinfo"] = hostinfo.Copy();
        BroadcastMessage(notice);
    }

    void RemoteDBServer::DestroyHostKey(const Variant &msg) {
        Key_t hostkey = msg["key"].AsNumber<Key_t>();
        Variant hostinfo = datamap[hostkey];
        ASSERT(hostinfo["type"].AsString() == "hostinfo");
        hostinfo["live"] = false;
        dbprintf(2, "Host %s stopped\n", hostinfo["name"].AsString().c_str());
        Variant notice = NewBroadcastMessage();
        notice["hostinfo"] = hostinfo.Copy();
        BroadcastMessage(notice);
    }

    void RemoteDBServer::GetHostInfo(const std::string &sender, const Variant &msg) {
        Key_t hostkey;
        Variant reply(Variant::ObjectType);
        reply["msgtype"] = "reply";
        reply["msgid"] = msg["msgid"];
        reply["success"] = false;
        if (msg["name"].IsString()) {
            NameKeyMap::iterator entry = hostmap.find(msg["name"].AsString());
            if (entry == hostmap.end()) {
                SendMessage(sender, reply);
                return;
            }
            hostkey = entry->second;
        } else {
            hostkey = msg["key"].AsNumber<Key_t>();
        }
        DataMap::iterator entry = datamap.find(hostkey);
        if (entry == datamap.end()) {
            SendMessage(sender, reply);
            return;
        }
        ASSERT(entry->second["type"].AsString() == "hostinfo");
        reply["success"] = true;
        reply["hostinfo"] = entry->second.Copy();
        SendMessage(sender, reply);
    }

    void RemoteDBServer::CreateNodeKey(const std::string &sender, const Variant &msg) {
        std::string nodename = msg["name"].AsString();
        Variant reply(Variant::ObjectType);
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["success"] = false;
        if (nodemap.find(nodename) != nodemap.end()) {
            SendMessage(sender, reply);
            return;
        }
        Key_t nodekey = NewKey();
        Variant nodeinfo(Variant::ObjectType);
        nodeinfo["name"] = nodename;
        nodeinfo["key"] = nodekey;
        nodeinfo["hostkey"] = msg["hostkey"];
        nodeinfo["started"] = false;
        nodeinfo["dead"] = false;
        nodeinfo["type"] = "nodeinfo";
        nodeinfo["endpoints"] = Variant::ObjectType;
        dbprintf(2, "Node %s created\n", nodename.c_str());
        nodemap[nodename] = nodekey;
        datamap[nodekey] = nodeinfo;
        reply["success"] = true;
        reply["nodeinfo"] = nodeinfo.Copy();
        SendMessage(sender, reply);
    }

    void RemoteDBServer::SignalNodeStart(const Variant &msg) {
        ++numlivenodes;
        Key_t nodekey = msg["key"].AsNumber<Key_t>();
        Variant nodeinfo = datamap[nodekey];
        ASSERT(nodeinfo["type"].AsString() == "nodeinfo");
        nodeinfo["started"] = true;
        dbprintf(2, "Node %s started\n", nodeinfo["name"].AsString().c_str());
        Variant notice = NewBroadcastMessage();
        notice["nodeinfo"] = nodeinfo.Copy();
        BroadcastMessage(notice);
    }

    void RemoteDBServer::SignalNodeEnd(const Variant &msg) {
        --numlivenodes;
        Key_t nodekey = msg["key"].AsNumber<Key_t>();
        Variant nodeinfo = datamap[nodekey];
        ASSERT(nodeinfo["type"].AsString() == "nodeinfo");
        nodeinfo["dead"] = true;
        dbprintf(2, "Node %s stopped\n", nodeinfo["name"].AsString().c_str());
        Variant notice = NewBroadcastMessage();
        notice["nodeinfo"] = nodeinfo.Copy();
        BroadcastMessage(notice);
    }

    void RemoteDBServer::GetNodeInfo(const std::string &sender, const Variant &msg) {
        Key_t nodekey;
        Variant reply(Variant::ObjectType);
        reply["msgtype"] = "reply";
        reply["msgid"] = msg["msgid"];
        reply["success"] = false;
        if (msg["name"].IsString()) {
            NameKeyMap::iterator entry = nodemap.find(msg["name"].AsString());
            if (entry == nodemap.end()) {
                SendMessage(sender, reply);
                return;
            }
            nodekey = entry->second;
        } else {
            nodekey = msg["key"].AsNumber<Key_t>();
        }
        DataMap::iterator entry = datamap.find(nodekey);
        if (entry == datamap.end()) {
            SendMessage(sender, reply);
            return;
        }
        reply["nodeinfo"] = entry->second.Copy();
        ASSERT(reply["nodeinfo"]["type"].AsString() == "nodeinfo");
        reply["success"] = true;
        SendMessage(sender, reply);
    }

    void RemoteDBServer::GetNumNodeLive(const std::string &sender, const Variant &msg) {
        Variant reply(Variant::ObjectType);
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["numlivenodes"] = numlivenodes;
        reply["success"] = true;
        SendMessage(sender, reply);
    }

    void RemoteDBServer::GetCreateEndpointKey(const std::string &sender, const Variant &msg) {
        std::string name = msg["name"].AsString();
        Key_t nodekey = msg["nodekey"].AsNumber<Key_t>();
        Variant nodeinfo = datamap[nodekey];
        Variant epinfo;
        Key_t epkey;
        if (nodeinfo["endpoints"][name].IsNumber()) {
            epkey = nodeinfo["endpoints"][name].AsNumber<Key_t>();
            epinfo = datamap[epkey];
            ASSERT(epinfo["type"].AsString() == "endpointinfo");
        } else {
            epkey = NewKey();
            nodeinfo["endpoints"][name] = epkey;
            epinfo = Variant(Variant::ObjectType);
            epinfo["key"] = epkey;
            epinfo["name"] = name;
            epinfo["nodekey"] = nodekey;
            epinfo["hostkey"] = nodeinfo["hostkey"];
            epinfo["live"] = true;
            epinfo["type"] = "endpointinfo";
            datamap[epkey] = epinfo;
        }
        Variant reply;
        reply["endpointinfo"] = epinfo.Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["success"] = true;
        SendMessage(sender, reply);
    }

    void RemoteDBServer::DestroyEndpointKey(const Variant &msg) {
        Key_t epkey = msg["key"].AsNumber<Key_t>();
        ASSERT(datamap[epkey]["type"].AsString() == "endpointinfo");
        datamap[epkey]["live"] = false;
    }

    void RemoteDBServer::GetEndpointInfo(const std::string &sender, const Variant &msg) {
        Key_t epkey = msg["key"].AsNumber<Key_t>();
        Variant reply;
        reply["endpointinfo"] = datamap[epkey].Copy();
        ASSERT(reply["endpointinfo"]["type"].AsString() == "endpointinfo");
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["success"] = true;
        SendMessage(sender, reply);

    }

    void RemoteDBServer::ConnectEndpoints(const Variant &msg) {
        Key_t wkey = msg["writerkey"].AsNumber<Key_t>();
        Key_t rkey = msg["readerkey"].AsNumber<Key_t>();
        datamap[wkey]["readerkey"] = rkey;
        datamap[rkey]["writerkey"] = wkey;
    }

    Key_t RemoteDBServer::NewKey() {
        // 0 is reserved
        return ++keycount;
    }

    void RemoteDBServer::RouteKernelMessage(const Variant &msg) {
        Key_t hostkey = msg["hostkey"].AsNumber<Key_t>();
        Variant hostinfo = datamap[hostkey];
        SendMessage(hostinfo["client"].AsString(), msg);
    }

    Variant RemoteDBServer::NewBroadcastMessage() {
        Variant msg;
        msg["msgtype"] = "broadcast";
        msg["numlivenodes"] = numlivenodes;
        return msg;
    }
}

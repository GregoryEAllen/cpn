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

#include "RemoteContextServer.h"
#include "RCTXMT.h"
#include "Assert.h"
#include "VariantToJSON.h"
#include <stdio.h>
#include <stdarg.h>

namespace CPN {

    RemoteContextServer::RemoteContextServer()
        : debuglevel(0), shutdown(false), numlivenodes(0), keycount(0)
    {
    }

    RemoteContextServer::~RemoteContextServer() {
    }

    void RemoteContextServer::dbprintf(int level, const char *fmt, ...) {
        if (debuglevel >= level) {
            va_list ap;
            va_start(ap, fmt);
            vprintf(fmt, ap);
            va_end(ap);
        }
    }

    void RemoteContextServer::DispatchMessage(const std::string &sender, const Variant &msg) {
        dbprintf(4, "msg:%s:%s\n", sender.c_str(), VariantToJSON(msg).c_str());
        if (IsTerminated()) {
            return;
        }
        RCTXMT_t type = msg["type"].AsNumber<RCTXMT_t>();
        switch (type) {
        case RCTXMT_SETUP_KERNEL:
            SetupKernel(sender, msg);
            break;
        case RCTXMT_SIGNAL_KERNEL_START:
            SignalKernelStart(msg);
            break;
        case RCTXMT_SIGNAL_KERNEL_END:
            SignalKernelEnd(msg);
            break;
        case RCTXMT_GET_KERNEL_INFO:
            GetKernelInfo(sender, msg);
            break;
        case RCTXMT_CREATE_WRITER:
        case RCTXMT_CREATE_READER:
        case RCTXMT_CREATE_QUEUE:
        case RCTXMT_CREATE_NODE:
            RouteKernelMessage(msg);
            break;
        case RCTXMT_CREATE_NODE_KEY:
            CreateNodeKey(sender, msg);
            break;
        case RCTXMT_SIGNAL_NODE_START:
            SignalNodeStart(msg);
            break;
        case RCTXMT_SIGNAL_NODE_END:
            SignalNodeEnd(msg);
            break;
        case RCTXMT_GET_NODE_INFO:
            GetNodeInfo(sender, msg);
            break;
        case RCTXMT_GET_NUM_NODE_LIVE:
            GetNumNodeLive(sender, msg);
            break;
        case RCTXMT_GET_CREATE_READER_KEY:
            GetCreateEndpointKey(sender, msg);
            break;
        case RCTXMT_GET_READER_INFO:
            GetEndpointInfo(sender, msg);
            break;
        case RCTXMT_GET_CREATE_WRITER_KEY:
            GetCreateEndpointKey(sender, msg);
            break;
        case RCTXMT_GET_WRITER_INFO:
            GetEndpointInfo(sender, msg);
            break;
        case RCTXMT_CONNECT_ENDPOINTS:
            ConnectEndpoints(msg);
            break;
        case RCTXMT_TERMINATE:
            Terminate();
            break;
        case RCTXMT_LOG:
            LogMessage(sender + ":" + msg["msg"].AsString());
            break;
        default:
            ASSERT(false);
        }
    }

    void RemoteContextServer::Terminate() {
        Variant msg(Variant::ObjectType);
        msg["type"] = RCTXMT_TERMINATE;
        BroadcastMessage(msg);
        shutdown = true;
    }

    void RemoteContextServer::SetupKernel(const std::string &sender, const Variant &msg) {
        std::string name = msg["name"].AsString();
        Variant reply(Variant::ObjectType);
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        if (kernelmap.find(name) != kernelmap.end()) {
            reply["success"] = false;
            return;
        } else {
            Variant kernelinfo(Variant::ObjectType);
            Key_t kernelkey = NewKey();
            kernelinfo["key"] = kernelkey;
            kernelinfo["name"] = name;
            kernelinfo["hostname"] = msg["hostname"];
            kernelinfo["servname"] = msg["servname"];
            kernelinfo["live"] = false;
            kernelinfo["type"] = "kernelinfo";
            kernelinfo["client"] = sender;
            datamap.insert(std::make_pair(kernelkey, kernelinfo));
            kernelmap.insert(std::make_pair(name, kernelkey));
            dbprintf(2, "Kernel %s created\n", name.c_str());
            reply["success"] = true;
            reply["kernelinfo"] = kernelinfo.Copy();
        }
        SendMessage(sender, reply);
    }

    void RemoteContextServer::SignalKernelStart(const Variant &msg) {
        Key_t kernelkey = msg["key"].AsNumber<Key_t>();
        Variant kernelinfo = datamap[kernelkey];
        ASSERT(kernelinfo["type"].AsString() == "kernelinfo");
        kernelinfo["live"] = true;
        dbprintf(2, "Kernel %s started\n", kernelinfo["name"].AsString().c_str());
        Variant notice = NewBroadcastMessage();
        notice["kernelinfo"] = kernelinfo.Copy();
        BroadcastMessage(notice);
    }

    void RemoteContextServer::SignalKernelEnd(const Variant &msg) {
        Key_t kernelkey = msg["key"].AsNumber<Key_t>();
        Variant kernelinfo = datamap[kernelkey];
        ASSERT(kernelinfo["type"].AsString() == "kernelinfo");
        kernelinfo["live"] = false;
        dbprintf(2, "Kernel %s stopped\n", kernelinfo["name"].AsString().c_str());
        Variant notice = NewBroadcastMessage();
        notice["kernelinfo"] = kernelinfo.Copy();
        BroadcastMessage(notice);
    }

    void RemoteContextServer::GetKernelInfo(const std::string &sender, const Variant &msg) {
        Key_t kernelkey;
        Variant reply(Variant::ObjectType);
        reply["msgtype"] = "reply";
        reply["msgid"] = msg["msgid"];
        reply["success"] = false;
        if (msg["name"].IsString()) {
            NameKeyMap::iterator entry = kernelmap.find(msg["name"].AsString());
            if (entry == kernelmap.end()) {
                SendMessage(sender, reply);
                return;
            }
            kernelkey = entry->second;
        } else {
            kernelkey = msg["key"].AsNumber<Key_t>();
        }
        DataMap::iterator entry = datamap.find(kernelkey);
        if (entry != datamap.end()) {
            ASSERT(entry->second["type"].AsString() == "kernelinfo");
            reply["success"] = true;
            reply["kernelinfo"] = entry->second.Copy();
        }
        SendMessage(sender, reply);
    }

    void RemoteContextServer::CreateNodeKey(const std::string &sender, const Variant &msg) {
        std::string nodename = msg["name"].AsString();
        Variant reply(Variant::ObjectType);
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        if (nodemap.find(nodename) != nodemap.end()) {
            reply["success"] = false;
        } else {
            Key_t nodekey = NewKey();
            Variant nodeinfo(Variant::ObjectType);
            nodeinfo["name"] = nodename;
            nodeinfo["key"] = nodekey;
            nodeinfo["kernelkey"] = msg["kernelkey"];
            nodeinfo["started"] = false;
            nodeinfo["dead"] = false;
            nodeinfo["type"] = "nodeinfo";
            nodeinfo["endpoints"] = Variant::ObjectType;
            dbprintf(2, "Node %s created\n", nodename.c_str());
            nodemap[nodename] = nodekey;
            datamap[nodekey] = nodeinfo;
            reply["success"] = true;
            reply["nodeinfo"] = nodeinfo.Copy();
        }
        SendMessage(sender, reply);
    }

    void RemoteContextServer::SignalNodeStart(const Variant &msg) {
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

    void RemoteContextServer::SignalNodeEnd(const Variant &msg) {
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

    void RemoteContextServer::GetNodeInfo(const std::string &sender, const Variant &msg) {
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
        if (entry != datamap.end()) {
            reply["nodeinfo"] = entry->second.Copy();
            ASSERT(reply["nodeinfo"]["type"].AsString() == "nodeinfo");
            reply["success"] = true;
        }
        SendMessage(sender, reply);
    }

    void RemoteContextServer::GetNumNodeLive(const std::string &sender, const Variant &msg) {
        Variant reply(Variant::ObjectType);
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["numlivenodes"] = numlivenodes;
        reply["success"] = true;
        SendMessage(sender, reply);
    }

    void RemoteContextServer::GetCreateEndpointKey(const std::string &sender, const Variant &msg) {
        std::string name = msg["name"].AsString();
        Variant reply;
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["success"] = false;
        Key_t nodekey = msg["nodekey"].AsNumber<Key_t>();
        DataMap::iterator nodeentry = datamap.find(nodekey);
        if (nodeentry != datamap.end()) {
            Variant nodeinfo = nodeentry->second;
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
                epinfo["kernelkey"] = nodeinfo["kernelkey"];
                epinfo["live"] = true;
                epinfo["type"] = "endpointinfo";
                datamap[epkey] = epinfo;
            }
            reply["endpointinfo"] = epinfo.Copy();
            reply["success"] = true;
        }
        SendMessage(sender, reply);
    }

    void RemoteContextServer::GetEndpointInfo(const std::string &sender, const Variant &msg) {
        Key_t epkey = msg["key"].AsNumber<Key_t>();
        Variant reply;
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["success"] = false;
        DataMap::iterator entry = datamap.find(epkey);
        if (entry != datamap.end()) {
            reply["success"] = true;
            reply["endpointinfo"] = entry->second.Copy();
            ASSERT(reply["endpointinfo"]["type"].AsString() == "endpointinfo");
        }
        SendMessage(sender, reply);

    }

    void RemoteContextServer::ConnectEndpoints(const Variant &msg) {
        Key_t wkey = msg["writerkey"].AsNumber<Key_t>();
        Key_t rkey = msg["readerkey"].AsNumber<Key_t>();
        std::string qname = msg["qname"].AsString();
        datamap[wkey]["readerkey"] = rkey;
        datamap[rkey]["writerkey"] = wkey;
        datamap[rkey]["qname"] = qname;
        datamap[wkey]["qname"] = qname;
    }

    Key_t RemoteContextServer::NewKey() {
        // 0 is reserved
        return ++keycount;
    }

    void RemoteContextServer::RouteKernelMessage(const Variant &msg) {
        Key_t kernelkey = msg["kernelkey"].AsNumber<Key_t>();
        Variant kernelinfo = datamap[kernelkey];
        SendMessage(kernelinfo["client"].AsString(), msg);
    }

    Variant RemoteContextServer::NewBroadcastMessage() {
        Variant msg;
        msg["msgtype"] = "broadcast";
        msg["numlivenodes"] = numlivenodes;
        return msg;
    }
}

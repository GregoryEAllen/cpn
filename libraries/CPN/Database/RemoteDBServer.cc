
#include "RemoteDBServer.h"
#include "RDBMT.h"
#include "Assert.h"

namespace CPN {

    RemoteDBServer::RemoteDBServer()
        : numlivenodes(0), keycount(0)
    {
    }

    RemoteDBServer::~RemoteDBServer() {
    }

    void RemoteDBServer::DispatchMessage(const std::string &sender, const Variant &msg) {
        RDBMT_t type = msg["type"].AsNumber<RDBMT_t>();
        switch (type) {
        case RDBMT_SETUP_HOST:
            SetupHost(sender, msg);
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
            BroadcastMessage(msg);
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
        case RDBMT_GET_WRITER_INFO:
            GetEndpointInfo(sender, msg);
            break;
        case RDBMT_CONNECT_ENDPOINTS:
            ConnectEndpoints(msg);
            break;
        default:
            ASSERT(false);
        }
    }

    void RemoteDBServer::SetupHost(const std::string &sender, const Variant &msg) {
        Variant hostinfo(Variant::ObjectType);
        Key_t hostkey = NewKey();
        std::string name = msg["name"].AsString();
        hostinfo["key"] = hostkey;
        hostinfo["name"] = name;
        hostinfo["hostname"] = msg["hostname"];
        hostinfo["servname"] = msg["servname"];
        hostinfo["live"] = true;
        hostinfo["type"] = "hostinfo";
        datamap.insert(std::make_pair(hostkey, hostinfo));
        hostmap.insert(std::make_pair(name, hostkey));
        Variant reply = hostinfo.Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        SendMessage(sender, reply);
        Variant notice = hostinfo.Copy();
        notice["msgtype"] = "broadcast";
        BroadcastMessage(notice);
    }

    void RemoteDBServer::DestroyHostKey(const Variant &msg) {
        Key_t hostkey = msg["key"].AsNumber<Key_t>();
        Variant hostinfo = datamap[hostkey];
        hostinfo["live"] = false;
        Variant notice = hostinfo.Copy();
        notice["msgtype"] = "broadcast";
        BroadcastMessage(notice);
    }

    void RemoteDBServer::GetHostInfo(const std::string &sender, const Variant &msg) {
        Key_t hostkey;
        if (msg["name"].IsString()) {
            NameKeyMap::iterator entry = hostmap.find(msg["name"].AsString());
            ASSERT(entry != hostmap.end());
            hostkey = entry->second;
        } else {
            hostkey = msg["key"].AsNumber<Key_t>();
        }
        Variant reply = datamap[hostkey].Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        SendMessage(sender, reply);
    }

    void RemoteDBServer::CreateNodeKey(const std::string &sender, const Variant &msg) {
        std::string nodename = msg["name"].AsString();
        Key_t nodekey = NewKey();
        Variant nodeinfo(Variant::ObjectType);
        nodeinfo["name"] = nodename;
        nodeinfo["key"] = nodekey;
        nodeinfo["hostkey"] = msg["hostkey"];
        nodeinfo["started"] = false;
        nodeinfo["dead"] = false;
        nodeinfo["endpoints"] = Variant::ObjectType;
        nodemap[nodename] = nodekey;
        datamap[nodekey] = nodeinfo;
        Variant reply = nodeinfo.Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        SendMessage(sender, reply);
    }

    void RemoteDBServer::SignalNodeStart(const Variant &msg) {
        ++numlivenodes;
        Key_t nodekey = msg["key"].AsNumber<Key_t>();
        Variant nodeinfo = datamap[nodekey];
        nodeinfo["started"] = true;
        Variant notice = nodeinfo.Copy();
        notice["msgtype"] = "broadcast";
        BroadcastMessage(notice);
    }

    void RemoteDBServer::SignalNodeEnd(const Variant &msg) {
        --numlivenodes;
        Key_t nodekey = msg["key"].AsNumber<Key_t>();
        Variant nodeinfo = datamap[nodekey];
        nodeinfo["dead"] = true;
        Variant notice = nodeinfo.Copy();
        notice["msgtype"] = "broadcast";
        BroadcastMessage(notice);
    }

    void RemoteDBServer::GetNodeInfo(const std::string &sender, const Variant &msg) {
        Key_t nodekey;
        if (msg["name"].IsString()) {
            nodekey = nodemap[msg["name"].AsString()];
        } else {
            nodekey = msg["key"].AsNumber<Key_t>();
        }
        Variant reply = datamap[nodekey].Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        SendMessage(sender, reply);
    }

    void RemoteDBServer::GetNumNodeLive(const std::string &sender, const Variant &msg) {
        Variant reply(Variant::ObjectType);
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        reply["numlivenodes"] = numlivenodes;
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
        } else {
            epkey = NewKey();
            epinfo = Variant(Variant::ObjectType);
            epinfo["key"] = epkey;
            epinfo["name"] = name;
            epinfo["nodekey"] = nodekey;
            epinfo["hostkey"] = nodeinfo["hostkey"];
            epinfo["live"] = true;
            datamap[epkey] = epinfo;
        }
        Variant reply = epinfo.Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
        SendMessage(sender, reply);
    }

    void RemoteDBServer::DestroyEndpointKey(const Variant &msg) {
        Key_t epkey = msg["key"].AsNumber<Key_t>();
        datamap[epkey]["live"] = false;
    }

    void RemoteDBServer::GetEndpointInfo(const std::string &sender, const Variant &msg) {
        Key_t epkey = msg["key"].AsNumber<Key_t>();
        Variant reply = datamap[epkey].Copy();
        reply["msgid"] = msg["msgid"];
        reply["msgtype"] = "reply";
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

}

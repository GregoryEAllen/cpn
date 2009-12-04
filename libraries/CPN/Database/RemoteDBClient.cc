
#include "RemoteDBClient.h"
#include "Assert.h"
#include "AutoUnlock.h"
#include "Base64.h"

namespace CPN {

    RemoteDBClient::RemoteDBClient()
        : trancounter(0)
    {
    }

    RemoteDBClient::~RemoteDBClient() {
    }

    Key_t RemoteDBClient::SetupHost(const std::string &name, const std::string &hostname,
            const std::string &servname, KernelMessageHandler *kmh) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_SETUP_HOST;
        msg["name"] = name;
        msg["hostname"] = hostname;
        msg["servname"] = servname;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        Key_t key = winfo.msg["key"].AsNumber<Key_t>();
        kmhandlers.insert(std::make_pair(key, kmh));
        return key;
    }

    Key_t RemoteDBClient::GetHostKey(const std::string &host) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["name"] = host;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["key"].AsNumber<Key_t>();
    }

    const std::string &RemoteDBClient::GetHostName(Key_t hostkey) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["key"] = hostkey;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["name"].AsString();
    }

    void RemoteDBClient::GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["key"] = hostkey;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        hostname = winfo.msg["hostname"].AsString();
        servname = winfo.msg["servname"].AsString();
    }

    void RemoteDBClient::DestroyHostKey(Key_t hostkey) {
        PthreadMutexProtected plock(lock);

        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_DESTROY_HOST_KEY;
        msg["key"] = hostkey;
        SendMessage(msg);
        kmhandlers.erase(hostkey);
    }

    Key_t RemoteDBClient::WaitForHostStart(const std::string &host) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);
        GenericWaiterPtr genwait = NewGenericWaiter();

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["name"] = host;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        if (winfo.msg["success"].IsTrue() && winfo.msg["live"].IsTrue()) {
            return winfo.msg["key"].AsNumber<Key_t>();
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["name"].AsString() == host && msg["live"].IsTrue()) {
                    return msg["key"].AsNumber<Key_t>();
                }
            }
        }
    }

    void RemoteDBClient::SignalHostStart(Key_t hostkey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["key"] = hostkey;
        msg["type"] = RDBMT_SIGNAL_HOST_START;
        SendMessage(msg);
    }

    void RemoteDBClient::SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr) {
        PthreadMutexProtected plock(lock);
        SendQueueMsg(hostkey, RDBMT_CREATE_WRITER, attr);
    }

    void RemoteDBClient::SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr) {
        PthreadMutexProtected plock(lock);
        SendQueueMsg(hostkey, RDBMT_CREATE_READER, attr);
    }

    void RemoteDBClient::SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr) {
        PthreadMutexProtected plock(lock);
        SendQueueMsg(hostkey, RDBMT_CREATE_QUEUE, attr);
    }

    void RemoteDBClient::SendCreateNode(Key_t hostkey, const NodeAttr &attr) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["msgtype"] = "kernel";
        msg["type"] = RDBMT_CREATE_NODE;
        msg["hostkey"] = hostkey;
        msg["host"] = attr.GetHost();
        msg["name"] = attr.GetName();
        msg["nodetype"] = attr.GetTypeName();
        msg["param"] = attr.GetParam();
        Base64Encoder encode;
        encode.EncodeBlock(attr.GetArg().GetBuffer(), attr.GetArg().GetSize());
        msg["arg"] = encode.BlockEnd();
        msg["key"] = attr.GetKey();
        SendMessage(msg);
    }

    NodeAttr MsgToNodeAttr(const Variant &msg) {
        NodeAttr attr(msg["name"].AsString(), msg["nodetype"].AsString());
        attr.SetHost(msg["host"].AsString()).SetParam(msg["param"].AsString());
        Base64Decoder decode;
        decode.DecodeBlock(msg["arg"].AsString());
        std::vector<char> arg = decode.BlockEnd();
        attr.SetParam(StaticConstBuffer(&arg[0], arg.size()));
        attr.SetKey(msg["key"].AsNumber<Key_t>());
        return attr;
    }

    void RemoteDBClient::SendQueueMsg(Key_t hostkey, RDBMT_t msgtype, const SimpleQueueAttr &attr) {
        Variant msg(Variant::ObjectType);
        msg["msgtype"] = "kernel";
        msg["type"] = msgtype;
        msg["hostkey"] = hostkey;
        msg["queuehint"] = attr.GetHint();
        msg["datatype"] = attr.GetDatatype();
        msg["queueLength"] = attr.GetLength();
        msg["maxThreshold"] = attr.GetMaxThreshold();
        msg["numChannels"] = attr.GetNumChannels();
        msg["readerkey"] = attr.GetReaderKey();
        msg["writerkey"] = attr.GetWriterKey();
        msg["readernodekey"] = attr.GetReaderNodeKey();
        msg["writernodekey"] = attr.GetWriterNodeKey();
        SendMessage(msg);
    }

    SimpleQueueAttr MsgToQueueAttr(const Variant &msg) {
        SimpleQueueAttr attr;
        attr.SetHint(msg["queuehint"].AsNumber<CPN::QueueHint_t>());
        attr.SetDatatype(msg["datatype"].AsString());
        attr.SetLength(msg["queueLength"].AsUnsigned());
        attr.SetMaxThreshold(msg["maxThreshold"].AsUnsigned());
        attr.SetNumChannels(msg["numChannels"].AsUnsigned());
        attr.SetReaderKey(msg["readerkey"].AsNumber<Key_t>());
        attr.SetWriterKey(msg["writerkey"].AsNumber<Key_t>());
        attr.SetReaderNodeKey(msg["readernodekey"].AsNumber<Key_t>());
        attr.SetWriterNodeKey(msg["writernodekey"].AsNumber<Key_t>());
        return attr;
    }

    Key_t RemoteDBClient::CreateNodeKey(Key_t hostkey, const std::string &nodename) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_CREATE_NODE_KEY;
        msg["hostkey"] = hostkey;
        msg["name"] = nodename;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["key"].AsNumber<Key_t>();
    }

    Key_t RemoteDBClient::GetNodeKey(const std::string &nodename) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["name"] = nodename;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["key"].AsNumber<Key_t>();
    }

    const std::string &RemoteDBClient::GetNodeName(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["key"] = nodekey;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["name"].AsString();
    }

    void RemoteDBClient::SignalNodeStart(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_SIGNAL_NODE_START;
        msg["key"] = nodekey;
        SendMessage(msg);
    }

    void RemoteDBClient::SignalNodeEnd(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_SIGNAL_NODE_END;
        msg["key"] = nodekey;
        SendMessage(msg);
    }

    Key_t RemoteDBClient::WaitForNodeStart(const std::string &nodename) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);
        GenericWaiterPtr genwait = NewGenericWaiter();

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["name"] = nodename;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        if (winfo.msg["success"].IsTrue() && winfo.msg["started"].IsTrue()) {
            return winfo.msg["key"].AsNumber<Key_t>();
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["started"].IsTrue() && msg["name"].AsString() == nodename) {
                    return msg["key"].AsNumber<Key_t>();
                }
            }
        }
    }

    void RemoteDBClient::WaitForNodeEnd(const std::string &nodename) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);
        GenericWaiterPtr genwait = NewGenericWaiter();

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["name"] = nodename;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        if (winfo.msg["success"].IsTrue() && winfo.msg["dead"].IsTrue()) {
            return;
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["dead"].IsTrue() && msg["name"].AsString() == nodename) {
                    return;
                }
            }
        }
    }

    void RemoteDBClient::WaitForAllNodeEnd() {
        PthreadMutexProtected plock(lock);
        GenericWaiterPtr genwait = NewGenericWaiter();
        while (true) {
            WaiterInfo winfo(NewTranID());
            AddWaiter(&winfo);
            Variant msg(Variant::ObjectType);
            msg["msgid"] = winfo.waiterid;
            msg["type"] = RDBMT_GET_NUM_NODE_LIVE;
            SendMessage(msg);
            while (!winfo.signaled) {
                winfo.cond.Wait(lock);
            }
            ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
            if (winfo.msg["numlivenodes"].AsUnsigned() == 0) {
                return;
            }
            while (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
            }
            genwait->messages.clear();
        }
    }

    Key_t RemoteDBClient::GetNodeHost(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["key"] = nodekey;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["hostkey"].AsNumber<Key_t>();
    }


    Key_t RemoteDBClient::GetCreateReaderKey(Key_t nodekey, const std::string &portname) {
        PthreadMutexProtected plock(lock);
        return GetCreateEndpointKey(RDBMT_GET_CREATE_READER_KEY, nodekey, portname);
    }

    Key_t RemoteDBClient::GetReaderNode(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_READER_INFO, portkey);
        return info["nodekey"].AsNumber<Key_t>();
    }

    Key_t RemoteDBClient::GetReaderHost(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_READER_INFO, portkey);
        return info["hostkey"].AsNumber<Key_t>();
    }

    const std::string &RemoteDBClient::GetReaderName(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_READER_INFO, portkey);
        return info["name"].AsString();
    }

    void RemoteDBClient::DestroyReaderKey(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_DESTROY_READER_KEY;
        msg["key"] = portkey;
        SendMessage(msg);
    }

    Key_t RemoteDBClient::GetCreateWriterKey(Key_t nodekey, const std::string &portname) {
        PthreadMutexProtected plock(lock);
        return GetCreateEndpointKey(RDBMT_GET_CREATE_WRITER_KEY, nodekey, portname);
    }

    Key_t RemoteDBClient::GetWriterNode(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_WRITER_INFO, portkey);
        return info["nodekey"].AsNumber<Key_t>();
    }

    Key_t RemoteDBClient::GetWriterHost(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_WRITER_INFO, portkey);
        return info["hostkey"].AsNumber<Key_t>();
    }

    const std::string &RemoteDBClient::GetWriterName(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_WRITER_INFO, portkey);
        return info["name"].AsString();
    }

    void RemoteDBClient::DestroyWriterKey(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_DESTROY_WRITER_KEY;
        msg["key"] = portkey;
        SendMessage(msg);
    }

    void RemoteDBClient::ConnectEndpoints(Key_t writerkey, Key_t readerkey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_CONNECT_ENDPOINTS;
        msg["readerkey"] = readerkey;
        msg["writerkey"] = writerkey;
        SendMessage(msg);
    }

    Key_t RemoteDBClient::GetReadersWriter(Key_t readerkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_READER_INFO, readerkey);
        return info["writerkey"].AsNumber<Key_t>();
    }

    Key_t RemoteDBClient::GetWritersReader(Key_t writerkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_WRITER_INFO, writerkey);
        return info["readerkey"].AsNumber<Key_t>();
    }

    void RemoteDBClient::AddWaiter(WaiterInfo *info) {
        callwaiters.insert(std::make_pair(info->waiterid, info));
    }

    RemoteDBClient::GenericWaiterPtr RemoteDBClient::NewGenericWaiter() {
        GenericWaiterPtr gw = GenericWaiterPtr(new GenericWaiter);
        waiters.push_back(gw);
        return gw;
    }

    unsigned RemoteDBClient::NewTranID() {
        return ++trancounter;
    }

    void RemoteDBClient::DispatchMessage(const Variant &msg) {
        PthreadMutexProtected plock(lock);
        // Need to add for the general informative messages.
        //
        // The specific reply messages
        std::string msgtype = msg["msgtype"].AsString();
        if (msgtype == "reply") {
            unsigned tranid = msg["msgid"].AsUnsigned();
            std::map<unsigned, WaiterInfo*>::iterator entry;
            entry = callwaiters.find(tranid);
            ASSERT(entry != callwaiters.end());
            entry->second->msg = msg;
            entry->second->signaled = true;
            entry->second->cond.Signal();
            callwaiters.erase(entry);
        } else if (msgtype == "broadcast") {
            std::list<std::tr1::weak_ptr<GenericWaiter> >::iterator entry;
            entry = waiters.begin();
            while (entry != waiters.end()) {
                GenericWaiterPtr waiter = entry->lock();
                if (waiter) {
                    waiter->messages.push_back(msg);
                    waiter->cond.Signal();
                    ++entry;
                } else {
                    entry = waiters.erase(entry);
                }
            }
        } else if (msgtype == "kernel") {
            Key_t hostkey = msg["hostkey"].AsNumber<Key_t>();
            std::map<Key_t, KernelMessageHandler*>::iterator entry = kmhandlers.find(hostkey);
            if (entry != kmhandlers.end()) {
                KernelMessageHandler *kmh = entry->second;
                // Release the lock (but get it back once we leave scope)
                // We can't call the kernel with the lock
                AutoUnlock<PthreadMutex> aul(lock);
                ASSERT(kmh);
                switch (msg["type"].AsNumber<RDBMT_t>()) {
                case RDBMT_CREATE_NODE:
                    kmh->CreateNode(hostkey, MsgToNodeAttr(msg));
                    break;
                case RDBMT_CREATE_QUEUE:
                    kmh->CreateQueue(hostkey, MsgToQueueAttr(msg));
                    break;
                case RDBMT_CREATE_WRITER:
                    kmh->CreateWriter(hostkey, MsgToQueueAttr(msg));
                    break;
                case RDBMT_CREATE_READER:
                    kmh->CreateReader(hostkey, MsgToQueueAttr(msg));
                    break;
                default:
                    ASSERT(false);
                }
            } else {
                printf("No kernel for message\n");
            }
        } else {
            ASSERT(false);
        }
    }

    Key_t RemoteDBClient::GetCreateEndpointKey(RDBMT_t msgtype, Key_t nodekey, const std::string &portname) {
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = msgtype;
        msg["nodekey"] = nodekey;
        msg["name"] = portname;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg["key"].AsNumber<Key_t>();
    }

    Variant RemoteDBClient::GetEndpointInfo(RDBMT_t msgtype, Key_t portkey) {
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);

        Variant msg(Variant::ObjectType);
        msg["msgid"] = winfo.waiterid;
        msg["type"] = msgtype;
        msg["key"] = portkey;
        SendMessage(msg);

        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
        }
        ASSERT(winfo.msg["success"].IsTrue(), "msg: %s", winfo.msg.AsJSON().c_str());
        return winfo.msg;
    }
}

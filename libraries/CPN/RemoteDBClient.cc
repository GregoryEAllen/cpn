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

#include "RemoteDBClient.h"
#include "KernelBase.h"
#include "Exceptions.h"
#include "Assert.h"
#include "AutoUnlock.h"
#include "Base64.h"
#include "VariantToJSON.h"
#include "PthreadFunctional.h"
#include "ErrnoException.h"
#include <stdexcept>

namespace CPN {

    RemoteDBClient::RemoteDBClient()
        : trancounter(0), shutdown(false), loglevel(Logger::WARNING)
    {
    }

    RemoteDBClient::~RemoteDBClient() {
    }

    int RemoteDBClient::LogLevel() const {
        PthreadMutexProtected plock(lock);
        return loglevel;
    }

    int RemoteDBClient:: LogLevel(int level) {
        PthreadMutexProtected plock(lock);
        return loglevel = level;
    }

    void RemoteDBClient::Log(int level, const std::string &logmsg) {
        PthreadMutexProtected plock(lock);
        if (level >= loglevel) {
            Variant msg(Variant::ObjectType);
            msg["type"] = RDBMT_LOG;
            msg["msg"] = logmsg;
            SendMessage(msg);
        }
    }

    Key_t RemoteDBClient::SetupHost(const std::string &name, const std::string &hostname,
            const std::string &servname, KernelBase *kernel) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        if (!kernel) { throw std::invalid_argument("Must have non null Kernel."); }
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_SETUP_HOST;
        msg["name"] = name;
        msg["hostname"] = hostname;
        msg["servname"] = servname;
        Variant reply = RemoteCall(msg);
        if (!reply["success"].IsTrue()) {
           throw std::invalid_argument("Cannot create two kernels with the same name");
        }
        Key_t key = reply["hostinfo"]["key"].AsNumber<Key_t>();
        kernels.insert(std::make_pair(key, kernel));
        return key;
    }

    Key_t RemoteDBClient::GetHostKey(const std::string &host) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["name"] = host;
        Variant reply = RemoteCall(msg);
        if (!reply["success"].IsTrue()) {
            throw std::invalid_argument("No such host");
        }
        return reply["hostinfo"]["key"].AsNumber<Key_t>();
    }

    std::string RemoteDBClient::GetHostName(Key_t hostkey) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["key"] = hostkey;
        Variant reply = RemoteCall(msg);
        if (!reply["success"].IsTrue()) {
            throw std::invalid_argument("No such host");
        }
        return reply["hostinfo"]["name"].AsString();
    }

    void RemoteDBClient::GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["key"] = hostkey;
        Variant reply = RemoteCall(msg);
        if (!reply["success"].IsTrue()) {
            throw std::invalid_argument("No such host");
        }
        Variant hostinfo = reply["hostinfo"];
        hostname = hostinfo["hostname"].AsString();
        servname = hostinfo["servname"].AsString();
    }

    void RemoteDBClient::SignalHostEnd(Key_t hostkey) {
        PthreadMutexProtected plock(lock);
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_SIGNAL_HOST_END;
        msg["key"] = hostkey;
        SendMessage(msg);
        kernels.erase(hostkey);
    }

    Key_t RemoteDBClient::WaitForHostStart(const std::string &host) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        GenericWaiterPtr genwait = NewGenericWaiter();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_HOST_INFO;
        msg["name"] = host;
        Variant reply = RemoteCall(msg);
        if (reply["success"].IsTrue()) {
            Variant hostinfo = reply["hostinfo"];
            if (hostinfo["live"].IsTrue()) {
                return hostinfo["key"].AsNumber<Key_t>();
            }
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
                InternalCheckTerminated();
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["hostinfo"].IsObject()) {
                    Variant hostinfo = msg["hostinfo"];
                    if (hostinfo["name"].AsString() == host && hostinfo["live"].IsTrue()) {
                        return hostinfo["key"].AsNumber<Key_t>();
                    }
                }
            }
        }
    }

    void RemoteDBClient::SignalHostStart(Key_t hostkey) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["key"] = hostkey;
        msg["type"] = RDBMT_SIGNAL_HOST_START;
        SendMessage(msg);
    }

    void RemoteDBClient::SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        SendQueueMsg(hostkey, RDBMT_CREATE_WRITER, attr);
    }

    void RemoteDBClient::SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        SendQueueMsg(hostkey, RDBMT_CREATE_READER, attr);
    }

    void RemoteDBClient::SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        SendQueueMsg(hostkey, RDBMT_CREATE_QUEUE, attr);
    }

    void RemoteDBClient::SendCreateNode(Key_t hostkey, const NodeAttr &attr) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["msgtype"] = "kernel";
        msg["type"] = RDBMT_CREATE_NODE;
        msg["hostkey"] = hostkey;
        Variant nodeattr;
        nodeattr["hostkey"] = hostkey;
        nodeattr["host"] = attr.GetHost();
        nodeattr["name"] = attr.GetName();
        nodeattr["nodetype"] = attr.GetTypeName();
        nodeattr["param"] = attr.GetParam();
        Base64Encoder encode(0);
        encode.EncodeBlock(attr.GetArg().GetBuffer(), attr.GetArg().GetSize());
        nodeattr["arg"] = encode.BlockEnd();
        nodeattr["key"] = attr.GetKey();
        msg["nodeattr"] = nodeattr;
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
        Variant queueattr;
        queueattr["queuehint"] = attr.GetHint();
        queueattr["datatype"] = attr.GetDatatype();
        queueattr["queueLength"] = attr.GetLength();
        queueattr["maxThreshold"] = attr.GetMaxThreshold();
        queueattr["numChannels"] = attr.GetNumChannels();
        queueattr["readerkey"] = attr.GetReaderKey();
        queueattr["writerkey"] = attr.GetWriterKey();
        queueattr["readernodekey"] = attr.GetReaderNodeKey();
        queueattr["writernodekey"] = attr.GetWriterNodeKey();
        msg["queueattr"] = queueattr;
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
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_CREATE_NODE_KEY;
        msg["hostkey"] = hostkey;
        msg["name"] = nodename;
        Variant reply = RemoteCall(msg);
        if (!(reply["success"].IsTrue())) {
            throw std::invalid_argument("Node " + nodename + " already exists.");
        }
        return reply["nodeinfo"]["key"].AsNumber<Key_t>();
    }

    Key_t RemoteDBClient::GetNodeKey(const std::string &nodename) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["name"] = nodename;
        Variant reply = RemoteCall(msg);
        if (!(reply["success"].IsTrue())) {
            throw std::invalid_argument("No such node");
        }
        ASSERT(reply["success"].IsTrue(), "msg: %s", VariantToJSON(reply).c_str());
        return reply["nodeinfo"]["key"].AsNumber<Key_t>();
    }

    std::string RemoteDBClient::GetNodeName(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["key"] = nodekey;
        Variant reply = RemoteCall(msg);
        if (!(reply["success"].IsTrue())) {
            throw std::invalid_argument("No such node");
        }
        return reply["nodeinfo"]["name"].AsString();
    }

    void RemoteDBClient::SignalNodeStart(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
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
        InternalCheckTerminated();
        GenericWaiterPtr genwait = NewGenericWaiter();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["name"] = nodename;
        Variant reply = RemoteCall(msg);

        if (reply["success"].IsTrue()) {
            Variant nodeinfo = reply["nodeinfo"];
            if (nodeinfo["started"].IsTrue()) {
                return nodeinfo["key"].AsNumber<Key_t>();
            }
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
                InternalCheckTerminated();
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["nodeinfo"].IsObject()) {
                    Variant nodeinfo = msg["nodeinfo"];
                    if (nodeinfo["started"].IsTrue() && nodeinfo["name"].AsString() == nodename) {
                        return nodeinfo["key"].AsNumber<Key_t>();
                    }
                }
            }
        }
    }

    void RemoteDBClient::WaitForNodeEnd(const std::string &nodename) {
        PthreadMutexProtected plock(lock);
        if (shutdown) { return; }
        GenericWaiterPtr genwait = NewGenericWaiter();

        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["name"] = nodename;
        Variant reply;
       try {
          reply  = RemoteCall(msg);
       } catch (const ShutdownException &e) {
           return;
       }
        if (reply["success"].IsTrue()) {
            if (reply["nodeinfo"]["dead"].IsTrue()) {
                return;
            }
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
                if (shutdown) { return; }
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["nodeinfo"].IsObject()) {
                    Variant nodeinfo = msg["nodeinfo"];
                    if (nodeinfo["dead"].IsTrue() && nodeinfo["name"].AsString() == nodename) {
                        return;
                    }
                }
            }
        }
    }

    void RemoteDBClient::WaitForAllNodeEnd() {
        PthreadMutexProtected plock(lock);
        if (shutdown) { return; }
        GenericWaiterPtr genwait = NewGenericWaiter();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_NUM_NODE_LIVE;
        Variant reply;
        try {
            reply = RemoteCall(msg);
        } catch (const ShutdownException &e) {
            return;
        }
        ASSERT(reply["success"].IsTrue(), "msg: %s", VariantToJSON(reply).c_str());
        if (reply["numlivenodes"].AsUnsigned() == 0) {
            return;
        }
        while (true) {
            if (genwait->messages.empty()) {
                genwait->cond.Wait(lock);
                if (shutdown) { return; }
            } else {
                Variant msg = genwait->messages.front();
                genwait->messages.pop_front();
                if (msg["numlivenodes"].IsNumber()) {
                    if (msg["numlivenodes"].AsUnsigned() == 0) {
                        return;
                    }
                }
            }
        }
    }

    Key_t RemoteDBClient::GetNodeHost(Key_t nodekey) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = RDBMT_GET_NODE_INFO;
        msg["key"] = nodekey;
        Variant reply = RemoteCall(msg);
        if (!(reply["success"].IsTrue())) {
            throw std::invalid_argument("No such node");
        }
        return reply["nodeinfo"]["hostkey"].AsNumber<Key_t>();
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

    std::string RemoteDBClient::GetReaderName(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_READER_INFO, portkey);
        return info["name"].AsString();
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

    std::string RemoteDBClient::GetWriterName(Key_t portkey) {
        PthreadMutexProtected plock(lock);
        Variant info = GetEndpointInfo(RDBMT_GET_WRITER_INFO, portkey);
        return info["name"].AsString();
    }

    void RemoteDBClient::ConnectEndpoints(Key_t writerkey, Key_t readerkey) {
        PthreadMutexProtected plock(lock);
        InternalCheckTerminated();
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

    void RemoteDBClient::Terminate() {
        PthreadMutexProtected plock(lock);
        if (!shutdown) {
            InternalTerminate();
            Variant msg(Variant::ObjectType);
            msg["type"] = RDBMT_TERMINATE;
            SendMessage(msg);
        }
    }

    void *RemoteDBClient::TerminateThread() {
        KernelMap mapcopy;
        {
            PthreadMutexProtected plock(lock);
            mapcopy = kernels;
        }
        KernelMap::iterator itr, end;
        itr = mapcopy.begin();
        end = mapcopy.end();
        while (itr != end) {
            itr->second->NotifyTerminate();
            ++itr;
        }
        return 0;
    }

    void RemoteDBClient::InternalTerminate() {
        shutdown = true;
        WaiterMap::iterator cwitr = callwaiters.begin();
        while (cwitr != callwaiters.end()) {
            cwitr->second->cond.Signal();
            ++cwitr;
        }
        WaiterList::iterator gwitr;
        gwitr = waiters.begin();
        while (gwitr != waiters.end()) {
            GenericWaiterPtr ptr = gwitr->lock();
            if (ptr) {
                ptr->cond.Signal();
            }
            ++gwitr;
        }
        terminateThread.reset(CreatePthreadFunctional(this, &RemoteDBClient::TerminateThread));
        terminateThread->Start();
    }

    bool RemoteDBClient::IsTerminated() {
        PthreadMutexProtected plock(lock);
        return shutdown;
    }

    bool RemoteDBClient::RequireRemote() {
        return true;
    }

    void RemoteDBClient::InternalCheckTerminated() {
        if (shutdown) {
            throw ShutdownException();
        }
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
        AutoLock<PthreadMutex> plock(lock);
        if (msg["type"].AsNumber<RDBMT_t>() == RDBMT_TERMINATE) {
            InternalTerminate();
            return;
        }
        // Need to add for the general informative messages.
        //
        // The specific reply messages
        std::string msgtype = msg["msgtype"].AsString();
        if (msgtype == "reply") {
            unsigned tranid = msg["msgid"].AsUnsigned();
            WaiterMap::iterator entry;
            entry = callwaiters.find(tranid);
            ASSERT(entry != callwaiters.end());
            entry->second->msg = msg;
            entry->second->signaled = true;
            entry->second->cond.Signal();
            callwaiters.erase(entry);
        } else if (msgtype == "broadcast") {
            WaiterList::iterator entry;
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
            KernelMap::iterator entry = kernels.find(hostkey);
            if (entry != kernels.end()) {
                KernelBase *kernel = entry->second;
                // Cannot call a kernel method with the lock.
                plock.Unlock();
                ASSERT(kernel);
                switch (msg["type"].AsNumber<RDBMT_t>()) {
                case RDBMT_CREATE_NODE:
                    kernel->RemoteCreateNode(MsgToNodeAttr(msg["nodeattr"]));
                    break;
                case RDBMT_CREATE_QUEUE:
                    kernel->RemoteCreateQueue(MsgToQueueAttr(msg["queueattr"]));
                    break;
                case RDBMT_CREATE_WRITER:
                    kernel->RemoteCreateWriter(MsgToQueueAttr(msg["queueattr"]));
                    break;
                case RDBMT_CREATE_READER:
                    kernel->RemoteCreateReader(MsgToQueueAttr(msg["queueattr"]));
                    break;
                default:
                    ASSERT(false, "Unknown kernel message type");
                }
            } else {
                ASSERT(false, "Message for kernel %llu but said kernel doesn't exist!", hostkey);
            }
        } else {
            ASSERT(false);
        }
    }

    Key_t RemoteDBClient::GetCreateEndpointKey(RDBMT_t msgtype, Key_t nodekey, const std::string &portname) {
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = msgtype;
        msg["nodekey"] = nodekey;
        msg["name"] = portname;
        Variant reply = RemoteCall(msg);
        if (!reply["success"].IsTrue()) {
            throw std::invalid_argument("No such port");
        }
        return reply["endpointinfo"]["key"].AsNumber<Key_t>();
    }

    Variant RemoteDBClient::GetEndpointInfo(RDBMT_t msgtype, Key_t portkey) {
        InternalCheckTerminated();
        Variant msg(Variant::ObjectType);
        msg["type"] = msgtype;
        msg["key"] = portkey;
        Variant reply = RemoteCall(msg);
        ASSERT(reply["success"].IsTrue(), "msg: %s", VariantToJSON(reply).c_str());
        return reply["endpointinfo"];
    }

    Variant RemoteDBClient::RemoteCall(Variant msg) {
        WaiterInfo winfo(NewTranID());
        AddWaiter(&winfo);
        msg["msgid"] = winfo.waiterid;
        SendMessage(msg);
        while (!winfo.signaled) {
            winfo.cond.Wait(lock);
            InternalCheckTerminated();
        }
        return winfo.msg;
    }
}

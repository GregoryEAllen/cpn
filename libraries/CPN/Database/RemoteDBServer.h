
#pragma once
#include "CPNCommon.h"
#include "Variant.h"
#include "PthreadMutex.h"

/*
 
    hostinfo = {
        "key"       : key for the host,
        "name"      : name of the host,
        "hostname"  : name to use for other kernels to connect to this one,
        "servname"  : service name to use to connecto to this kernel,
        "live"      : true or false,
        "type"      : "endpointinfo"
    }

    nodeinfo = {
        "key"       : the node key,
        "name"      : name of this node,
        "hostkey"   : the host key,
        "started"   : true or false,
        "dead"      : true or false,
        "endpoints" : {
            "endpointname" : key of endpoint,
            ...
        },
        "type"      : "endpointinfo"
    }

    endpointinfo = {
        "key"       : The key for this endpoint,
        "name"      : the name of this endpoint,
        "nodekey"   : The key of the node this endpoint belongs to,
        "hostkey"   : the key of the host this node endpoint is on,
        "live"      : true or false
        "writerkey" : the key of the writer this endpoint is connected to if a reader,
        "readerkey" : the key of the reader this endpoint is connected to if a writer,
        "type"      : "endpointinfo"
    }
*/
namespace CPN {
    class RemoteDBServer {
    public:
        RemoteDBServer();
        virtual ~RemoteDBServer();
    protected:
        void DispatchMessage(const std::string &sender, const Variant &msg);
        virtual void SendMessage(const std::string &recipient, const Variant &msg) = 0;
        virtual void BroadcastMessage(const Variant &msg) = 0;
    private:

        void SetupHost(const std::string &sender, const Variant &msg);
        void SignalHostStart(const Variant &msg);
        void DestroyHostKey(const Variant &msg);
        void GetHostInfo(const std::string &sender, const Variant &msg);
        void CreateNodeKey(const std::string &sender, const Variant &msg);
        void SignalNodeStart(const Variant &msg);
        void SignalNodeEnd(const Variant &msg);
        void GetNodeInfo(const std::string &sender, const Variant &msg);
        void GetNumNodeLive(const std::string &sender, const Variant &msg);
        void GetCreateEndpointKey(const std::string &sender, const Variant &msg);
        void DestroyEndpointKey(const Variant &msg);
        void GetEndpointInfo(const std::string &sender, const Variant &msg);
        void ConnectEndpoints(const Variant &msg);

        Key_t NewKey();

        typedef std::map<Key_t, Variant> DataMap;
        DataMap datamap;
        typedef std::map<std::string, Key_t> NameKeyMap;
        NameKeyMap hostmap;
        NameKeyMap nodemap;

        unsigned numlivenodes;

        Key_t keycount;
    };
}


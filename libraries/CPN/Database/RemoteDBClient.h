
#pragma once

#include "RDBMT.h"
#include "Database.h"
#include "QueueAttr.h"
#include "NodeAttr.h"
#include "Message.h"
#include "Pthread.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "Variant.h"
#include <list>
#include <tr1/memory>

namespace CPN {

    class RemoteDBClient : public Database {
    public:
        virtual ~RemoteDBClient();
        virtual CPN::Key_t SetupHost(const std::string &name, const std::string &hostname,
                const std::string &servname, CPN::KernelMessageHandler *kmh);
        virtual CPN::Key_t GetHostKey(const std::string &host);
        virtual const std::string &GetHostName(CPN::Key_t hostkey);
        virtual void GetHostConnectionInfo(CPN::Key_t hostkey, std::string &hostname, std::string &servname);
        virtual void DestroyHostKey(CPN::Key_t hostkey);
        virtual CPN::Key_t WaitForHostSetup(const std::string &host);

        virtual void SendCreateWriter(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr);
        virtual void SendCreateReader(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr);
        virtual void SendCreateQueue(CPN::Key_t hostkey, const CPN::SimpleQueueAttr &attr);
        virtual void SendCreateNode(CPN::Key_t hostkey, const CPN::NodeAttr &attr);

        virtual CPN::Key_t CreateNodeKey(CPN::Key_t hostkey, const std::string &nodename);
        virtual CPN::Key_t GetNodeKey(const std::string &nodename);
        virtual const std::string &GetNodeName(CPN::Key_t nodekey);
        virtual CPN::Key_t GetNodeHost(CPN::Key_t nodekey);
        virtual void SignalNodeStart(CPN::Key_t nodekey);
        virtual void SignalNodeEnd(CPN::Key_t nodekey);

        /** Waits until the node starts and returns the key, if the node is
         * already started returns the key
         */
        virtual CPN::Key_t WaitForNodeStart(const std::string &nodename);
        virtual void WaitForNodeEnd(const std::string &nodename);
        virtual void WaitForAllNodeEnd();


        virtual CPN::Key_t GetCreateReaderKey(CPN::Key_t nodekey, const std::string &portname);
        virtual CPN::Key_t GetReaderNode(CPN::Key_t portkey);
        virtual CPN::Key_t GetReaderHost(CPN::Key_t portkey);
        virtual const std::string &GetReaderName(CPN::Key_t portkey);
        virtual void DestroyReaderKey(CPN::Key_t portkey);

        virtual CPN::Key_t GetCreateWriterKey(CPN::Key_t nodekey, const std::string &portname);
        virtual CPN::Key_t GetWriterNode(CPN::Key_t portkey);
        virtual CPN::Key_t GetWriterHost(CPN::Key_t portkey);
        virtual const std::string &GetWriterName(CPN::Key_t portkey);
        virtual void DestroyWriterKey(CPN::Key_t portkey);

        virtual void ConnectEndpoints(CPN::Key_t writerkey, CPN::Key_t readerkey);
        virtual CPN::Key_t GetReadersWriter(CPN::Key_t readerkey);
        virtual CPN::Key_t GetWritersReader(CPN::Key_t writerkey);

    protected:
        RemoteDBClient();
        void DispatchMessage(const Variant &msg);
        virtual void SendMessage(const Variant &msg) = 0;
        PthreadMutex lock;
    private:

        struct WaiterInfo {
            WaiterInfo(unsigned wid) : waiterid(wid), signaled(false) {}
            unsigned waiterid;
            PthreadCondition cond;
            bool signaled;
            Variant msg;
        };

        struct GenericWaiter {
            PthreadCondition cond;
            std::list<Variant> messages;
        };
        typedef std::tr1::shared_ptr<GenericWaiter> GenericWaiterPtr;

        void SendQueueMsg(CPN::Key_t hostkey, RDBMT_t msgtype, const CPN::SimpleQueueAttr &attr);
        unsigned NewTranID();
        void AddWaiter(WaiterInfo *info);
        GenericWaiterPtr NewGenericWaiter();

        CPN::Key_t GetCreateEndpointKey(RDBMT_t msgtype, CPN::Key_t nodekey, const std::string &portname);
        Variant GetEndpointInfo(RDBMT_t msgtype, CPN::Key_t portkey);

        std::map<unsigned, WaiterInfo*> callwaiters;
        std::list<std::tr1::weak_ptr<GenericWaiter> > waiters;
        std::map<CPN::Key_t, CPN::KernelMessageHandler*> kmhandlers;
        unsigned trancounter;
    };
}


#pragma once

#include "Variant.h"

#include "D4RTesterBase.h"
#include "D4RTestQueue.h"
#include "D4RTestNode.h"
#include "PthreadMutex.h"
#include <map>

namespace D4R {
    class Tester : public TesterBase {
    public:
        Tester();
        ~Tester();

        void Deadlock(TestNodeBase *tnb) { Abort(); }

        void Complete(TestNodeBase *tnb) {}
        void Failure(TestNodeBase *tnb, const std::string &msg);

        void Abort();

        void Run();

        void Report();
    protected:
        virtual void CreateNode(const Variant &noded);
        virtual void CreateQueue(const Variant &queued);

    private:
        void PrintNodes();
        typedef std::map<std::string, TestNode*> NodeMap;
    public:
        struct QueueInfo {
            shared_ptr<TestQueue> queue;
            TestNode *reader;
            TestNode *writer;
        };
    private:
        PthreadMutex lock;
        typedef std::map<std::string, QueueInfo> QueueMap;
        NodeMap nodemap;
        QueueMap queuemap;
    };
}



#pragma once

#include "Variant.h"

#include "D4RTesterBase.h"
#include "D4RTestQueue.h"
#include "D4RTestNode.h"
#include <map>

namespace D4R {
    class Tester : public TesterBase {
    public:
        Tester();
        ~Tester();

        void Deadlock(TestNodeBase *tnb) { Abort(); }

        void Complete(TestNodeBase *tnb) {}

        void Abort();

        void Run();
    protected:
        virtual void CreateNode(const Variant &noded);
        virtual void CreateQueue(const Variant &queued);

    private:
        void PrintNodes();
        typedef std::map<std::string, TestNode*> NodeMap;
        typedef std::map<std::string, TestQueue*> QueueMap;
        NodeMap nodemap;
        QueueMap queuemap;
    };
}



#pragma once

#include "D4RTestNodeBase.h"
#include "PthreadLib.h"
#include "PthreadCondition.h"
#include "Variant.h"
#include <map>
#include <deque>

namespace D4R {
    class TestQueue;
    class TesterBase;

    class TestNode
        : public Pthread,
        public TestNodeBase
    {
    public:

        TestNode(const std::string &name_, uint64_t k, TesterBase *tb);
        TestNode(const Variant &noded, TesterBase *tb);
        ~TestNode();

        Tag GetPublicTag() const { return node->GetPublicTag(); }
        Tag GetPrivateTag() const { return node->GetPrivateTag(); }

        void AddReadQueue(shared_ptr<TestQueue> q);
        void AddWriteQueue(shared_ptr<TestQueue> q);

        const std::string &GetName() const { return name; }

        void PrintNode();
    private:
        void *EntryPoint();

        void Enqueue(const std::string &qname, unsigned amount);
        void Dequeue(const std::string &qname, unsigned amount);
        void VerifyReaderSize(const std::string &qname, unsigned amount);
        void VerifyWriterSize(const std::string &qname, unsigned amount);

        PthreadMutex lock;
        const std::string name;
        typedef std::map<std::string, shared_ptr<TestQueue> > QueueMap;
        QueueMap readermap;
        QueueMap writermap;
        shared_ptr<Node> node;
    };
}

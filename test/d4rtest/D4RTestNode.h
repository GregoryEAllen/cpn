
#pragma once

#include "D4RTestNodeBase.h"
#include "Pthread.h"
#include "PthreadCondition.h"
#include "Variant.h"
#include <map>
#include <deque>

namespace D4R {
    class TestQueue;
    class TesterBase;

    class TestNode
        : public Pthread,
        public TestNodeBase,
        public Node
    {
    public:

        TestNode(const std::string &name_, uint64_t k, TesterBase *tb);
        TestNode(const Variant &noded, TesterBase *tb);

        void SignalTagChanged();

        void AddReadQueue(TestQueue *q);
        void AddWriteQueue(TestQueue *q);

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
        typedef std::map<std::string, TestQueue*> QueueMap;
        QueueMap readermap;
        QueueMap writermap;
    };
}

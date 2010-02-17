
#include "D4RTestNode.h"
#include "D4RTestQueue.h"
#include "D4RTester.h"
#include "D4RDeadlockException.h"
#include "Assert.h"
#include <sstream>

namespace D4R {


    TestNode::TestNode(const std::string &name_, uint64_t k, TesterBase *tb)
        : TestNodeBase(tb), Node(k), name(name_)
    {
        Logger::Name(name);
    }

    TestNode::TestNode(const Variant &noded, TesterBase *tb)
        : TestNodeBase(tb),
        Node(noded["key"].AsNumber<uint64_t>()),
        name(noded["name"].AsString())
    {
        Logger::Name(name);
        Variant::ConstListIterator itr = noded["instructions"].ListBegin();
        while (itr != noded["instructions"].ListEnd()) {
            AddOp(*itr);
            ++itr;
        }
    }

    void TestNode::SignalTagChanged() {
        PthreadMutexProtected al(lock);
        QueueMap::iterator itr = readermap.begin();
        while (itr != readermap.end()) {
            (itr++)->second->SignalReaderTagChanged();
        }
        itr = writermap.begin();
        while (itr != writermap.end()) {
            (itr++)->second->SignalWriterTagChanged();
        }
    }

    void TestNode::AddReadQueue(TestQueue *q) {
        PthreadMutexProtected al(lock);
        q->SetReaderNode(this);
        readermap.insert(std::make_pair(q->GetName(), q));
    }

    void TestNode::AddWriteQueue(TestQueue *q) {
        PthreadMutexProtected al(lock);
        q->SetWriterNode(this);
        writermap.insert(std::make_pair(q->GetName(), q));
    }

    void *TestNode::EntryPoint() {
        try {
            Run();
        } catch (const TestQueueAbortException &e) {
        } catch (const DeadlockException &e) {
        }
        return 0;
    }

    void TestNode::Enqueue(const std::string &qname, unsigned amount) {
        TestQueue *q;
        {
            PthreadMutexProtected al(lock);
            q  = writermap[qname];
        }
        q->Enqueue(amount);
    }

    void TestNode::Dequeue(const std::string &qname, unsigned amount) {
        TestQueue *q;
        {
            PthreadMutexProtected al(lock);
            q  = readermap[qname];
        }
        q->Dequeue(amount);
    }

    void TestNode::VerifyReaderSize(const std::string &qname, unsigned amount) {
        TestQueue *q;
        {
            PthreadMutexProtected al(lock);
            q  = readermap[qname];
        }
        if (amount != q->QueueSize()) {
            std::ostringstream oss;
            oss << "Queue " << qname << " not expected size! was: " << q->QueueSize()
                << " expected: " << amount;
            testerbase->Failure(this, oss.str());
        }
    }

    void TestNode::VerifyWriterSize(const std::string &qname, unsigned amount) {
        TestQueue *q;
        {
            PthreadMutexProtected al(lock);
            q  = writermap[qname];
        }
        if (amount != q->QueueSize()) {
            std::ostringstream oss;
            oss << "Queue " << qname << " not expected size! was: " << q->QueueSize()
                << " expected: " << amount;
            testerbase->Failure(this, oss.str());
        }
    }

    void TestNode::PrintNode() {
        Info("Public tag: (%llu, %llu, %u)", publicTag.Count(), publicTag.Key(), publicTag.QueueSize());
        Info("Private tag: (%llu, %llu, %u)", privateTag.Count(), privateTag.Key(), privateTag.QueueSize());
    }
}

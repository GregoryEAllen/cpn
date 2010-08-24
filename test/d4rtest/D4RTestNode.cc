
#include "D4RTestNode.h"
#include "D4RTestQueue.h"
#include "D4RTester.h"
#include "D4RDeadlockException.h"
#include "Assert.h"
#include <sstream>

namespace D4R {


    TestNode::TestNode(const std::string &name_, uint64_t k, TesterBase *tb)
        : TestNodeBase(tb), name(name_), node(new Node(k))
    {
        Logger::Name(name);
    }

    TestNode::TestNode(const Variant &noded, TesterBase *tb)
        : TestNodeBase(tb),
        name(noded["name"].AsString()),
        node(new Node(noded["key"].AsNumber<uint64_t>()))
    {
        Logger::Name(name);
        Variant::ConstListIterator itr = noded["instructions"].ListBegin();
        while (itr != noded["instructions"].ListEnd()) {
            AddOp(*itr);
            ++itr;
        }
    }

    TestNode::~TestNode() {
        Join();
    }

    void TestNode::AddReadQueue(shared_ptr<TestQueue> q) {
        PthreadMutexProtected al(lock);
        q->SetReaderNode(node);
        node->AddReader(q);
        readermap.insert(std::make_pair(q->GetName(), q));
    }

    void TestNode::AddWriteQueue(shared_ptr<TestQueue> q) {
        PthreadMutexProtected al(lock);
        q->SetWriterNode(node);
        node->AddWriter(q);
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
        shared_ptr<TestQueue> q;
        {
            PthreadMutexProtected al(lock);
            q  = writermap[qname];
        }
        ASSERT(q, "%s Invalid queue %s", name.c_str(), qname.c_str());
        for (unsigned i = 0; i < amount; ++i) {
            q->Enqueue(1);
        }
    }

    void TestNode::Dequeue(const std::string &qname, unsigned amount) {
        shared_ptr<TestQueue> q;
        {
            PthreadMutexProtected al(lock);
            q  = readermap[qname];
        }
        ASSERT(q, "%s Invalid queue %s", name.c_str(), qname.c_str());
        for (unsigned i = 0; i < amount; ++i) {
            q->Dequeue(1);
        }
    }

    void TestNode::VerifyReaderSize(const std::string &qname, unsigned amount) {
        shared_ptr<TestQueue> q;
        {
            PthreadMutexProtected al(lock);
            q  = readermap[qname];
        }
        ASSERT(q, "%s Invalid queue %s", name.c_str(), qname.c_str());
        if (amount != q->QueueSize()) {
            std::ostringstream oss;
            oss << "Queue " << qname << " not expected size! was: " << q->QueueSize()
                << " expected: " << amount;
            testerbase->Failure(this, oss.str());
        }
    }

    void TestNode::VerifyWriterSize(const std::string &qname, unsigned amount) {
        shared_ptr<TestQueue> q;
        {
            PthreadMutexProtected al(lock);
            q  = writermap[qname];
        }
        ASSERT(q, "%s Invalid queue %s", name.c_str(), qname.c_str());
        if (amount != q->QueueSize()) {
            std::ostringstream oss;
            oss << "Queue " << qname << " not expected size! was: " << q->QueueSize()
                << " expected: " << amount;
            testerbase->Failure(this, oss.str());
        }
    }

    void TestNode::PrintNode() {
        Tag publicTag = GetPublicTag();
        Tag privateTag = GetPrivateTag();
        Info("Public tag: (%llu, %llu, %u)", publicTag.Count(), publicTag.Key(), publicTag.QueueSize());
        Info("Private tag: (%llu, %llu, %u)", privateTag.Count(), privateTag.Key(), privateTag.QueueSize());
    }
}

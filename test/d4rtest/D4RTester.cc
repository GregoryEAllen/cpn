

#include "D4RTester.h"
#include "D4RTestNode.h"

namespace D4R {

    Tester::Tester() {
    }

    Tester::~Tester() {
        Abort();
        NodeMap::iterator nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            TestNode *n = nodeitr->second;
            delete n;
            ++nodeitr;
        }
        nodemap.clear();
        QueueMap::iterator qitr = queuemap.begin();
        while (qitr != queuemap.end()) {
            delete qitr->second;
            ++qitr;
        }
        queuemap.clear();
    }

    void Tester::Abort() {
        QueueMap::iterator qitr = queuemap.begin();
        while (qitr != queuemap.end()) {
            qitr->second->Abort();
            ++qitr;
        }
    }

    void Tester::Run() {
        NodeMap::iterator nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            nodeitr->second->Start();
            ++nodeitr;
        }
        nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            nodeitr->second->Join();
            ++nodeitr;
        }
    }

    void Tester::CreateNode(const Variant &noded) {
        TestNode *tn = new TestNode(noded, this);
        tn->Output(this);
        tn->LogLevel(LogLevel());
        nodemap.insert(std::make_pair(tn->GetName(), tn));
    }

    void Tester::CreateQueue(const Variant &queued) {
        TestQueue *tq = new TestQueue(queued);
        tq->Output(this);
        tq->LogLevel(LogLevel());
        queuemap.insert(std::make_pair(tq->GetName(), tq));
        nodemap[queued["reader"].AsString()]->AddReadQueue(tq);
        nodemap[queued["writer"].AsString()]->AddWriteQueue(tq);
    }

    void Tester::PrintNodes() {
        NodeMap::iterator nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            nodeitr->second->PrintNode();
            ++nodeitr;
        }
    }
}

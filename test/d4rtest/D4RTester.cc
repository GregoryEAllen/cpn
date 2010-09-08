

#include "D4RTester.h"
#include "D4RTestNode.h"
#include <vector>
#include <iostream>
#include <algorithm>

namespace D4R {

    Tester::Tester() {
    }

    Tester::~Tester() {
        Abort();
        PthreadMutexProtected al(lock);
        NodeMap::iterator nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            TestNode *n = nodeitr->second;
            delete n;
            ++nodeitr;
        }
        nodemap.clear();
        queuemap.clear();
    }

    void Tester::Abort() {
        PthreadMutexProtected al(lock);
        QueueMap::iterator qitr = queuemap.begin();
        while (qitr != queuemap.end()) {
            qitr->second.queue->Abort();
            ++qitr;
        }
    }

    void Tester::Run() {
        std::vector<TestNode*> joiners;
        {
            PthreadMutexProtected al(lock);
            NodeMap::iterator nodeitr = nodemap.begin();
            while (nodeitr != nodemap.end()) {
                nodeitr->second->Start();
                joiners.push_back(nodeitr->second);
                ++nodeitr;
            }
        }
        std::vector<TestNode*>::iterator itr = joiners.begin();
        while (itr != joiners.end()) {
            (*itr)->Join();
            ++itr;
        }
    }

    struct node_t;
    struct queue_t {
        queue_t() : down(0), up(0) {}
        TestNode *blockee; // the one who is blocked
        node_t *down;

        TestNode *blocker; // the one who is being blocked on
        node_t *up;

        std::string name;
        Tester::QueueInfo qinfo; // info about the queue in question
    };

    typedef std::vector<queue_t*> qlist_t;
    struct node_t {
        node_t() : blocker(0) {}
        queue_t *blocker; // Who we are blocked on
        qlist_t blockees; // who is blocked on us
        std::string name;
    };
    typedef std::vector<node_t*> nlist_t;
    typedef std::map<std::string, node_t*> nmap_t;
    typedef std::map<std::string, queue_t*> qmap_t;


    void Tester::Report() {
        PthreadMutexProtected al(lock);
        nmap_t nmap;
        qmap_t qmap;

        QueueMap::iterator qentry = queuemap.begin();
        for (;qentry != queuemap.end(); ++qentry) {
            queue_t *queue = new queue_t;
            queue->qinfo = qentry->second;
            shared_ptr<TestQueue> q = queue->qinfo.queue;
            queue->name = q->GetName();
            if (q->EnqueueAmount() > 0) {
                if (q->DequeueAmount() > 0) {
                    std::cout << "Queue \"" << queue->name << "\" is blocked on both sides." << std::endl;
                    delete queue;
                    continue;
                }
                queue->blockee = queue->qinfo.writer;
                queue->blocker = queue->qinfo.reader;
            } else if (q->DequeueAmount() > 0) {
                if (q->EnqueueAmount() > 0) {
                    std::cout << "Queue \"" << queue->name << "\" is blocked on both sides." << std::endl;
                    delete queue;
                    continue;
                }
                queue->blockee = queue->qinfo.reader;
                queue->blocker = queue->qinfo.writer;
            } else {
                delete queue;
                continue;
            }

            nmap_t::iterator nm_itr = nmap.find(queue->blockee->GetName());
            node_t *current = 0;
            if (nm_itr == nmap.end()) {
                current = new node_t;
                current->name = queue->blockee->GetName();
                nmap.insert(std::make_pair(current->name, current));
            } else {
                current = nm_itr->second;
            }
            current->blocker = queue;
            queue->down = current;
            current = 0;
            nm_itr = nmap.find(queue->blocker->GetName());
            if (nm_itr == nmap.end()) {
                current = new node_t;
                current->name = queue->blocker->GetName();
                nmap.insert(std::make_pair(current->name, current));
            } else {
                current = nm_itr->second;
            }
            current->blockees.push_back(queue);
            queue->up = current;
            qmap.insert(std::make_pair(queue->name, queue));
        }

        std::cout << "Printing chains" << std::endl;
        nmap_t::iterator nm_itr = nmap.begin();
        for (;nm_itr != nmap.end(); nm_itr++) {
            node_t *n = nm_itr->second;
            if (n->blockees.empty()) {
                std::cout << "Blocked chain:" << std::endl;
                node_t *cur = n;
                nlist_t nlist;
                do {
                    queue_t *q = cur->blocker;
                    node_t *next = q->up;

                    Tag publicTag = q->blockee->GetPublicTag();
                    Tag privateTag = q->blockee->GetPrivateTag();
                    std::cout << "\"" << cur->name << ", public: ("
                        << publicTag.Count() << ", "
                        << publicTag.Key() << ", "
                        << (int)publicTag.QueueSize() << ", "
                        << publicTag.QueueKey() << ") private: ("
                        << privateTag.Count() << ", "
                        << privateTag.Key() << ", "
                        << (int)privateTag.QueueSize() << ", "
                        << privateTag.QueueKey() << ")\t-> (\""
                        << q->name << ", s: "
                        << q->qinfo.queue->QueueSize() << ", c: "
                        << q->qinfo.queue->Count() << ", e: "
                        << q->qinfo.queue->EnqueueAmount() << ", d: "
                        << q->qinfo.queue->DequeueAmount() << ")\t-> \""
                        << next->name << "\"" << std::endl;

                    nlist.push_back(cur);
                    cur = next;
                } while (!(cur->blocker == 0 || std::find(nlist.begin(), nlist.end(), cur) != nlist.end()));
            }
        }
        std::cout << "Done printing chains" << std::endl;

        for (nm_itr = nmap.begin(); nm_itr != nmap.end(); ++nm_itr) {
            delete nm_itr->second;
        }
        qmap_t::iterator qm_itr;
        for (qm_itr = qmap.begin(); qm_itr != qmap.end(); ++qm_itr) {
            delete qm_itr->second;
        }
    }

    void Tester::CreateNode(const Variant &noded) {
        TestNode *tn = new TestNode(noded, this);
        tn->Output(this);
        tn->LogLevel(LogLevel());
        PthreadMutexProtected al(lock);
        nodemap.insert(std::make_pair(tn->GetName(), tn));
    }

    void Tester::CreateQueue(const Variant &queued) {
        shared_ptr<TestQueue> tq(new TestQueue(queued));
        tq->Output(this);
        tq->LogLevel(LogLevel());
        QueueInfo qinfo;
        qinfo.queue = tq;
        qinfo.reader = nodemap[queued["reader"].AsString()];
        qinfo.writer = nodemap[queued["writer"].AsString()];
        qinfo.reader->AddReadQueue(tq);
        qinfo.writer->AddWriteQueue(tq);
        PthreadMutexProtected al(lock);
        queuemap.insert(std::make_pair(tq->GetName(), qinfo));
    }

    void Tester::PrintNodes() {
        NodeMap::iterator nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            nodeitr->second->PrintNode();
            ++nodeitr;
        }
    }

    void Tester::Failure(TestNodeBase *tnb, const std::string &msg) {
        Report();
        TesterBase::Failure(tnb, msg);
    }
}



#include "D4RTester.h"
#include "D4RTestNode.h"
#include <vector>

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
            delete qitr->second.queue;
            ++qitr;
        }
        queuemap.clear();
    }

    void Tester::Abort() {
        QueueMap::iterator qitr = queuemap.begin();
        while (qitr != queuemap.end()) {
            qitr->second.queue->Abort();
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
        nmap_t nmap;
        qmap_t qmap;

        QueueMap::iterator qentry = queuemap.begin();
        for (;qentry != queuemap.end(); ++qentry) {
            queue_t *queue = new queue_t;
            queue->qinfo = qentry->second;
            TestQueue *q = queue->qinfo.queue;
            queue->name = q->GetName();
            if (q->EnqueueAmount() > 0) {
                if (q->DequeueAmount() > 0) {
                    printf("Queue \"%s\" is blocked on both sides.\n", queue->name.c_str());
                    delete queue;
                    continue;
                }
                queue->blockee = queue->qinfo.writer;
                queue->blocker = queue->qinfo.reader;
            }
            if (q->DequeueAmount() > 0) {
                if (q->EnqueueAmount() > 0) {
                    printf("Queue \"%s\" is blocked on both sides.\n", queue->name.c_str());
                    delete queue;
                    continue;
                }
                queue->blockee = queue->qinfo.reader;
                queue->blocker = queue->qinfo.writer;
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

        printf("Printing chains\n");
        nmap_t::iterator nm_itr = nmap.begin();
        for (;nm_itr != nmap.end(); nm_itr++) {
            node_t *n = nm_itr->second;
            if (n->blockees.empty()) {
                printf("Blocked chain:\n");
                node_t *cur = n;
                nlist_t nlist;
                do {
                    queue_t *q = cur->blocker;
                    node_t *next = q->up;

                    Tag publicTag = q->blockee->GetPublicTag();
                    Tag privateTag = q->blockee->GetPrivateTag();
                    printf("\"%s\", public: (%llu, %llu, %d) private: (%llu, %llu, %d)"
                           "\t-> (\"%s\", s: %u, c: %u, e: %u, d: %u)\t-> \"%s\"\n",
                            cur->name.c_str(),
                            publicTag.Count(), publicTag.Key(), (int)publicTag.QueueSize(),
                            privateTag.Count(), privateTag.Key(), (int)privateTag.QueueSize(),

                            q->name.c_str(),
                            q->qinfo.queue->QueueSize(),
                            q->qinfo.queue->Count(),
                            q->qinfo.queue->EnqueueAmount(),
                            q->qinfo.queue->DequeueAmount(),
                            next->name.c_str()
                          );
                    nlist.push_back(cur);
                    cur = next;
                } while (!(cur->blocker == 0 || std::find(nlist.begin(), nlist.end(), cur) != nlist.end()));
            }
        }
        printf("Done printing chains\n");

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
        nodemap.insert(std::make_pair(tn->GetName(), tn));
    }

    void Tester::CreateQueue(const Variant &queued) {
        TestQueue *tq = new TestQueue(queued);
        tq->Output(this);
        tq->LogLevel(LogLevel());
        QueueInfo qinfo;
        qinfo.queue = tq;
        qinfo.reader = nodemap[queued["reader"].AsString()];
        qinfo.writer = nodemap[queued["writer"].AsString()];
        queuemap.insert(std::make_pair(tq->GetName(), qinfo));
        qinfo.reader->AddReadQueue(tq);
        qinfo.writer->AddWriteQueue(tq);
    }

    void Tester::PrintNodes() {
        NodeMap::iterator nodeitr = nodemap.begin();
        while (nodeitr != nodemap.end()) {
            nodeitr->second->PrintNode();
            ++nodeitr;
        }
    }
}

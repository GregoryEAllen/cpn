

#include "D4RNode.h"
#include "D4RQueue.h"
#include "Assert.h"

class Node : public D4R::Node {
public:
    Node(unsigned long long k)
        : D4R::Node(k),
        locked(false)
    {}

    void SignalTagChanged() {}

    void Lock() const {
        ASSERT(!locked, "Deadlock");
        locked = true;
    }

    void Unlock() const {
        ASSERT(locked, "Double unlock!");
        locked = false;
    }

private:
    mutable bool locked;
};

class Queue : public D4R::QueueBase {
public:
    Queue(unsigned initialsize)
        : detected(false), queuesize(initialsize), locked(false)
    {}

    void Lock() const {
        ASSERT(!locked, "Deadlock");
        locked = true;
    }

    void Unlock() const {
        ASSERT(locked, "Double unlock!");
        locked = false;
    }

    bool Detected() const { return detected; }
private:

    bool ReadBlocked() { return true; }
    bool WriteBlocked() { return true; }
    void Detect(bool artificial) {
        detected = true;
    }
    void Signal() {}
    void Wait() {}

    bool detected;
    unsigned queuesize;
    mutable bool locked;
};

const char VALID_OPS[] = "p";

int main(int argc, char **argv) {
    bool procOpts = true;
    while (procOpts) {
        int opt = getopt(argc, argv, VALID_OPS);
        switch (opt) {
        case -1:
            procOpts = false;
            break;
        default:
            return 0;
        }
    }

    Queue queueAB(1);
    Node A(1);
    Node B(2);
    queueAB.Lock();
    // A blocks on B
    // B blocks on A
    if (queueAB.Detected()) {
        printf("Success!\n");
    } else {
        printf("didn't detect deadlock");
    }
}


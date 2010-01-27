

#include "D4RNode.h"
#include "Assert.h"

class Node : public D4R::Node {
public:
    Node(unsigned long long k)
        : D4R::Node(k),
        locked(false)
    {}

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

class Queue : public D4R::Queue {
public:
    Queue(unsigned initialsize)
        : detected(false), queuesize(initialsize), locked(false)
    {}

    void ReadBlock() { D4R::Queue::ReadBlock(); }
    void WriteBlock(unsigned count) { D4R::Queue::WriteBlock(count); }
    void ReadUnblock() { D4R::Queue::ReadUnblock(); }
    void WriteUnblock() { D4R::Queue::WriteUnblock(); }

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

    void Detect() {
        detected = true;
    }

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
    queueAB.SetWriterNode(&A);
    queueAB.SetReaderNode(&B);
    queueAB.Lock();
    // A blocks on B
    queueAB.WriteBlock(2);
    // B blocks on A
    queueAB.ReadBlock();
    if (queueAB.Detected()) {
        queueAB.WriteUnblock();
        queueAB.ReadUnblock();
        printf("Success!\n");
    } else {
        printf("didn't detect deadlock");
    }
}


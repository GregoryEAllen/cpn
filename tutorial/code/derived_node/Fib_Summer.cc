#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include <stdint.h>

using namespace CPN;

class Summer : public NodeBase {
public:
    Summer(Kernel &ker, const NodeAttr &attr)
    : NodeBase(ker, attr) {}
private:
    void Process();
};


void Summer::Process() {
    IQueue<uint64_t> in_a = GetIQueue("A");
    IQueue<uint64_t> in_b = GetIQueue("B");
    OQueue<uint64_t> out = GetOQueue("C");
    while (true) {
        uint64_t val_a, val_b, sum;
        if (!in_a.Dequeue(&val_a, 1)) break;
        if (!in_b.Dequeue(&val_b, 1)) break;
        sum = val_a + val_b;
        out.Enqueue(&sum, 1);
    }
}

CPN_DECLARE_NODE_FACTORY(Summer, Summer);

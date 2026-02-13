#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include <stdint.h>

using namespace CPN;

CPN_DECLARE_NODE_AND_FACTORY(Cons, Cons);

void Cons::Process() {
    IQueue<uint64_t> in = GetIQueue("in");
    OQueue<uint64_t> out_a = GetOQueue("out0");
    OQueue<uint64_t> out_b = GetOQueue("out1");
    uint64_t current = GetParam<uint64_t>("initial");
    while (true) {
        out_a.Enqueue(&current, 1);
        out_b.Enqueue(&current, 1);
        if (!in.Dequeue(&current, 1)) break;
    }
}



#include "Kernel.h"
#include "NodeBase.h"
#include "IQueue.h"
#include "OQueue.h"
#include <stdlib.h>
#include <iostream>

using namespace CPN;
using std::string;

static void Summer(NodeBase *node) {
    IQueue<uint64_t> in_a = node->GetIQueue("A");
    IQueue<uint64_t> in_b = node->GetIQueue("B");
    OQueue<uint64_t> out = node->GetOQueue("C");
    while (true) {
        uint64_t val_a, val_b, sum;
        if (!in_a.Dequeue(&val_a, 1)) break;
        if (!in_b.Dequeue(&val_b, 1)) break;
        sum = val_a + val_b;
        out.Enqueue(&sum, 1);
    }
}

static void Cons(NodeBase *node, uint64_t initial) {
    IQueue<uint64_t> in = node->GetIQueue("in");
    OQueue<uint64_t> out_a = node->GetOQueue("out0");
    OQueue<uint64_t> out_b = node->GetOQueue("out1");
    uint64_t current = initial;
    while (true) {
        out_a.Enqueue(&current, 1);
        out_b.Enqueue(&current, 1);
        if (!in.Dequeue(&current, 1)) break;
    }
}


int main(int argc, char **argv) {
    uint64_t max_fib = 100;
    while(true) {
        int c = getopt(argc, argv, "m:");
        if (c == -1) break;
        switch (c) {
        case 'm':
            max_fib = strtoull(optarg, 0, 10);
            break;
        default:
            break;
        }
    }

    Kernel kernel(KernelAttr("kernel")
        .UseD4R(false)
        .SwallowBrokenQueueExceptions(true));

    kernel.CreateFunctionNode("summer", Summer);
    kernel.CreateFunctionNode("Cons 1", Cons, 1);
    kernel.CreateFunctionNode("Cons 2", Cons, 1);
    kernel.CreateExternalReader("result");

    QueueAttr qattr(2*sizeof(uint64_t), sizeof(uint64_t));
    qattr.SetDatatype<uint64_t>();
    qattr.SetEndpoints("summer", "A", "Cons 1", "out0");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Cons 2", "out0").SetReader("summer", "B");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("summer", "C").SetReader("Cons 1", "in");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Cons 1", "out1").SetReader("Cons 2", "in");
    kernel.CreateQueue(qattr);

    qattr.SetWriter("Cons 2", "out1").SetExternalReader("result");
    kernel.CreateQueue(qattr);

    // This is our result reader.
    {
        IQueue<uint64_t> result = kernel.GetExternalIQueue("result");
        uint64_t value;
        do {
            result.Dequeue(&value, 1);
            std::cout << "- " << value << std::endl;
        } while (value < max_fib);
        result.Release();
    }

    kernel.DestroyExternalEndpoint("result");
    kernel.WaitForAllNodeEnd();
    return 0;
}


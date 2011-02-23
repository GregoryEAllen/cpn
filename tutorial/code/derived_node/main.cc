#include "Kernel.h"
#include "IQueue.h"
#include <iostream>
#include <stdlib.h>
#include <stdint.h>

using namespace CPN;

int main(int argc, char **argv) {
    uint64_t max_fib = 100;
    while (true) {
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

    // Create the three nodes use the same parameters for both the cons nodes
    NodeAttr nattr("summer", "Summer");
    kernel.CreateNode(nattr);
    nattr = NodeAttr("Cons 1", "Cons");
    nattr.SetParam("initial", 1);
    nattr.SetParam("num outputs", 2);
    kernel.CreateNode(nattr);
    nattr.SetName("Cons 2");
    kernel.CreateNode(nattr);
    Key_t pkey = kernel.CreatePseudoNode("result");

    QueueAttr qattr(2*sizeof(uint64_t), sizeof(uint64_t));
    qattr.SetDatatype<uint64_t>();
    qattr.SetWriter("Cons 1", "out0").SetReader("summer", "A");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Cons 2", "out0").SetReader("summer", "B");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("summer", "C").SetReader("Cons 1", "in");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Cons 1", "out1").SetReader("Cons 2", "in");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Cons 2", "out1").SetReader("result", "in");
    kernel.CreateQueue(qattr);

    IQueue<uint64_t> result = kernel.GetPseudoIQueue(pkey, "in");
    uint64_t value;
    do {
        result.Dequeue(&value, 1);
        std::cout << "- " << value << std::endl;
    } while (value < max_fib);
    result.Release();
    kernel.DestroyPseudoNode(pkey);
    kernel.WaitForAllNodeEnd();

    return 0;
}


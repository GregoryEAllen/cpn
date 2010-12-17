#include "Kernel.h"
#include "QueueReaderAdapter.h"
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

    Kernel kernel(KernelAttr("kernel"));
    kernel.GetDatabase()->UseD4R(false);
    kernel.GetDatabase()->SwallowBrokenQueueExceptions(true);

    // Create the three nodes use the same parameters for both the delay nodes
    NodeAttr nattr("summer", "Summer", "{\"inputs\":[ \"A\","
           " \"B\"], \"output\": \"C\"}");
    kernel.CreateNode(nattr);
    nattr = NodeAttr("Delay 1", "Delay", "{\"input\": \"in\","
           " \"outputs\":[ \"A\", \"B\" ], \"initial\": 1}");
    kernel.CreateNode(nattr);
    nattr.SetName("Delay 2");
    kernel.CreateNode(nattr);
    Key_t pkey = kernel.CreatePseudoNode("result");

    QueueAttr qattr(2*sizeof(uint64_t), sizeof(uint64_t));
    qattr.SetDatatype<uint64_t>();
    qattr.SetWriter("Delay 1", "A").SetReader("summer", "A");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Delay 2", "A").SetReader("summer", "B");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("summer", "C").SetReader("Delay 1", "in");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Delay 1", "B").SetReader("Delay 2", "in");
    kernel.CreateQueue(qattr);
    qattr.SetWriter("Delay 2", "B").SetReader("result", "in");
    kernel.CreateQueue(qattr);

    QueueReaderAdapter<uint64_t> result = kernel.GetPseudoReader(pkey, "in");
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


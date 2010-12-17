
#include "Kernel.h"
#include "NodeBase.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include <stdlib.h>
#include <iostream>

using namespace CPN;
using std::string;

static void Summer(NodeBase *node, string input_a, string input_b,
        string output) {
    QueueReaderAdapter<uint64_t> in_a = node->GetReader(input_a);
    QueueReaderAdapter<uint64_t> in_b = node->GetReader(input_b);
    QueueWriterAdapter<uint64_t> out = node->GetWriter(output);
    while (true) {
        uint64_t val_a, val_b, sum;
        if (!in_a.Dequeue(&val_a, 1)) break;
        if (!in_b.Dequeue(&val_b, 1)) break;
        sum = val_a + val_b;
        out.Enqueue(&sum, 1);
    }
}

static void Delay(NodeBase *node, string input, string output_a,
        string output_b, uint64_t initial) {
    QueueReaderAdapter<uint64_t> in = node->GetReader(input);
    QueueWriterAdapter<uint64_t> out_a = node->GetWriter(output_a);
    QueueWriterAdapter<uint64_t> out_b = node->GetWriter(output_b);
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

    Kernel kernel(KernelAttr("kernel"));
    kernel.GetDatabase()->UseD4R(false);
    kernel.GetDatabase()->SwallowBrokenQueueExceptions(true);

    kernel.CreateFunctionNode("summer", Summer, string("A"), string("B"),
            string("C"));
    kernel.CreateFunctionNode("Delay 1", Delay, string("in"), string("A"),
            string("B"), 1ull);
    kernel.CreateFunctionNode("Delay 2", Delay, string("in"), string("A"),
            string("B"), 1ull);
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


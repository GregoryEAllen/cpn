

#include "Kernel.h"
#include "SieveControllerNode.h"
#include "ErrnoException.h"
#include "VariantToJSON.h"
#include "QueueReaderAdapter.h"
#include <sys/time.h>
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <cstdlib>

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

static const char* const VALID_OPS = "hm:q:";

int main(int argc, char **argv) {
    int maxprime = 100;
    int queueSize = 100;
    int threshold = 1;
    bool procOpts = true;
    while (procOpts) {
        int opt = getopt(argc, argv, VALID_OPS);
        switch (opt) {
        case 'm':
            maxprime = atoi(optarg);
            if (maxprime < 2) maxprime = 2;
            break;
        case 'q':
            queueSize = atoi(optarg);
            if (queueSize < 1) queueSize = 1;
            break;
        case -1:
            procOpts = false;
            break;
        case 'h':
        default:
            printf("Usage: %s -m maxprime -q queuesize\n", argv[0]);
            return 0;
        }
    }
    CPN::Kernel kernel(CPN::KernelAttr("SimpleSieveKernel"));
    std::vector<unsigned long> results;
    Variant param;
    param["primeBound"] = maxprime;
    param["numberBound"] = maxprime;
    param["queueSize"] = queueSize;
    if (threshold == 1) {
        param["queuehint"] = CPN::QUEUEHINT_DEFAULT;
    } else {
        param["queuehint"] = CPN::QUEUEHINT_THRESHOLD;
    }
    param["threshold"] = threshold;
    CPN::NodeAttr attr("controller", SIEVECONTROLLERNODE_TYPENAME);
    attr.SetParam(VariantToJSON(param));
    kernel.CreateNode(attr);
    CPN::Key_t pseudokey = kernel.CreatePseudoNode("output");
    CPN::QueueAttr qattr(100*sizeof(unsigned long), 100*sizeof(unsigned long));
    qattr.SetHint(threshold == 1 ? CPN::QUEUEHINT_DEFAULT : CPN::QUEUEHINT_THRESHOLD);
    qattr.SetWriter("controller", "output");
    qattr.SetReader("output", "out");
    kernel.CreateQueue(qattr);
    double start = getTime();
    CPN::QueueReaderAdapter<unsigned long> in = kernel.GetPseudoReader(pseudokey, "out");
    unsigned long val;
    while (in.Dequeue(&val, 1)) {
        results.push_back(val);
    }
    kernel.DestroyPseudoNode(pseudokey);
    kernel.WaitNodeTerminate("controller");
    double stop = getTime();
    printf("Duration: %f\n", (stop - start));
    for (unsigned i = 0; i < results.size(); i++) {
        printf(" %lu,", results[i]);
    }
    printf("\n");
    return 0;
}



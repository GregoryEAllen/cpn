

#include "Kernel.h"
#include "SieveControllerNode.h"
#include "ErrnoException.h"
#include "IQueue.h"
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
    CPN::NodeAttr attr("controller", SIEVECONTROLLERNODE_TYPENAME);
    attr.SetParam("primeBound", maxprime);
    attr.SetParam("numberBound", maxprime);
    attr.SetParam("queueSize", queueSize);
    kernel.CreateNode(attr);
    kernel.CreateExternalReader("output");
    CPN::QueueAttr qattr(100*sizeof(unsigned long), 100*sizeof(unsigned long));
    qattr.SetWriter("controller", "output");
    qattr.SetExternalReader("output");
    kernel.CreateQueue(qattr);
    double start = getTime();
    CPN::IQueue<unsigned long> in = kernel.GetExternalIQueue("output");
    unsigned long val;
    while (in.Dequeue(&val, 1)) {
        results.push_back(val);
    }
    in.Release();
    kernel.DestroyExternalEndpoint("output");
    kernel.WaitForNode("controller");
    double stop = getTime();
    printf("Duration: %f\n", (stop - start));
    for (unsigned i = 0; i < results.size(); i++) {
        printf(" %lu,", results[i]);
    }
    printf("\n");
    return 0;
}



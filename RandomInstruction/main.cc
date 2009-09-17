

#include "RandomInstructionNode.h"
#include "Kernel.h"
#include "Database.h"
#include <unistd.h>

// one interesting seed is 37733

int main(int argc, char **argv) {
    RandomInstructionNode::RegisterNodeType();

    unsigned iterations = 10;
    unsigned numNodes = 10;
    int debugLevel = 0;
    LFSR::LFSR_t seed = RandomInstructionGenerator::DEFAULT_SEED;

    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, "i:d:n:s:")) {
        case 'i':
            iterations = atoi(optarg);
            break;
        case 'd':
            debugLevel = atoi(optarg);
            break;
        case 'n':
            numNodes = atoi(optarg);
            break;
        case 's':
            {
                char *end = 0;
                seed = strtoul(optarg, &end, 0);
                if (seed == 0) {
                    printf("Seed cannot be 0\n");
                    return 1;
                }
            }
            break;
        case -1:
            procOpts = false;
            break;
        default:
            printf("Invald option %s\n", optarg);
        }
    }

    printf("Starting with %u nodes going for %u iterations with debug level %d and seed %lu\n",
            numNodes, iterations, debugLevel, seed);
    CPN::Kernel kernel(CPN::KernelAttr("test kernel"));
    RandomInstructionNode::CreateRIN(kernel, iterations, numNodes, debugLevel, seed);
    kernel.GetDatabase()->WaitForAllNodeEnd();
    return 0;
}



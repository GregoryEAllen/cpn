

#include "RandomInstructionNode.h"
#include "Kernel.h"
#include "Database.h"
#include <unistd.h>

// one interesting seed is 37733

const char* const VALID_OPTS = "hi:d:n:s:l:";

const char* const HELP_OPTS = "Usage: %s \n"
"\t-h\tPrint out this message\n"
"\t-i n\tRun for n iterations (default 10)\n"
"\t-n n\tStart with n nodes (default 10)\n"
"\t-s n\tStart with n as the seed\n"
"\t-d n\tUse debug level n (default 0)\n"
"\t-l n\tUse log level n (default 75)\n"
;

int main(int argc, char **argv) {
    RandomInstructionNode::RegisterNodeType();

    unsigned iterations = 10;
    unsigned numNodes = 10;
    int debugLevel = 0;
    int loglevel = 75;
    LFSR::LFSR_t seed = RandomInstructionGenerator::DEFAULT_SEED;

    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'i':
            iterations = atoi(optarg);
            break;
        case 'd':
            debugLevel = atoi(optarg);
            break;
        case 'n':
            numNodes = atoi(optarg);
            break;
        case 'l':
            loglevel = atoi(optarg);
            break;
        case 'h':
            printf(HELP_OPTS);
            return 0;
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
            printf(HELP_OPTS);
            return 0;
        }
    }

    printf("Starting with %u nodes going for %u iterations with debug level %d and seed %lu\n",
            numNodes, iterations, debugLevel, seed);

    shared_ptr<CPN::Database> database = CPN::Database::LocalDatabase();
    database->LogLevel(loglevel);

    CPN::Kernel kernel(CPN::KernelAttr("test kernel").SetDatabase(database));
    RandomInstructionNode::CreateRIN(kernel, iterations, numNodes, debugLevel, seed);
    kernel.GetDatabase()->WaitForAllNodeEnd();
    return 0;
}





#include "RandomInstructionNode.h"

#include "Kernel.h"
#include "Context.h"

#include "ToString.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

using CPN::shared_ptr;
using CPN::Context;
using CPN::Kernel;
using CPN::KernelAttr;

// one interesting seed is 37733

const char* const VALID_OPTS = "hi:d:n:s:l:k:C:D:";

const char* const HELP_OPTS = "Usage: %s \n"
"\t-h\tPrint out this message\n"
"\t-i n\tRun for n iterations (default 10)\n"
"\t-n n\tStart with n nodes (default 10)\n"
"\t-s n\tStart with n as the seed\n"
"\t-d n\tUse debug level n (default 0)\n"
"\t-l n\tUse log level n (default 75)\n"
"\t-k n\tCreate n kernels (default 1)\n"
"\t-C n\tSet probability to create (default 0.01)\n"
"\t-D n\tSet probability to destroy (default 0.01)\n"
;

int main(int argc, char **argv) {

    unsigned iterations = 10;
    unsigned numNodes = 10;
    int debugLevel = 0;
    int loglevel = 75;
    unsigned numKernels = 1;
    LFSR::LFSR_t seed = RandomInstructionGenerator::DEFAULT_SEED;
    double createProb = 0.01;
    double destroyProb = 0.01;

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
        case 'k':
            numKernels = atoi(optarg);
            break;
        case 'C':
            createProb = strtod(optarg, 0);
            break;
        case 'D':
            destroyProb = strtod(optarg, 0);
            break;
        case 'h':
            puts(HELP_OPTS);
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
            puts(HELP_OPTS);
            return 0;
        }
    }

    printf("Starting with %u nodes going for %u iterations with debug level %d and seed %lu\n",
            numNodes, iterations, debugLevel, seed);

    shared_ptr<Context> context = Context::Local();
    context->LogLevel(loglevel);

    std::vector< shared_ptr<Kernel> > kernels;
    std::vector<std::string> kernelnames;

    for (unsigned i = 0; i < numKernels; ++i) {
        std::string name = ToString("K #%u", i);
        kernelnames.push_back(name);
        kernels.push_back(shared_ptr<Kernel>(new Kernel(KernelAttr(name).SetContext(context).SetRemoteEnabled(numKernels > 1))));
    }
    RandomInstructionNode::CreateRIN(*kernels.front(), iterations, numNodes, debugLevel, seed, kernelnames, createProb, destroyProb);

    context->WaitForAllNodeEnd();

    return 0;
}



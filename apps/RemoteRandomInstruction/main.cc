

#include "RandomInstructionNode.h"
#include "RemoteContext.h"

#include "Variant.h"
#include "JSONToVariant.h"

#include "Kernel.h"
#include "Context.h"

#include "ToString.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <signal.h>
#include <sstream>

using CPN::shared_ptr;
using CPN::Context;
using CPN::Kernel;
using CPN::KernelAttr;


// one interesting seed is 37733

const char* const VALID_OPTS = "hb:i:n:s:d:l:S:c:C:D:";

const char* const HELP_OPTS = "Usage: %s <name> <context hostname> <context port>\n"
"All options can be done as a mixture of command line and configuration files which override each other.\n"
"\t-h\tPrint out this message\n"
"\t-b\tSpecify an IP for the kernel to bind to.\n"
"\t-i n\tRun for n iterations (default 10)\n"
"\t-n n\tStart with n nodes (default 10)\n"
"\t-s n\tStart with n as the seed\n"
"\t-d n\tUse debug level n (default 0)\n"
"\t-l n\tUse log level n (default 75)\n"
"\t-k 'space sperated list of kernel names'\n"
"\t-S n\tSay kernel n should be the starter\n"
"\t-c cfg\tA config file.\n"
"\t-C n\tSet probability to create (default 0.01)\n"
"\t-D n\tSet probability to destroy (default 0.01)\n"
"\n"
"The config file can have any of the following keys:\n"
"\n" 
"{\n"
"\t\"iterations\"\t: number of iterations,\n"
"\t\"numnodes\"\t: number of starting nodes,\n"
"\t\"seed\"\t\t: starting seed,\n"
"\t\"debugLevel\"\t: number for debug level,\n"
"\t\"loglevel\"\t: number for log level,\n"
"\t\"createProb\"\t: probability to create a node,\n"
"\t\"destroyProb\"\t: probability to destroy a node,\n"
"\t\"kernels\"\t: [\n"
"\t\t\"kernelone\", \"kerneltwo\", ...\n"
"\t],\n"
"\t\"starter\"\t: \"kernelname to start all nodes\"\n"
"}\n"
"Options after -c will override the config options and options before -c will be overridden by the config.\n"
"-c can be specified multiple times\n"
;

int main(int argc, char **argv) {

    signal(SIGPIPE, SIG_IGN);
    unsigned iterations = 10;
    unsigned numNodes = 10;
    int debugLevel = 0;
    int loglevel = 75;
    LFSR::LFSR_t seed = RandomInstructionGenerator::DEFAULT_SEED;
    std::string bindip = "";
    std::string starter = "";
    std::vector<std::string> kernelnames;
    double createProb = 0.01;
    double destroyProb = 0.01;

    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'h':
            printf(HELP_OPTS, argv[0]);
            return 0;
        case 'b':
            bindip = optarg;
            break;
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
        case 'S':
            starter = optarg;
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
        case 'k':
            {
                std::istringstream iss(optarg);
                iss >> std::skipws;
                kernelnames.clear();
                std::string k;
                while (iss >> k) {
                    kernelnames.push_back(k);
                }
            }
        case 'C':
            createProb = strtod(optarg, 0);
            break;
        case 'D':
            destroyProb = strtod(optarg, 0);
            break;
        case 'c':
            {
                JSONToVariant parser;
                parser.ParseFile(optarg);
                if (!parser.Done()) {
                    printf("Error parsing %s on line %d column %d\n", optarg, parser.GetLine(), parser.GetColumn());
                    return 1;
                }
                Variant conf = parser.Get();
#define check(name, var, type, typeconvert) \
                if (conf.Contains(name) == Variant::type) {\
                    var = conf[name].typeconvert();\
                } else (void)0
                check("iterations", iterations, NumberType, AsUnsigned);
                check("numnodes", numNodes, NumberType, AsUnsigned);
                check("debugLevel", debugLevel, NumberType, AsInt);
                check("loglevel", loglevel, NumberType, AsInt);
                check("starter", starter, StringType, AsString);
                check("seed", seed, NumberType, AsNumber<LFSR::LFSR_t>);
                check("createProb", createProb, NumberType, AsDouble);
                check("destroyProb", destroyProb, NumberType, AsDouble);
#undef check
                if (conf.Contains("kernels") == Variant::ArrayType) {
                    Variant k = conf["kernels"];
                    kernelnames.clear();
                    for (Variant::ListIterator i = k.ListBegin(); i != k.ListEnd(); ++i) {
                        kernelnames.push_back(i->AsString());
                    }
                }

            }
            break;
        case -1:
            procOpts = false;
            break;
        default:
            printf(HELP_OPTS, argv[0]);
            return 0;
        }
    }
    if (argc < optind + 3) {
        printf("Not enough arguments\n");
        printf(HELP_OPTS, argv[0]);
        return 1;
    }


    std::string name = argv[optind];
    std::string hostname = argv[optind+1];
    std::string servname = argv[optind+2];

    SockAddrList addrs = SocketAddress::CreateIP(hostname, servname);


    printf("%s started\n", name.c_str());
    if (starter == name) {
        printf("debug level %d seed %lu and iterations %u numnodes %u\n", debugLevel, seed, iterations, numNodes);
    } else {
        printf("in slave mode\n");
    }

    shared_ptr<RemoteContext> context = shared_ptr<RemoteContext>(new RemoteContext(addrs));
    context->LogLevel(loglevel);

    KernelAttr kattr(name);
    kattr.SetContext(context);
    if (!bindip.empty()) {
        kattr.SetHostName(bindip);
    }
    Kernel kernel(kattr);

    if (starter == name) {
        printf("Creating %u nodes\n", numNodes);
        RandomInstructionNode::CreateRIN(kernel, iterations, numNodes, debugLevel, seed, kernelnames, createProb, destroyProb);
    }

    // Wait for one of the nodes to start before we wait for all nodes to be gone
    context->WaitForNodeStart(RandomInstructionNode::GetNodeNameFromID(0));
    context->WaitForAllNodeEnd();
    printf("Done\n");

    return 0;
}



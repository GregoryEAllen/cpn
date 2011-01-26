

#include "RandomInstructionNode.h"
#include "RemoteContext.h"

#include "Variant.h"
#include "JSONToVariant.h"

#include "Kernel.h"
#include "Context.h"

#include "ToString.h"

#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <signal.h>

using CPN::shared_ptr;
using CPN::Context;
using CPN::Kernel;
using CPN::KernelAttr;


// one interesting seed is 37733

const char* const VALID_OPTS = "hb:";

const char* const HELP_OPTS = "Usage: %s <config> <name> <context hostname> <context port>\n"
"\t-h\tPrint out this message\n"
"\t-b\tSpecify an IP for the kernel to bind to.\n"
"\n"
"The config file should be of the format:\n"
"\n" 
"{\n"
"\t\"iterations\"\t: number of iterations,\n"
"\t\"numnodes\"\t: number of starting nodes,\n"
"\t\"seed\"\t\t: (optional) starting seed,\n"
"\t\"debugLevel\"\t: (optional) number for debug level,\n"
"\t\"loglevel\"\t: (optional) number for log level,\n"
"\t\"kernels\"\t: [\n"
"\t\t\"kernelone\", \"kerneltwo\", ...\n"
"\t]\n"
"\t\"starter\"\t: \"kernelname to start all nodes\"\n"
"}\n"
;

int main(int argc, char **argv) {

    signal(SIGPIPE, SIG_IGN);
    unsigned iterations = 10;
    unsigned numNodes = 10;
    int debugLevel = 0;
    int loglevel = 75;
    LFSR::LFSR_t seed = RandomInstructionGenerator::DEFAULT_SEED;
    std::string bindip = "";

    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'h':
            printf(HELP_OPTS, argv[0]);
            return 0;
        case 'b':
            bindip = optarg;
            break;
        case -1:
            procOpts = false;
            break;
        default:
            printf(HELP_OPTS, argv[0]);
            return 0;
        }
    }
    if (argc < optind + 4) {
        printf("Not enough arguments\n");
        printf(HELP_OPTS, argv[0]);
        return 1;
    }

    FILE *config = fopen(argv[optind], "r");
    if (0 == config) {
        perror("failed to open config");
        return 1;
    }
    JSONToVariant parse;
    std::vector<char> buffer(4000, 0);
    while (!feof(config) && parse.Ok()) {
        unsigned numread = fread(&buffer[0], 1, buffer.size(), config);
        parse.Parse(&buffer[0], numread);
    }
    fclose(config);
    config = 0;

    std::string name = argv[optind+1];
    std::string hostname = argv[optind+2];
    std::string servname = argv[optind+3];

    if (!parse.Done()) {
        printf("Unable to parse config file.\n");
        printf("Stopped on line %u column %u\n", parse.GetLine(), parse.GetColumn());
        return 1;
    }
    Variant conf = parse.Get();

    Variant val = conf["iterations"];
    if (val.IsNumber()) {
        iterations = val.AsUnsigned();
    }
    val = conf["numnodes"];
    if (val.IsNumber()) {
        numNodes = val.AsUnsigned();
    }
    val = conf["seed"];
    if (val.IsNumber()) {
        seed = val.AsNumber<LFSR::LFSR_t>();
    }
    val = conf["debuglevel"];
    if (val.IsNumber()) {
        debugLevel = val.AsInt();
    }
    val = conf["loglevel"];
    if (val.IsNumber()) {
        loglevel = val.AsInt();
    }

    std::string starter = conf["starter"].AsString();

    SockAddrList addrs = SocketAddress::CreateIP(hostname, servname);

    val = conf["kernels"];
    std::vector<std::string> kernelnames;
    for (Variant::List::const_iterator i = val.AsArray().begin();
            i != val.AsArray().end();
            ++i)
    {
        kernelnames.push_back(i->AsString());
    }


    printf("%s started\n", name.c_str());
    printf("debug level %d seed %lu and iterations %u numnodes %u\n", debugLevel, seed, iterations, numNodes);

    shared_ptr<RemoteContext> context = shared_ptr<RemoteContext>(new RemoteContext(addrs));
    context->LogLevel(loglevel);

    Kernel kernel(KernelAttr(name).SetHostName(bindip).SetContext(context));

    if (name == starter) {
        printf("Creating %u nodes\n", numNodes);
        RandomInstructionNode::CreateRIN(kernel, iterations, numNodes, debugLevel, seed, kernelnames);
    }

    // Wait for one of the nodes to start before we wait for all nodes to be gone
    context->WaitForNodeStart(RandomInstructionNode::GetNodeNameFromID(0));
    context->WaitForAllNodeEnd();

    return 0;
}



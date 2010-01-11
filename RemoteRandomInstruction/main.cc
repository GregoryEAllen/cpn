

#include "RandomInstructionNode.h"
#include "RemoteDatabase.h"

#include "Variant.h"

#include "Kernel.h"
#include "Database.h"

#include "ToString.h"

#include <unistd.h>
#include <vector>
#include <signal.h>

using CPN::shared_ptr;
using CPN::Database;
using CPN::Kernel;
using CPN::KernelAttr;


// one interesting seed is 37733

const char* const VALID_OPTS = "h";

const char* const HELP_OPTS = "Usage: %s <config> <name>\n"
"\t-h\tPrint out this message\n"
"\n"
"The config file should be of the format:\n"
"\n" 
"{\n"
"\t\"iterations\"\t: number of iterations,\n"
"\t\"numnodes\"\t: number of starting nodes,\n"
"\t\"seed\"\t\t: (optional) starting seed,\n"
"\t\"debugLevel\"\t: (optional) number for debug level,\n"
"\t\"loglevel\"\t: (optional) number for log level,\n"
"\t\"numkernels\"\t: total number of kernels in this network,\n"
"\t\"kernels\"\t: {\n"
"\t\t\"kernelhostnameone\"\t: [start kernel num, end kernel num],\n"
"\t\t...\n"
"\t}\n"
"\t\"starter\"\t: \"kernelhostnamethatstarts all nodes\",\n"
"\t\"server\"\t: {\"hostname\": hostname, \"port\": portname}\n"
"}\n"
;

int main(int argc, char **argv) {
    RandomInstructionNode::RegisterNodeType();

    signal(SIGPIPE, SIG_IGN);
    unsigned iterations = 10;
    unsigned numNodes = 10;
    int debugLevel = 0;
    int loglevel = 75;
    unsigned numKernels = 0;
    LFSR::LFSR_t seed = RandomInstructionGenerator::DEFAULT_SEED;

    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'h':
            puts(HELP_OPTS);
            return 0;
        case -1:
            procOpts = false;
            break;
        default:
            puts(HELP_OPTS);
            return 0;
        }
    }
    if (argc < optind + 2) {
        printf("Not enough arguments\n");
        return 1;
    }

    FILE *config = fopen(argv[optind], "r");
    if (0 == config) {
        perror("failed to open config");
        return 1;
    }
    std::vector<char> buffer(4000, 0);
    unsigned position = 0;
    while (!feof(config)) {
        position += fread(&buffer[position], 1, buffer.size() - position, config);
        if (position == buffer.size()) {
            buffer.resize(2*position, 0);
        }
    }
    buffer.resize(position);
    fclose(config);
    config = 0;

    std::string name = argv[optind+1];

    Variant conf = Variant::FromJSON(buffer);

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
    val = conf["numkernels"];
    if (!val.IsNumber()) {
        printf("numkernels must be a number\n");
        return 1;
    }
    numKernels = val.AsUnsigned();

    std::string starter = conf["starter"].AsString();

    val = conf["server"];
    if (!val.IsObject()) {
        printf("Invalid server specification");
        return 1;
    }
    SockAddrList addrs = SocketAddress::CreateIP(val["hostname"].AsString(),
            val["port"].AsString());

    val = conf["kernels"];
    val = val[name];
    if (!val.IsArray() || !val[0].IsNumber() || !val[1].IsNumber()) {
        printf("Invalid config or name\n");
        return 1;
    }
    unsigned kernelstart = val[0].AsUnsigned();
    unsigned kernelend = val[1].AsUnsigned();

    printf("%s started running kernels %u to %u\n", name.c_str(), kernelstart, kernelend);
    printf("debug level %d seed %lu and iterations %u numnodes %u\n", debugLevel, seed, iterations, numNodes);

    shared_ptr<RemoteDatabase> database = shared_ptr<RemoteDatabase>(new RemoteDatabase(addrs));
    database->LogLevel(loglevel);
    database->Start();

    std::vector< shared_ptr<Kernel> > kernels;

    for (unsigned i = kernelstart; i <= kernelend; ++i) {
        kernels.push_back(shared_ptr<Kernel>(new Kernel(KernelAttr(ToString("K #%u", i)).SetDatabase(database))));
    }

    if (name == starter) {
        printf("Creating %u nodes\n", numNodes);
        RandomInstructionNode::CreateRIN(*kernels.front(), iterations, numNodes, debugLevel, seed, numKernels);
    }

    // Wait for one of the nodes to start before we wait for all nodes to be gone
    database->WaitForNodeStart(RandomInstructionNode::GetNodeNameFromID(0));
    database->WaitForAllNodeEnd();

    return 0;
}



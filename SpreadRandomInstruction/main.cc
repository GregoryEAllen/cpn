

#include "RandomInstructionNode.h"
#include "SpreadDBClient.h"

#include "Variant.h"

#include "Kernel.h"
#include "Database.h"

#include "ToString.h"

#include <unistd.h>
#include <vector>

using CPN::shared_ptr;
using CPN::Database;
using CPN::Kernel;
using CPN::KernelAttr;

// one interesting seed is 37733

const char* const VALID_OPTS = "h";

const char* const HELP_OPTS = "Usage: %s <config> <name>\n"
"\t-h\tPrint out this message\n"
;

int main(int argc, char **argv) {
    RandomInstructionNode::RegisterNodeType();

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

    std::string server = conf["server"].AsString();
    std::string group = conf["group"].AsString();
    val = conf["spread_name"];
    std::string spread_name;
    if (val.IsString()) {
        spread_name = val.AsString();
    }

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

    shared_ptr<Database> database = shared_ptr<Database>(new SpreadDBClient(spread_name, name, server, group));
    database->LogLevel(loglevel);
    std::tr1::dynamic_pointer_cast<SpreadDBClient>(database)->Start();

    std::vector< shared_ptr<Kernel> > kernels;

    for (unsigned i = kernelstart; i <= kernelend; ++i) {
        kernels.push_back(shared_ptr<Kernel>(new Kernel(KernelAttr(ToString("K #%u", i)).SetDatabase(database))));
    }

    if (name == starter) {
        printf("Creating %u nodes\n", numNodes);
        RandomInstructionNode::CreateRIN(*kernels.front(), iterations, numNodes, debugLevel, seed, numKernels);
    }

    database->WaitForAllNodeEnd();

    return 0;
}



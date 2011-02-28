#include "Kernel.h"
#include "IQueue.h"
#include "VariantToJSON.h"
#include "JSONToVariant.h"
#include "XMLToVariant.h"
#include "VariantCPNLoader.h"
#include "ParseBool.h"
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>

using namespace CPN;

static bool LoadJSONConfig(VariantCPNLoader &loader,
        const std::string &filename) {
    std::ifstream def_file(filename.c_str());
    if (def_file) {
        JSONToVariant parser;
        parser.ParseStream(def_file);
        if (!parser.Done()) {
            std::cerr << "Parse error in " << filename << " at line " 
                << parser.GetLine() << " column " << parser.GetColumn()
                << std::endl;
            return false;
        } else {
            loader.MergeConfig(parser.Get());
        }
    } else {
        std::cerr << "Could not open the process network definition file\n";
        return false;
    }
    return true;
}

static bool LoadXMLConfig(VariantCPNLoader &loader,
        const std::string &filename) {
    std::ifstream def_file(filename.c_str());
    if (def_file) {
        XMLToVariant parser;
        parser.ParseStream(def_file);
        if (!parser.Done()) {
            std::cerr << "Parse error in " << filename << ": "
                << parser.GetMessage() << std::endl;
            return false;
        } else {
            loader.MergeConfig(parser.Get());
        }
    } else {
        std::cerr << "Could not open the process network definition file\n";
        return false;
    }
    return true;
}

static void PrintUsage(const char *progname) {
    using std::cerr;
    cerr << "Usage: " << progname << " [options]\n"
    "\t-m val\n\t--max=val\n\t\tSpecify the value to stop at.\n"
    "\t-c yes|no\n\t--config=yes|no\n\t\tSpecify if the config"
       " file should be loaded.\n"
    "\t-C\n\t\tPrint the internal config and exit.\n"
    "\t--ctx-port port\n\t\tSpecify the port the context daemon is"
       " listening on.\n"
    "\t--ctx-host host\n\t\tSpecify the host the context daemon is on.\n"
    "\t--port port\n\t\tSpecify the port the kernel should listen on.\n"
    "\t--host host\n\t\tSpecify the host the kernel should listen on.\n"
    "\t--name name\n\t\tSpecify the name the kernel should have,"
       " note that this must match the names in the nodemap file.\n"
    ;
}

enum Option_t {
    OP_CTX_PORT = 256,
    OP_CTX_HOST,
    OP_KERNEL_PORT,
    OP_KERNEL_HOST,
    OP_NAME
};

static const option long_options[] = {
    { "config", 1, 0, 'c'},
    { "max", 1, 0, 'm'},
    { "ctx-port", 1, 0, OP_CTX_PORT},
    { "ctx-host", 1, 0, OP_CTX_HOST},
    { "port", 1, 0, OP_KERNEL_PORT},
    { "host", 1, 0, OP_KERNEL_HOST},
    { "name", 1, 0, OP_NAME},
    { "help", 0, 0, 'h'},
    {0,0,0,0}
};
int main(int argc, char **argv) {
    uint64_t max_fib = 100;
    bool load_config = true;
    bool print_config = false;
    VariantCPNLoader loader;
    while (true) {
        int c = getopt_long(argc, argv, "m:c:Chj:x:", long_options, 0);
        if (c == -1) break;
        switch (c) {
        case 'm':
            max_fib = strtoull(optarg, 0, 10);
            break;
        case 'c':
            load_config = ParseBool(optarg);
            break;
        case 'C':
            print_config = true;
            break;
        case OP_CTX_PORT:
            loader.ContextPort(optarg);
            break;
        case OP_CTX_HOST:
            loader.ContextHost(optarg);
            break;
        case OP_KERNEL_PORT:
            loader.KernelPort(optarg);
            break;
        case OP_KERNEL_HOST:
            loader.KernelHost(optarg);
            break;
        case OP_NAME:
            loader.KernelName(optarg);
            break;
        case 'j':
            if (!LoadJSONConfig(loader, optarg)) {
                return 1;
            }
            break;
        case 'x':
            if (!LoadXMLConfig(loader, optarg)) {
                return 1;
            }
            break;
        case 'h':
        default:
            PrintUsage(*argv);
            return 1;
        }
    }

    if (print_config) {
        std::cout << PrettyJSON(loader.GetConfig(), true) << std::endl;
        return 0;
    }

    std::pair<bool, std::string> validate_result = loader.Validate();
    if (!validate_result.first) {
        std::cout << "Configuration did not validate:\n\t"
            << validate_result.second << std::endl;
        return 1;
    }

    Kernel kernel(loader.GetKernelAttr());

    if (load_config) {
        kernel.CreateExternalReader("result");
    }

    loader.Setup(&kernel);

    if (load_config) {
        QueueAttr qattr(2*sizeof(uint64_t), sizeof(uint64_t));
        qattr.SetWriter("Cons 2", "out1");
        qattr.SetExternalReader("result");
        kernel.CreateQueue(qattr);

        IQueue<uint64_t> result = kernel.GetExternalIQueue("result");
        uint64_t value;
        do {
            result.Dequeue(&value, 1);
            std::cout << "- " << value << std::endl;
        } while (value < max_fib);
        kernel.DestroyExternalEndpoint("result");
    } else {
        kernel.WaitNodeTerminate("result");
    }
    kernel.WaitForAllNodeEnd();

    return 0;
}


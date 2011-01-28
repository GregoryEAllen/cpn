#include "JSONToVariant.h"
#include "Kernel.h"
#include "PathUtils.h"
#include "VariantCPNLoader.h"
#include "Variant.h"
#include "VariantToJSON.h"
#include "XMLToVariant.h"

#include <iostream>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <unistd.h>

using CPN::Kernel;
using CPN::KernelAttr;
using CPN::shared_ptr;
using std::string;



static void LoadJSONConfig(const std::string &f, VariantCPNLoader &loader) {
    JSONToVariant parser;
    parser.ParseFile(f);
    if (!parser.Done()) {
        std::cerr << "Error parsing " << f << "on line "
            << parser.GetLine() << " column " << parser.GetColumn() << std::endl;
        return;
    }
    Variant val = parser.Get();
    loader.MergeConfig(val);
}

static void LoadXMLConfig(const std::string &f, VariantCPNLoader &loader) {
    XMLToVariant parser;
    parser.ParseFile(f);
    if (!parser.Done()) {
        std::cerr << "Error parsing " << f << "\n" << parser.GetMessage() << std::endl;
        return;
    }
    Variant val = parser.Get();
    loader.MergeConfig(val);
}

static void PrintHelp(const std::string &progname) {
    using std::cerr;
    cerr << "Usage: " << progname << " [options]\n";
    cerr << "\t-x config  Use config in XML format.\n";
    cerr << "\t-j config  Use config in JSON format.\n";
    cerr << "\t-c opts    Specify context options (see -chelp).\n";
    cerr << "\t-w node    Wait for node then terminate, if not specified waits for the kernel to terminate.\n";
    cerr << "\t-k opts    Comma seperated list of host options (see -khelp).\n";
    cerr << "\t-C         Print out in JSON format the internal configuration after parsing all options\n";
    cerr << "\t-n 'json'  Add or modify a node.\n";
    cerr << "\t-q 'json'  Add or modify a queue.\n";
    cerr << "\t-v         Verify config\n";
    cerr << "\nNote that options are overrided in the order they are in the command line.\n";
    cerr << "It is valid to specify multiple configuration files, they will be merged.\n";
}

static void PrintContextHelp(Variant &config) {
    using std::cerr;
    cerr << "The -c options takes a comma seperated list.\n";
    cerr << "The recognized set of values are:\n"
         << "\t host=xxx    Use a remote context located at xxx, else use local.\n"
         << "\t port=xxx    Specify the port for the remote context.\n"
         << "\t help        This message.\n";

}

static void PrintKernelHelp(Variant &config) {
    using std::cerr;
    cerr << "The -k options takes a comma seperated list.\n";
    cerr << "The recognized set of values are:\n"
         << "\t name=xxx    Use xxx as the name for the kernel.\n"
         << "\t host=xxx    Use xxx as the hostname to bind the kernel listening socket to.\n"
         << "\t port=xxx    Use xxx as the port to bind the kernel listening socket to.\n"
         << "\t [no-]d4r    Turn on or off D4R. (currently: " << (config["d4r"].AsBool() ? "on" : "off") << ")\n"
         << "\t [no-]gqmt   Turn on or off growing of queues when they request larger max threshold. (currently: " << (config["grow-queue-max-threshold"].AsBool() ? "on" : "off") << ")\n"
         << "\t [no-]sbqe   Turn on or off sollowing of broken queue exceptions. (currently: " << (config["swallow-broken-queue-exceptions"].AsBool() ? "on" : "off") << ")\n"
         << "\t lib=file    Load file as a shared object to be searched for nodes. May be set multiple times.\n"
         << "\t list=file   Add file to the list of lists for looking up where to load nodes from.\n"
         << "\t help        This message.\n";
}

static bool ParseContextSubOpts(VariantCPNLoader &loader) {
    enum { ophost, opport, ophelp, opend };
    const char *opts[opend + 1];
    opts[ophost] = "host";
    opts[opport] = "port";
    opts[ophelp] = "help";
    opts[opend] = 0;
    char *subopt = optarg;
    char *valuep = 0;
    while (*subopt != '\0') {
        int i = getsubopt(&subopt, (char * const *)opts, &valuep);
        if (i < 0) return false;
        switch (i) {
        case ophost:
            if (valuep == 0) {
                std::cerr << "The " << opts[ophost] << " options requires a parameter\n";
                return false;
            }
            loader.ContextHost(valuep);
            break;
        case opport:
            if (valuep == 0) {
                std::cerr << "The " << opts[opport] << " options requires a parameter\n";
                return false;
            }
            loader.ContextPort(valuep);
            break;
        case ophelp:
        default:
            return false;
        }
    }
    return true;
}

static bool ParseKernelSubOpts(VariantCPNLoader &loader) {
    enum { opd4r, opnd4r, opgqmt, opngqmt, opsbqe, opnsbqe, oplib, ophost, opport, ophelp, oplist,
       opname, opend };
    const char *opts[opend + 1];
    opts[opd4r] = "d4r";
    opts[opnd4r] = "no-d4r";
    opts[opgqmt] = "gqmt";
    opts[opngqmt] = "no-gqmt";
    opts[opsbqe] = "sbqe";
    opts[opnsbqe] = "no-sbqe";
    opts[oplib] = "lib";
    opts[ophost] = "host";
    opts[opport] = "port";
    opts[ophelp] = "help";
    opts[oplist] = "list";
    opts[opname] = "name";
    opts[opend] = 0;

    char *subopt = optarg;
    char *valuep = 0;
    while (*subopt != '\0') {
        int i = getsubopt(&subopt, (char * const *)opts, &valuep);
        if (i < 0) return false;
        switch (i) {
        case opd4r: // d4r
            loader.UseD4R(true);
            break;
        case opnd4r: // no-d4r
            loader.UseD4R(false);
            break;
        case opgqmt: // gqmt
            loader.GrowQueueMaxThreshold(true);
            break;
        case opngqmt: // no-gqmt
            loader.GrowQueueMaxThreshold(false);
            break;
        case opsbqe: // sbqe
            loader.SwallowBrokenQueueExceptions(true);
            break;
        case opnsbqe: // no-sbqe
            loader.SwallowBrokenQueueExceptions(false);
            break;
        case oplib: // lib
            if (valuep == 0) {
                std::cerr << "The " << opts[oplib] << " options requires a parameter\n";
                return false;
            }
            loader.AddLib(valuep);
            break;
        case oplist:
            if (valuep == 0) {
                std::cerr << "The " << opts[oplist] << " options requires a parameter\n";
                return false;
            }
            loader.AddLibList(valuep);
            break;
        case ophost:
            if (valuep == 0) {
                std::cerr << "The " << opts[ophost] << " options requires a parameter\n";
                return false;
            }
            loader.KernelHost(valuep);
            break;
        case opport:
            if (valuep == 0) {
                std::cerr << "The " << opts[opport] << " options requires a parameter\n";
                return false;
            }
            loader.KernelPort(valuep);
            break;
        case opname:
            if (valuep == 0) {
                std::cerr << "The " << opts[opport] << " options requires a parameter\n";
                return false;
            }
            loader.KernelName(valuep);
            break;
        case ophelp:
        default:
            return false;
        }
    }
    return true;
}

int main(int argc, char **argv) __attribute__((weak));
int main(int argc, char **argv) {
    Variant config;
    config["name"] = *argv;
    config["d4r"] = false;
    config["grow-queue-max-threshold"] = true;
    config["swallow-broken-queue-exceptions"] = false;
    std::string defaultlist = RealPath("node.list");
    if (!defaultlist.empty()) {
        config["liblist"].Append(defaultlist);
    }
    VariantCPNLoader loader(config);
    bool output_config = false;
    bool print_help = false;
    bool print_ctx_help = false;
    bool print_kernel_help = false;
    bool validate = false;
    while (true) {
        int c = getopt(argc, argv, "x:j:hw:c:k:Cn:q:v");
        if (c == -1) break;
        switch (c) {
        case 'x':
            LoadXMLConfig(optarg, loader);
            break;
        case 'j':
            LoadJSONConfig(optarg, loader);
            break;
        case 'w':
            config["wait-node"] = optarg;
            break;
        case 'c':
            if (!ParseContextSubOpts(loader)) {
                print_ctx_help = true;
            }
            break;
        case 'k':
            if (!ParseKernelSubOpts(loader)) {
                print_kernel_help = true;
            }
            break;
        case 'n':
            loader.AddNode(VariantFromJSON(optarg));
            break;
        case 'q':
            loader.AddQueue(VariantFromJSON(optarg));
            break;
        case 'C':
            output_config = true;
            break;
        case 'v':
            validate = true;
            break;
        case 'h':
            print_help = true;
            break;
        default:
            PrintHelp(*argv);
            return 1;
        }
    }
    if (print_help) {
        PrintHelp(*argv);
    }
    if (print_ctx_help) {
        PrintContextHelp(config);
    }
    if (print_kernel_help) {
        PrintKernelHelp(config);
    }
    if (output_config) {
        std::cout << PrettyJSON(config) << std::endl;
    }
    if (validate) {
        std::pair<bool, std::string> r = loader.Validate();
        if (!r.first) {
            std::cerr << "Error validating config: " << r.second << std::endl;
            return 1;
        }
    }
    if (print_help || print_ctx_help || output_config || print_kernel_help) {
        return 0;
    }
    // Load the kernel attr
    KernelAttr kattr = loader.GetKernelAttr();
    // Load the kernel
    Kernel kernel(kattr);
    // Load any nodes
    // Load any queues
    loader.Setup(&kernel);
    // if wait-node wait for it else wait
    if (config["wait-node"].IsNull()) {
        kernel.Wait();
    } else {
        kernel.WaitNodeTerminate(config["wait-node"].AsString());
    }
    return 0;
}


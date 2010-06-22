#include "Kernel.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"
#include "XMLToVariant.h"
#include "RemoteDatabase.h"
#include "VariantCPNLoader.h"

#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <string.h>

using std::string;

Variant LoadJSONConfig(const std::string &f) {
    Variant result;
    JSONToVariant parser;
    parser.ParseFile(f);
    if (!parser.Done()) {
        std::cerr << "Error parsing " << f << "on line "
            << parser.GetLine() << " column " << parser.GetColumn() << std::endl;
    } else {
        result = parser.Get();
    }
    return result;
}

Variant LoadXMLConfig(const std::string &f) {
    Variant result;
    XMLToVariant parser;
    parser.ParseFile(f);
    if (parser.Done()) {
        result = parser.Get();
    } else {
        std::cerr << "Error parsing f\n" << parser.GetMessage() << std::endl;
    }
    return result;
}

void PrintHelp(const std::string &progname) {
    using std::cerr;
    cerr << "Usage: " << progname << " [options]\n";
    cerr << "\t-x config  Use config in XML format.\n";
    cerr << "\t-j config  Use config in JSON format.\n";
    cerr << "\t-d opts    Specify database options (see -dhelp).\n";
    cerr << "\t-w node    Wait for node then terminate, if not specified waits for the kernel to terminate.\n";
    cerr << "\t-k opts    Comma seperated list of host options.\n";
    cerr << "\t           Valid options are host, port, and name. (eg. -kname=blah,host=localhost,port=1234)\n";
    cerr << "\nNote that all command line options override configuration options.\n";
    cerr << "It is valid to specify multiple configuration files, they will be merged.\n";
}

void PrintDatabaseHelp(Variant v) {
    using std::cerr;
    cerr << "The -d options takes a comma seperated list.\n";
    cerr << "The recognized set of values are:\n"
         << "\t [no-]d4r    Turn on or off D4R. (currently: " << (v["d4r"].IsTrue() ? "on" : "off") << ")\n"
         << "\t [no-]gqmt   Turn on or off growing of queues when they request larger max threshold. (currently: " << (v["grow-queue-max-threshold"].IsTrue() ? "on" : "off") << ")\n"
         << "\t [no-]sbqe   Turn on or off sollowing of broken queue exceptions. (currently: " << (v["swallow-broken-queue-exceptions"].IsTrue() ? "on" : "off") << ")\n"
         << "\t host=xxx    Use a remote database located at xxx, else use local.\n"
         << "\t port=xxx    Specify the port for the remote database.\n"
         << "\t lib=file    Load file as a shared object to be searched for nodes. Maybe be set multiple times.\n"
         << "\t help        This message.\n";

}

bool ParseDatabaseSubOpts(Variant &v) {
    enum { opd4r, opnd4r, opgqmt, opngqmt, opsbqe, opnsbqe, oplib, ophost, opport, ophelp, opend };
    char *opts[opend + 1];
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
    opts[opend] = 0;
    char *subopt = optarg;
    char *valuep = 0;
    while (*subopt != '\0') {
        switch (getsubopt(&subopt, opts, &valuep)) {
        case opd4r: // d4r
            v["d4r"] = true;
            break;
        case opnd4r: // no-d4r
            v["d4r"] = false;
            break;
        case opgqmt: // gqmt
            v["grow-queue-max-threshold"] = true;
            break;
        case opngqmt: // no-gqmt
            v["grow-queue-max-threshold"] = false;
            break;
        case opsbqe: // sbqe
            v["swallow-broken-queue-exceptions"] = true;
            break;
        case opnsbqe: // no-sbqe
            v["swallow-broken-queue-exceptions"] = false;
            break;
        case oplib: // lib
            if (valuep == 0) {
                std::cerr << "The " << opts[oplib] << " options requires a parameter\n";
                return false;
            }
            v["libs"].Append(valuep);
            break;
        case ophost:
            if (valuep == 0) {
                std::cerr << "The " << opts[ophost] << " options requires a parameter\n";
                return false;
            }
            v["host"] = valuep;
            break;
        case opport:
            if (valuep == 0) {
                std::cerr << "The " << opts[opport] << " options requires a parameter\n";
                return false;
            }
            v["port"] = valuep;
            break;
        case ophelp:
        default:
            return false;
        }
    }
    return true;
}

bool ParseKernelSubOpts(Variant &v) {
    char *const opts[] = {"name", "host", "port", 0};
    char *subopt = optarg;
    char *valuep = 0;
    while (*subopt != '\0') {
        int i = getsubopt(&subopt, opts, &valuep);
        if (i < 0) return false;
        v.At(opts[i]) = valuep;
    }
    return true;
}

int main(int argc, char **argv) {
    Variant config = Variant::ObjectType;
    config["database"]["d4r"] = false;
    config["database"]["grow-queue-max-threshold"] = true;
    config["database"]["swallow-broken-queue-exceptions"] = false;
    bool output_config = false;
    while (true) {
        int c = getopt(argc, argv, "x:j:hw:d:k:c");
        if (c == -1) break;
        switch (c) {
        case 'x':
            config["confs"].Append(LoadXMLConfig(optarg));
            break;
        case 'j':
            config["confs"].Append(LoadJSONConfig(optarg));
            break;
        case 'w':
            config["wait"] = optarg;
            break;
        case 'd':
            if (!ParseDatabaseSubOpts(config["database"])) {
                PrintDatabaseHelp(config["database"]);
                return 1;
            }
            break;
        case 'k':
            if (!ParseKernelSubOpts(config["kernel"])) {
                PrintHelp(*argv);
                return 1;
            }
            break;
        case 'c':
            output_config = true;
            break;
        case 'h':
        default:
            PrintHelp(*argv);
            return 1;
        }
    }
    if (output_config) {
        std::cout << PrettyJSON(config) << std::endl;
        return 0;
    }
    return 0;
    }


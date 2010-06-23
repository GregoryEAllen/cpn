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
using CPN::shared_ptr;
using CPN::Database;
using CPN::Kernel;
using CPN::KernelAttr;

static shared_ptr<Database> LoadDatabase(Variant v) {
    shared_ptr<Database> database;
    if (v.IsNull()) {
        database = Database::Local();
    } else {
        if (v["host"].IsNull() && v["port"].IsNull()) {
            database = Database::Local();
        } else {
            SockAddrList addrs = SocketAddress::CreateIP(
                    v["host"].AsString(),
                    v["port"].AsString()
                    );
            database = shared_ptr<Database>(new RemoteDatabase(addrs));
        }
        if (!v["d4r"].IsNull()) {
            database->UseD4R(v["d4r"].AsBool());
        }
        if (!v["swallow-broken-queue-exceptions"].IsNull()) {
            database->SwallowBrokenQueueExceptions(v["swallow-broken-queue-exceptions"].AsBool());
        }
        if (!v["grow-queue-max-threshold"].IsNull()) {
            database->GrowQueueMaxThreshold(v["grow-queue-max-threshold"].AsBool());
        }
        if (v["libs"].IsArray()) {
            for (Variant::ListIterator itr = v["libs"].ListBegin(); itr != v["libs"].ListEnd(); ++itr) {
                database->LoadSharedLib(itr->AsString());
            }
        }
    }
    return database;
}

static void MergeVariant(Variant &base, Variant &v) {
    switch (v.GetType()) {
    case Variant::ObjectType:
        if (base.IsObject()) {
            Variant::MapIterator itr = v.MapBegin();
            Variant::MapIterator end = v.MapEnd();
            while (itr != end) {
                MergeVariant(base[itr->first], itr->second);
                ++itr;
            }
        } else {
            base = v;
        }
        break;
    case Variant::ArrayType:
        if (base.IsArray()) {
            Variant::ListIterator itr = v.ListBegin();
            Variant::ListIterator end = v.ListEnd();
            while (itr != end) {
                base.Append(*itr);
                ++itr;
            }
        } else {
            base = v;
        }
        break;
    default:
        base = v;
        break;
    }
}

static void LoadJSONConfig(const std::string &f, Variant &config) {
    JSONToVariant parser;
    parser.ParseFile(f);
    if (!parser.Done()) {
        std::cerr << "Error parsing " << f << "on line "
            << parser.GetLine() << " column " << parser.GetColumn() << std::endl;
        return;
    }
    Variant val = parser.Get();
    MergeVariant(config, val);
}

static void LoadXMLConfig(const std::string &f, Variant &config) {
    XMLToVariant parser;
    parser.ParseFile(f);
    if (!parser.Done()) {
        std::cerr << "Error parsing " << f << "\n" << parser.GetMessage() << std::endl;
        return;
    }
    Variant val = parser.Get();
    MergeVariant(config, val);
}

static void PrintHelp(const std::string &progname) {
    using std::cerr;
    cerr << "Usage: " << progname << " [options]\n";
    cerr << "\t-x config  Use config in XML format.\n";
    cerr << "\t-j config  Use config in JSON format.\n";
    cerr << "\t-d opts    Specify database options (see -dhelp).\n";
    cerr << "\t-w node    Wait for node then terminate, if not specified waits for the kernel to terminate.\n";
    cerr << "\t-k opts    Comma seperated list of host options.\n";
    cerr << "\t           Valid options are host, port, and name. (eg. -kname=blah,host=localhost,port=1234)\n";
    cerr << "\t-c         Print out in JSON format the internal configuration after parsing all options\n";
    cerr << "\nNote that options are overrided in the order they are in the command line.\n";
    cerr << "It is valid to specify multiple configuration files, they will be merged.\n";
}

static void PrintDatabaseHelp(Variant &config) {
    Variant &v = config["database"];
    using std::cerr;
    cerr << "The -d options takes a comma seperated list.\n";
    cerr << "The recognized set of values are:\n"
         << "\t [no-]d4r    Turn on or off D4R. (currently: " << (v["d4r"].AsBool() ? "on" : "off") << ")\n"
         << "\t [no-]gqmt   Turn on or off growing of queues when they request larger max threshold. (currently: " << (v["grow-queue-max-threshold"].AsBool() ? "on" : "off") << ")\n"
         << "\t [no-]sbqe   Turn on or off sollowing of broken queue exceptions. (currently: " << (v["swallow-broken-queue-exceptions"].AsBool() ? "on" : "off") << ")\n"
         << "\t host=xxx    Use a remote database located at xxx, else use local.\n"
         << "\t port=xxx    Specify the port for the remote database.\n"
         << "\t lib=file    Load file as a shared object to be searched for nodes. Maybe be set multiple times.\n"
         << "\t help        This message.\n";

}

static bool ParseDatabaseSubOpts(Variant &config) {
    Variant &v = config["database"];
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

static bool ParseKernelSubOpts(Variant &config) {
    Variant &v = config;
    char *const opts[] = {"name", "host", "port", 0};
    char *subopt = optarg;
    char *valuep = 0;
    while (*subopt != '\0') {
        int i = getsubopt(&subopt, opts, &valuep);
        if (i < 0) return false;
        if (valuep == 0) {
            std::cerr << "The kernel option " << opts[i] << " requires a parameter.\n";
            return false;
        }
        v.At(opts[i]) = valuep;
    }
    return true;
}

int main(int argc, char **argv) {
    Variant config;
    config["name"] = *argv;
    config["database"]["d4r"] = false;
    config["database"]["grow-queue-max-threshold"] = true;
    config["database"]["swallow-broken-queue-exceptions"] = false;
    bool output_config = false;
    bool print_help = false;
    bool print_db_help = false;
    while (true) {
        int c = getopt(argc, argv, "x:j:hw:d:k:c");
        if (c == -1) break;
        switch (c) {
        case 'x':
            LoadXMLConfig(optarg, config);
            break;
        case 'j':
            LoadJSONConfig(optarg, config);
            break;
        case 'w':
            config["wait-node"] = optarg;
            break;
        case 'd':
            if (!ParseDatabaseSubOpts(config)) {
                print_db_help = true;
            }
            break;
        case 'k':
            if (!ParseKernelSubOpts(config)) {
                print_help = true;
            }
            break;
        case 'c':
            output_config = true;
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
    if (print_db_help) {
        PrintDatabaseHelp(config);
    }
    if (output_config) {
        std::cout << PrettyJSON(config) << std::endl;
    }
    if (print_help || print_db_help || output_config) {
        return 0;
    }
    // Load database

    shared_ptr<Database> database = LoadDatabase(config["database"]);
    // Load the kernel attr
    KernelAttr kattr = VariantCPNLoader::GetKernelAttr(config);
    kattr.SetDatabase(database);
    // Load the kernel
    Kernel kernel(kattr);
    // Load any nodes
    // Load any queues
    VariantCPNLoader::Setup(&kernel, config);
    // if wait-node wait for it else wait
    if (config["wait-node"].IsNull()) {
        kernel.Wait();
    } else {
        kernel.WaitNodeTerminate(config["wait-node"].AsString());
    }
    return 0;
}


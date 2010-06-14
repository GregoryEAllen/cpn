#include "Kernel.h"
#include "Variant.h"
#include "VariantCPNLoader.h"
#include "JSONToVariant.h"
#include "RemoteDatabase.h"

#include <unistd.h>
#include <stdio.h>
#include <libgen.h>


int hb_main(int argc, char **argv);

int vb_main(int argc, char **argv);

int localbeamform(int argc, char **argv) {
    static const char VALID_OPTS[] = "h:p:";
    static const char HELP_OPTS[] = "%sUsage %s [options] <config>\n"
        "\t-h n\t Host name\n"
        "\t-p n\t Port name\n"
        ;
    bool procOpts = true;
    bool useremote = false;
    std::string hostname, portname;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'h':
            useremote = true;
            hostname = optarg;
            break;
        case 'p':
            useremote = true;
            portname = optarg;
            break;
        case -1:
            procOpts = false;
            break;
        default:
            fprintf(stderr, HELP_OPTS, "", argv[0]);
            return 0;
        }
    }
    if (argc <= optind) {
        fprintf(stderr, HELP_OPTS, "Missing config\n", argv[0]);
        return 1;
    }
    JSONToVariant parser;
    parser.ParseFile(argv[optind]);
    if (!parser.Done()) {
        fprintf(stderr, "Error on line %u column %u in %s\n",
                parser.GetLine(), parser.GetColumn(), argv[optind]);
        return 1;
    }
    Variant config = parser.Get();
    Variant subconfig = config;
    if (argc > optind + 1) {
        subconfig = config.At(argv[optind + 1]);
    }
    CPN::KernelAttr kattr = VariantCPNLoader::GetKernelAttr(subconfig);
    if (!useremote) {
        if (subconfig["database"].IsObject()) {
            Variant db = subconfig["database"];
            useremote = true;
            hostname = db["hostname"].AsString();
            portname = db["portname"].AsString();
        }
    }
    if (useremote) {
        SockAddrList addrs = SocketAddress::CreateIP(hostname, portname);
        CPN::shared_ptr<CPN::Database> database = 
            CPN::shared_ptr<CPN::Database>(new RemoteDatabase(addrs));
        kattr.SetDatabase(database);
    }
    CPN::Kernel kernel(kattr);
    VariantCPNLoader::Setup(&kernel, subconfig);
    kernel.WaitForAllNodeEnd();
    return 0;
}

int main(int argc, char **argv) {
    char *selfname = basename(*argv);
    if (*selfname == 'v') {
        return vb_main(argc, argv);
    } else if (*selfname == 'h') {
        return hb_main(argc, argv);
    } else {
        return localbeamform(argc, argv);
    }
}


#include "RemoteDatabaseDaemon.h"
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

const char* const VALID_OPTS = "hdl:";

const char* const HELP_OPTS = "Usage: %s [-h|-d] [-l loglevel ] <address> <port>\n"
"\t-h\t This message.\n"
"\t-d\t Go into background\n"
"\t-l\t Level of output 0 for errors only, 1 for log, 2 for database status, etc. (default: 1)\n"
;

int main(int argc, char **argv) {

	signal(SIGPIPE,SIG_IGN);
    bool background = false;
    bool procOpts = true;
    int loglevel = 1;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'd':
            background = true;
            break;
        case 'l':
            loglevel = atoi(optarg);
            break;
        case 'h':
            printf(HELP_OPTS, argv[0]);
            return 0;
        case -1:
            procOpts = false;
            break;
        default:
            printf(HELP_OPTS, argv[0]);
            return 1;
        }
    }
    if (background) {
        if (0 > daemon(1, 0)) {
            perror("Failed to go into background");
            return 1;
        }
    }
    if (argc != optind + 2) {
        printf("Not enough arguments must have address and port to listen on");
        return 1;
    }
    SockAddrList addrlist = SocketAddress::CreateIP(argv[optind], argv[optind+1]);
    RemoteDatabaseDaemon rdd(addrlist);
    rdd.DebugLevel(loglevel);
    rdd.Run();
    return 0;
}




#include "SpreadDBServ.h"
#include <unistd.h>

const char* const VALID_OPTS = "hdn:s:c:";

const char* const HELP_OPTS = "Usage: %s \n"
"\t-n name\t connection name\n"
"\t-s name\t Self name\n"
"\t-c client group\n"
"\t-h\t This message.\n"
"\t-d\t Go into background\n"
;

int main(int argc, char **argv) {

    std::string spread_name;
    std::string self = "server";
    std::string clientgroup = "clients";
    bool background = false;
    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'n':
            spread_name = optarg;
            break;
        case 's':
            self = optarg;
            break;
        case 'c':
            clientgroup = optarg;
        case 'd':
            background = true;
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

    printf("Connecting to %s as %s broadcasting to %s\n",
            spread_name.c_str(), self.c_str(), clientgroup.c_str());
    SpreadDBServ server(spread_name, self, clientgroup);
    server.Run();
    return 0;
}





#include "D4RTester.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "Directory.h"
#include "Logger.h"
#include <vector>

const char VALID_OPS[] = "l:";


int main(int argc, char **argv) {
    bool procOpts = true;
    int loglevel = Logger::DEBUG;
    while (procOpts) {
        int opt = getopt(argc, argv, VALID_OPS);
        switch (opt) {
        case 'l':
            loglevel = atoi(optarg);
            break;
        case -1:
            procOpts = false;
            break;
        case '?':
            printf("Unknown option\n");
            return -1;
        default:
            return -1;
        }
    }

    std::string dirname = "D4R/Tests";
    if (argc > optind) {
        dirname = argv[optind];
    }

    std::string ext = ".test";
    LoggerStdOutput loggerout(loglevel);

    for ( Directory dir(dirname); !dir.End() ; dir.Next() ) {
        try {
            if (!dir.IsRegularFile()) continue;
            std::string fpath = dir.BaseName();
            if (fpath[0] == '.' || fpath[0] == '_') continue;
            if (fpath.size() < ext.size() ||
                    fpath.substr(fpath.size() - ext.size(), ext.size()) != ext) {
                continue; 
            }
            fpath = dir.FullName();
            std::vector<char> buf(4096);

            printf("Processing %s\n", fpath.c_str());
            FILE *f = fopen(fpath.c_str(), "r");
            if (!f) {
                perror("Could not open file");
                continue;
            }
            JSONToVariant parse;
            while (!feof(f)) {
                unsigned numread = fread(&buf[0], 1, buf.size(), f);
                unsigned numparsed = parse.Parse(&buf[0], numread);
                if (parse.Error()) {
                    printf("Unabled to parse line: %u column: %u\n", parse.GetLine(), parse.GetColumn());
                    break;
                }
            }
            fclose(f);

            if (!parse.Done()) {
                printf("****************** Failure! *******************\n");
                return 1;
            }
            Variant conf = parse.Get();

            D4R::Tester tester;
            tester.Output(&loggerout);
            tester.LogLevel(loggerout.LogLevel());
            tester.Setup(conf);
            tester.Run();

            if (tester.Success()) {
                printf("Success!\n");
            } else {
                printf("****************** Failure! *******************\n");
                return 1;
            }
        } catch (const std::exception &e) {
            printf("Error: %s\n", e.what());
        }
    }

    return 0;
}


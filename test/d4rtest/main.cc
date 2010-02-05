

#include "D4RTester.h"
#include "Variant.h"
#include "Directory.h"
#include "Logger.h"
#include <vector>

const char VALID_OPS[] = "";


int main(int argc, char **argv) {
    bool procOpts = true;
    while (procOpts) {
        int opt = getopt(argc, argv, VALID_OPS);
        switch (opt) {
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

    std::string dirname = ".";
    if (argc > optind) {
        dirname = argv[optind];
    }
    std::string ext = ".test";

    LoggerStdOutput loggerout(Logger::TRACE);

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
            std::vector<char> buf(dir.Size());

            printf("Processing %s\n", fpath.c_str());
            FILE *f = fopen(fpath.c_str(), "r");
            if (!f) {
                perror("Could not open file");
                continue;
            }
            if (fread(&buf[0], 1, buf.size(), f) != buf.size()) {
                printf("Unable to read file\n");
                continue;
            }
            fclose(f);

            Variant conf = Variant::FromJSON(buf);

            D4R::Tester tester;
            tester.Output(&loggerout);
            tester.LogLevel(loggerout.LogLevel());
            tester.Setup(conf);
            tester.Run();

            printf("Done\n");
        } catch (const std::exception &e) {
            printf("Error: %s\n", e.what());
        }
    }

}


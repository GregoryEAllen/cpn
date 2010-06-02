#include "HBeamformer.h"
#include "LoadFromFile.h"
#include "Assert.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex>
#include <iostream>

using std::complex;

namespace HB {
    const char* const VALID_OPTS = "hi:o:e";

    const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
    "\t-i filename\t Use input file (default stdin)\n"
    "\t-o filename\t Use output file (default stdout)\n"
    "\t-e\t Estimate FFT algorithm rather than measure.\n"
    ;

    int main(int argc, char **argv) {
        bool procOpts = true;
        std::string input_file;
        std::string output_file;
        bool estimate = false;
        while (procOpts) {
            switch (getopt(argc, argv, VALID_OPTS)) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'e':
                estimate = true;
                break;
            case -1:
                procOpts = false;
                break;
            case 'h':
            default:
                fprintf(stderr, HELP_OPTS, argv[0]);
                return 0;
            }
        }

        if (argc <= optind) {
            fprintf(stderr, "Not enough parameters, need coefficient file\n");
            return 1;
        }
        fprintf(stderr, "Loading..");
        std::auto_ptr<HBeamformer> former = HBLoadFromFile(argv[optind], estimate);
        fprintf(stderr, ". Done\n");

        FILE *fin = stdin;
        FILE *fout = stdout;

        if (!input_file.empty()) {
            fin = fopen(input_file.c_str(), "r");
            ASSERT(fin);
        }
        if (!output_file.empty()) {
            fout = fopen(output_file.c_str(), "w");
            ASSERT(fout);
        }

        std::vector< complex<float> > input( former->Length() * former->NumStaves(), 0);
        std::vector< complex<float> > output( former->Length() * former->NumBeams(), 0);
        unsigned len = former->Length() * sizeof(complex<float>);

        fprintf(stderr, "Reading Input..");
        DataFromFile(fin, &input[0], len, len, former->NumStaves());

        fprintf(stderr, ". Done\n");

        fprintf(stderr, "Beamform..");
        former->Run(&input[0], former->Length(), &output[0], former->Length());
        fprintf(stderr, ". Done\n");
        former->PrintTimes();

        fprintf(stderr, "Writing Output..");
        fprintf(stderr, ". Done\n");

        DataToFile(fout, &output[0], len, len, former->NumBeams());
        fprintf(stderr, "Cleanup..");
        former.reset();
        fprintf(stderr, ". Done\n");
        return 0;
    }

}

namespace VB {
    const char* const VALID_OPTS = "hi:o:";

    const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
    "\t-i filename\t Use input file (default stdin)\n"
    "\t-o filename\t Use output file (default stdout)\n"
    ;

    int main(int argc, char **argv) {
        bool procOpts = true;
        std::string input_file;
        std::string output_file;
        while (procOpts) {
            switch (getopt(argc, argv, VALID_OPTS)) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case -1:
                procOpts = false;
                break;
            case 'h':
            default:
                fprintf(stderr, HELP_OPTS, argv[0]);
                return 0;
            }
        }

        if (argc <= optind) {
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 1;
        }

        return 0;
    }
}


int main(int argc, char **argv) {
    if (*basename(*argv) == 'v') {
        return VB::main(argc, argv);
    } else {
        return HB::main(argc, argv);
    }
}

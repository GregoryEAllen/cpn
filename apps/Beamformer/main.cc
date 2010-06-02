#include "HBeamformer.h"
#include "VBeamformer.h"
#include "LoadFromFile.h"
#include "Assert.h"
#include "ErrnoException.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <complex>
#include <iostream>
#include <sys/time.h>
#include <libgen.h>

using std::complex;

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

namespace HB {
    const char* const VALID_OPTS = "hi:o:er:";

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
        unsigned repititions = 1;
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
            case 'r':
                repititions = atoi(optarg);
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

        for (unsigned j = 0; j < repititions; ++j) {
            fprintf(stderr, "Beamform..");
            former->Run(&input[0], former->Length(), &output[0], former->Length());
            fprintf(stderr, ". Done\n");
            former->PrintTimes();
        }

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
    const char* const VALID_OPTS = "hi:o:a:f:r:";

    const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
    "\t-i filename\t Use input file (default stdin)\n"
    "\t-o filename\t Use output file (default stdout)\n"
    "\t-f n\t Do only the given fan\n"
    ;

    int main(int argc, char **argv) {
        bool procOpts = true;
        std::string input_file;
        std::string output_file;
        VBeamformer::Algorithm_t algo = VBeamformer::SSE_VECTOR;
        unsigned fan = -1;
        unsigned repititions = 1;
        while (procOpts) {
            switch (getopt(argc, argv, VALID_OPTS)) {
            case 'i':
                input_file = optarg;
                break;
            case 'o':
                output_file = optarg;
                break;
            case 'a':
                algo = (VBeamformer::Algorithm_t)atoi(optarg);
                if (algo < VBeamformer::ALGO_BEGIN || algo >= VBeamformer::ALGO_END) {
                    fprintf(stderr, "Unknown algorithm number %d\n", algo);
                    return 1;
                }
                break;
            case 'f':
                fan = atoi(optarg);
                break;
            case 'r':
                repititions = atoi(optarg);
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
        fprintf(stderr, "Loading..");
        std::auto_ptr<VBeamformer> former = VBLoadFromFile(argv[optind]);
        fprintf(stderr, ". Done\n");
        former->SetAlgorithm(algo);

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
        unsigned numFans = former->NumFans();
        unsigned stride = 8192 + former->FilterLen();
        unsigned numSamples = stride;
        unsigned numElemsPerStave = former->NumElemsPerStave();
        unsigned numStaves = 256;
        unsigned numOutSamples = 0;

        std::vector< complex<short> > input( numSamples * numElemsPerStave * numStaves, 0);
        std::vector< complex<float> > output( numFans * numSamples * numStaves, 0);

        fprintf(stderr, "Reading Input..");
        unsigned len = numSamples * sizeof(complex<short>);
        len = DataFromFile(fin, &input[0], len, len, numStaves * numElemsPerStave);
        numSamples = len / sizeof(complex<short>);

        fprintf(stderr, ". Done\n");

        for (unsigned j = 0; j < repititions; ++j) {
            fprintf(stderr, "Beamform(%d)..", algo);
            double start = getTime();
            if (fan > numFans) {
                for (unsigned i = 0; i < numFans; ++i) {
                    numOutSamples = former->Run(&input[0], stride, numStaves,
                        numSamples, i, &output[i * stride * numStaves], stride);
                }
            } else {
                numOutSamples = former->Run(&input[0], stride, numStaves,
                    numSamples, fan, &output[fan * stride * numStaves], stride);
            }
            fprintf(stderr, ". Done (%f ms)\n", (getTime() - start) * 1000);
        }

        fprintf(stderr, "Writing Output..");
        unsigned lenout = numOutSamples * sizeof(complex<float>);
        if (fan > numFans) {
            for (unsigned i = 0; i < numFans; ++i) {
                DataToFile(fout, &output[i * stride * numStaves], lenout, stride*sizeof(complex<float>), numStaves);
            }
        } else {
            DataToFile(fout, &output[fan * stride * numStaves], lenout, stride*sizeof(complex<float>), numStaves);
        }
        fprintf(stderr, ". Done\n");
        fprintf(stderr, "Cleanup..");
        former.reset();
        fprintf(stderr, ". Done\n");

        return 0;
    }
}


int main(int argc, char **argv) {
    char *selfname = basename(*argv);
    if (*selfname == 'v') {
        return VB::main(argc, argv);
    } else if (*selfname == 'h') {
        return HB::main(argc, argv);
    } else {
        fprintf(stderr, "urk!\n");
    }
}

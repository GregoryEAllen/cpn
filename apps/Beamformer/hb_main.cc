#include "HBeamformer.h"
#include "LoadFromFile.h"
#include "Assert.h"
#include "FlowMeasure.h"
#include "NumProcs.h"
#include <complex>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef _OPENMP
#include <omp.h>
#endif

using std::complex;

static const char* const VALID_OPTS = "hi:o:er:np:";

static const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-r num\t Run num times\n"
"\t-n \t No output just time\n"
;

int hb_main(int argc, char **argv) {
    bool procOpts = true;
    std::string input_file;
    std::string output_file;
    bool estimate = false;
    bool nooutput = false;
    unsigned repetitions = 1;
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
            repetitions = atoi(optarg);
            break;
        case 'n':
            nooutput = true;
            break;
        case 'p':
            {
                int num_threads = atoi(optarg);
                SetNumProcs(num_threads);
#ifdef _OPENMP
                omp_set_num_threads(num_threads);
#endif
            }
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
    if (!output_file.empty() && !nooutput) {
        fout = fopen(output_file.c_str(), "w");
        ASSERT(fout);
    }

    std::vector< complex<float> > input( former->Length() * former->NumStaves(), 0);
    std::vector< complex<float> > output( former->Length() * former->NumBeams(), 0);
    unsigned len = former->Length() * sizeof(complex<float>);

    fprintf(stderr, "Reading Input..");
    DataFromFile(fin, &input[0], len, len, former->NumStaves());

    fprintf(stderr, ". Done\n");

    FlowMeasure measure;
    measure.Start();
    for (unsigned j = 0; j < repetitions; ++j) {
        fprintf(stderr, "Beamform..");
        former->Run(&input[0], former->Length(), &output[0], former->Length());
        fprintf(stderr, ". Done\n");
        former->PrintTimes();
        measure.Tick(former->Length());
    }
    fprintf(stderr,
            "Output:\nAvg:\t%f Hz\nMax:\t%f Hz\nMin:\t%f Hz\n",
            measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());

    if (!nooutput) {
        fprintf(stderr, "Writing Output..");
        DataToFile(fout, &output[0], len, len, former->NumBeams());
        fprintf(stderr, ". Done\n");
    }
    fprintf(stderr, "Cleanup..");
    if (!input_file.empty()) {
        fclose(fin);
    }
    if (!output_file.empty()) {
        fclose(fout);
    }
    former.reset();
    fprintf(stderr, ". Done\n");
    return 0;
}


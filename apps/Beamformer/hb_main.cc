#include "HBeamformer.h"
#include "LoadFromFile.h"
#include "Assert.h"
#include "FlowMeasure.h"
#include "NumProcs.h"
#include <complex>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory>

#ifdef _OPENMP
#include <omp.h>
#endif

using std::complex;

static const char* const VALID_OPTS = "h:i:o:er:np:l";

static const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-r num\t Run num times\n"
"\t-n \t No output just time\n"
"\t-h file\t Use file for horizontal coefficients.\n"
"\t-l \t Force the input to be the beamformer length by truncation or repetition, rather than zero extension.\n"
;

int hb_main(int argc, char **argv) {
    std::string input_file;
    std::string output_file;
    std::string horizontal_config;
    bool estimate = false;
    bool nooutput = false;
    unsigned repetitions = 1;
    bool forced_length = false;
    while (true) {
        int c = getopt(argc, argv, VALID_OPTS);
        if (c == -1) break;
        switch (c) {
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
        case 'h':
            horizontal_config = optarg;
            break;
        case 'l':
            forced_length = true;
            break;
        default:
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 0;
        }
    }

    if (horizontal_config.empty()) {
        fprintf(stderr, "Not enough parameters, need coefficient file\n");
        fprintf(stderr, HELP_OPTS, argv[0]);
        return 1;
    }
    fprintf(stderr, "Loading..");
    std::auto_ptr<HBeamformer> former = HBLoadFromFile(horizontal_config.c_str(), estimate);
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
    unsigned data_len = DataFromFile(fin, &input[0], len, len, former->NumStaves());
    if (forced_length) {
        while (data_len < len) {
            unsigned copy_len = std::min(data_len, len - data_len);
            memcpy(&input[data_len], &input[0], copy_len);
            data_len += copy_len;
        }
    }

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


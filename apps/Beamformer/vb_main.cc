#include "VBeamformer.h"
#include "LoadFromFile.h"
#include "Assert.h"
#include "ErrnoException.h"
#include "FlowMeasure.h"
#include "NumProcs.h"
#include <complex>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>

using std::complex;

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

static const char* const VALID_OPTS = "hi:o:a:f:r:p:";

static const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-f n\t Do only the given fan\n"
"\t-a n\t Use algorithm n\n"
"\t-r n\t Repeat n times\n"
;

int vb_main(int argc, char **argv) {
    bool procOpts = true;
    std::string input_file;
    std::string output_file;
    VBeamformer::Algorithm_t algo = VBeamformer::SSE_VECTOR;
    unsigned fan = -1;
    unsigned repetitions = 1;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'i':
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'p':
            SetNumProcs(atoi(optarg));
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
            repetitions = atoi(optarg);
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

    FlowMeasure measure;
    measure.Start();
    for (unsigned j = 0; j < repetitions; ++j) {
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
        measure.Tick(numOutSamples);
    }
    fprintf(stderr, "Avg:\t%f Hz\nMax:\t%f Hz\nMin:\t%f Hz\n", measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());

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


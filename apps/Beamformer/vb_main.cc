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
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

static const unsigned BLOCKSIZE = 8192;
using std::complex;

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

static const char* const VALID_OPTS = "hi:o:a:f:r:p:v:ln";

static const char* const HELP_OPTS = "Usage: %s <coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-f n\t Do only the given fan\n"
"\t-a n\t Use algorithm n\n"
"\t-r n\t Repeat n times\n"
"\t-n\t No output.\n"
"\t-v file\t Coefficient file.\n"
"\t-l \t Force the input to be the full input length by repetition rather than zero fill.\n"
;

int vb_main(int argc, char **argv) {
    std::string input_file;
    std::string output_file;
    std::string vertical_config;
    VBeamformer::Algorithm_t algo = VBeamformer::SSE_VECTOR;
    unsigned fan = -1;
    unsigned repetitions = 1;
    bool forced_length = false;
    bool nooutput = false;
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
        case 'p':
            {
                int num_threads = atoi(optarg);
                SetNumProcs(num_threads);
#ifdef _OPENMP
                omp_set_num_threads(num_threads);
#endif
            }
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
        case 'n':
            nooutput = true;
            break;
        case 'v':
            vertical_config = optarg;
            break;
        case 'l':
            forced_length = true;
            break;
        case 'h':
        default:
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 0;
        }
    }

    if (vertical_config.empty()) {
        fprintf(stderr, "Coefficient file required\n");
        fprintf(stderr, HELP_OPTS, argv[0]);
        return 1;
    }
    fprintf(stderr, "Loading..");
    std::auto_ptr<VBeamformer> former = VBLoadFromFile(vertical_config.c_str());
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
    unsigned stride = BLOCKSIZE + former->FilterLen();
    unsigned numSamples = BLOCKSIZE;
    unsigned numElemsPerStave = former->NumElemsPerStave();
    unsigned numStaves = 256;
    unsigned numOutSamples = 0;

    std::vector< complex<short> > input( numSamples * numElemsPerStave * numStaves, 0);
    std::vector< complex<float> > output( numFans * stride * numStaves, 0);

    fprintf(stderr, "Reading Input..");
    unsigned len = numSamples * sizeof(complex<short>);
    len = DataFromFile(fin, &input[0], len, len, numStaves * numElemsPerStave);
    numSamples = len / sizeof(complex<short>);
    if (forced_length) {
        while (numSamples < BLOCKSIZE) {
            unsigned num_more = BLOCKSIZE - numSamples;
            num_more = std::min(numSamples, num_more);
            memcpy(&input[numSamples], &input[0], num_more * sizeof(complex<short>));
            numSamples += num_more;
        }
    } else {
        // std::vector zero fills on allocation
        numSamples = BLOCKSIZE;
    }

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
        measure.Tick(numSamples);
    }
    fprintf(stderr, "Avg:\t%f Hz\nMax:\t%f Hz\nMin:\t%f Hz\n", measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());

    if (!nooutput) {
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


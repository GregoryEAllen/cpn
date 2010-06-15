#include "HBeamformer.h"
#include "VBeamformer.h"
#include "LoadFromFile.h"
#include "ErrnoException.h"
#include "Assert.h"
#include "FlowMeasure.h"
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


static const char* const VALID_OPTS = "hi:o:er:na:";

static const char* const HELP_OPTS = "Usage: %s <vertical coefficient file> <horizontal coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-r num\t Run num times\n"
"\t-a n\t Use algorithm n for vertical\n"
"\t-n \t No output, just time\n"
;


int ompbf_main(int argc, char **argv) {
    bool procOpts = true;
    std::string input_file;
    std::string output_file;
    VBeamformer::Algorithm_t algo = VBeamformer::SSE_VECTOR;
    bool estimate = false;
    unsigned repetitions = 1;
    bool nooutput = false;
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
        case 'a':
            algo = (VBeamformer::Algorithm_t)atoi(optarg);
            if (algo < VBeamformer::ALGO_BEGIN || algo >= VBeamformer::ALGO_END) {
                fprintf(stderr, "Unknown algorithm number %d\n", algo);
                return 1;
            }
            break;
        case 'r':
            repetitions = atoi(optarg);
            break;
        case 'n':
            nooutput = true;
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

    if (argc <= optind + 1) {
        fprintf(stderr, "Not enough parameters, need coefficient files\n");
        return 1;
    }
    fprintf(stderr, "Loading..");
    std::auto_ptr<VBeamformer> vformer = VBLoadFromFile(argv[optind]);
    std::auto_ptr<HBeamformer> hformer = HBLoadFromFile(argv[optind + 1], estimate);
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

    unsigned numFans = vformer->NumFans();
    unsigned stride = 8192 + vformer->FilterLen();
    unsigned numSamples = stride;
    unsigned numElemsPerStave = vformer->NumElemsPerStave();
    unsigned numStaves = 256;
    unsigned numOutSamples = 0;

    std::vector< complex<short> > vinput( numSamples * numElemsPerStave * numStaves, 0);
    std::vector< complex<float> > voutput( numFans * numSamples * numStaves, 0);
    std::vector< complex<float> > houtput( numFans * hformer->Length() * hformer->NumBeams(), 0);

    fprintf(stderr, "Reading Input..");
    unsigned len = numSamples * sizeof(complex<short>);
    len = DataFromFile(fin, &vinput[0], len, len, numStaves * numElemsPerStave);
    numSamples = len / sizeof(complex<short>);

    fprintf(stderr, ". Done\n");

    FlowMeasure measure;
    measure.Start();
    for (unsigned rep = 0; rep < repetitions; ++rep) {
        fprintf(stderr, "Vertical Beamform(%d)..", algo);
        double start = getTime();
        for (unsigned i = 0; i < numFans; ++i) {
            numOutSamples = vformer->Run(&vinput[0], stride, numStaves,
                numSamples, i, &voutput[i * stride * numStaves], stride);
        }
        fprintf(stderr, ". Done (%f ms)\n", (getTime() - start) * 1000);
        fprintf(stderr, "Horizontal Beamform..");
        start = getTime();
        for (unsigned i = 0; i < numFans; ++i) {
            hformer->Run(&voutput[i * stride * numStaves], numOutSamples, &houtput[i * hformer->Length() * hformer->NumBeams()], hformer->Length());
        }
        fprintf(stderr, ". Done (%f ms)\n", (getTime() - start) * 1000);
        measure.Tick(numSamples);
    }
    fprintf(stderr, "Avg:\t%f hz\nMax:\t%f hz\nMin:\t%f hz\n", measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());

    if (!nooutput) {
        fprintf(stderr, "Writing Output..");
        for (unsigned i = 0; i < numFans; ++i) {
            unsigned len = hformer->Length() * sizeof(complex<float>);
            DataToFile(fout, &houtput[i * hformer->Length() * hformer->NumBeams()], len, len, hformer->NumBeams());
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
    hformer.reset();
    vformer.reset();
    fprintf(stderr, ". Done\n");

    return 0;
}


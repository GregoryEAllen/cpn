#include "Assert.h"
#include "ErrnoException.h"
#include "FanVBeamformer.h"
#include "FlowMeasure.h"
#include "HBeamformer.h"
#include "LoadFromFile.h"
#include "NumProcs.h"
#include <complex>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

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


static const char* const VALID_OPTS = "h:v:i:o:er:na:p:l";

static const char* const HELP_OPTS = "Usage: %s <vertical coefficient file> <horizontal coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-r num\t Run num times\n"
"\t-a n\t Use algorithm n for vertical\n"
"\t-n \t No output, just time\n"
"\t-v file\t Use file for vertial coefficients.\n"
"\t-h file\t Use file for horizontal coefficients.\n"
"\t-l \t Force the input to be the full input length by repetition rather than zero fill.\n"
;


int ompbf_main(int argc, char **argv) {
    std::string input_file;
    std::string output_file;
    std::string vertical_config;
    std::string horizontal_config;
    FanVBeamformer::Algorithm_t algo = FanVBeamformer::AUTO;
    bool estimate = false;
    unsigned repetitions = 1;
    bool nooutput = false;
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
        case 'p':
            {
                int num_threads = atoi(optarg);
                SetNumProcs(num_threads);
#ifdef _OPENMP
                omp_set_num_threads(num_threads);
#endif
            }
            break;
        case 'e':
            estimate = true;
            break;
        case 'a':
            algo = (FanVBeamformer::Algorithm_t)atoi(optarg);
            if (algo < FanVBeamformer::ALGO_BEGIN || algo >= FanVBeamformer::ALGO_END) {
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
        case 'v':
            vertical_config = optarg;
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

    if (vertical_config.empty() || horizontal_config.empty()) {
        fprintf(stderr, "Not enough parameters, need coefficient files\n");
        return 1;
    }
    fprintf(stderr, "Loading..");
    std::auto_ptr<FanVBeamformer> vformer = FanVBLoadFromFile(vertical_config.c_str());
    vformer->SetAlgorithm(algo);
    std::auto_ptr<HBeamformer> hformer = HBLoadFromFile(horizontal_config.c_str(), estimate);
    fprintf(stderr, ". Done\n");


    FILE *fin = stdin;
    FILE *fout = stdout;

    if (!input_file.empty()) {
        fin = fopen(input_file.c_str(), "r");
        ASSERT(fin);
        fprintf(stderr, "Using %s as input.\n", input_file.c_str());
    }
    if (!output_file.empty()) {
        fout = fopen(output_file.c_str(), "w");
        ASSERT(fout);
    }

    unsigned numFans = vformer->NumFans();
    unsigned stride = BLOCKSIZE + vformer->FilterLen();
    unsigned numSamples = BLOCKSIZE;
    unsigned numElemsPerStave = vformer->NumElemsPerStave();
    unsigned numStaves = 256;
    unsigned numOutSamples = 0;

    std::vector< complex<short> > vinput( numSamples * numElemsPerStave * numStaves, 0);
    std::vector< complex<float> > voutput( numFans * stride * numStaves, 0);
    std::vector< complex<float> > houtput( numFans * hformer->Length() * hformer->NumBeams(), 0);

    fprintf(stderr, "Reading Input..");
    unsigned len = numSamples * sizeof(complex<short>);
    len = DataFromFile(fin, &vinput[0], len, len, numStaves * numElemsPerStave);
    numSamples = len / sizeof(complex<short>);
    if (forced_length) {
        while (numSamples < BLOCKSIZE) {
            unsigned num_more = BLOCKSIZE - numSamples;
            num_more = std::min(numSamples, num_more);
            memcpy(&vinput[numSamples], &vinput[0], num_more * sizeof(complex<short>));
            numSamples += num_more;
        }
    } else {
        // std::vector zero fills on allocation
        numSamples = BLOCKSIZE;
    }

    fprintf(stderr, ". Done\n");

    FlowMeasure measure;
    measure.Start();
    for (unsigned rep = 0; rep < repetitions; ++rep) {
        fprintf(stderr, "Vertical Beamform(%d)..", algo);
        std::vector<FanVBeamformer::ResVec> rv;
        for (unsigned i = 0; i < numFans; ++i) {
            rv.push_back(FanVBeamformer::ResVec(i,
                        &voutput[i * stride * numStaves], stride));
        }
        double start = getTime();
        numOutSamples = vformer->Run(&vinput[0], stride, numStaves, numSamples, &rv[0], rv.size());
        fprintf(stderr, ". Done (%f ms)\n", (getTime() - start) * 1000);
        fprintf(stderr, "Horizontal Beamform..");
        start = getTime();
        for (unsigned i = 0; i < numFans; ++i) {
            hformer->Run(&voutput[i * stride * numStaves], numOutSamples, &houtput[i * hformer->Length() * hformer->NumBeams()], hformer->Length());
        }
        fprintf(stderr, ". Done (%f ms)\n", (getTime() - start) * 1000);
        measure.Tick(numSamples);
    }
    fprintf(stderr, "Avg:\t%f Hz\nMax:\t%f Hz\nMin:\t%f Hz\n", measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());

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


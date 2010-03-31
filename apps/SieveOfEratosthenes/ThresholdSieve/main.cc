/** \file
 */

#include "Kernel.h"
#include "ThresholdSieveOptions.h"
#include "ThresholdSieveController.h"
#include "Time.h"
#include <sys/times.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

const char* const VALID_OPTS = "Mm:q:t:hf:i:p:vw:rz:s";
const char* const HELP_OPTS = "Usage: %s -hv -m maxprime -q queuesize -t threshold -f filename -p primes per filter -i iterations\n"
"\t-h\tPrint out this message\n"
"\t-v\tBe verbose, print out the primes found\n"
"\t-m\tSpecify the maximum number to consider for primes (default 100)\n"
"\t-q\tSpecify the queue size to use (default 100)\n"
"\t-t\tSpecify the threshold to use (default 2)\n"
"\t-f\tSpecify a file to use instead of stdout (clobbers)\n"
"\t-pa,b,c,...\tSpecify the number of primes per filter as a polynomial (default 1)\n"
"\t-w\tSpecify the number of primes in the producer prime wheel (default 0)\n"
"\t-i\tRerun the given number of times\n"
"\t-r\tReport per filter statistics\n"
"\n"
"Note that when the number of primes in the prime wheel is not zero the maximum\n"
"number to consider for primes is not exact.\n"
"Also, If the queue size is the same as the threshold size the sieve may deadlock.\n"
;

struct TestResults {
    double usertime;
    double systime;
    double realtime;
};

TestResults SieveTest(ThresholdSieveOptions options);

int main(int argc, char **argv) {
    std::vector<ThresholdSieveOptions::NumberT> results;
    std::vector<double> primesPerFilter;
    ThresholdSieveOptions options;
    options.maxprime = 100;
    options.queuesize = 100;
    options.threshold = 2;
    options.numPrimesSource = 0;
    options.queuehint = CPN::QUEUEHINT_THRESHOLD;
    options.results = &results;
    options.report = false;
    options.zerocopy = 0;
    std::string ppf = "1";
    int numIterations = 1;
    bool multitest = false;
    bool verbose = false;
    bool tofile = false;
    bool simpleoutput = false;
    std::string filename = "";
    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'w':
            options.numPrimesSource = atoi(optarg);
            if (options.numPrimesSource < 0) { options.numPrimesSource = 0; }
            if (options.numPrimesSource >= 8) { options.numPrimesSource = 8; }
            break;
        case 'm':
            options.maxprime = atoi(optarg);
            if (options.maxprime < 2) options.maxprime = 2;
            break;
        case 'M':
            multitest = true;
            break;
        case 'q':
            options.queuesize = atoi(optarg);
            if (options.queuesize < 1) options.queuesize = 1;
            break;
        case 't':
            options.threshold = atoi(optarg);
            if (options.threshold < 2) options.threshold = 2;
            break;
        case 'r':
            options.report = true;
            break;
        case 's':
            simpleoutput = true;
            break;
        case 'z':
            {
                options.zerocopy = atoi(optarg);
            }
            break;
        case 'p':
            {
                ppf = optarg;
                char *num = strtok(optarg, ", ");
                while (num != 0) {
                    primesPerFilter.push_back(atof(num));
                    num = strtok(0, ", ");
                }
            }
            break;
        case 'v':
            verbose = true;
            break;
        case 'f':
            filename = optarg;
            tofile = true;
            break;
        case 'i':
            numIterations = atoi(optarg);
            if (numIterations < 0) numIterations = 1;
            break;
        case 'h':
            printf(HELP_OPTS, argv[0]);
            return 0;
        case -1:
            procOpts = false;
            break;
        default:
            printf("Invalid option\n");
            printf(HELP_OPTS, argv[0]);
            return 0;
        }
    }
    options.primesPerFilter = primesPerFilter;
    const char STDOUT_FORMAT[] = "{\n    \"maxprime\"        : %lu,\n"
        "    \"queuesize\"       : %lu,\n"
        "    \"threshold\"       : %lu,\n"
        "    \"primewheel\"      : %lu,\n"
        "    \"ppf\"             : [%s],\n"
        "    \"zerocopy\"        : %d,\n"
        "    \"realtime\"        : %f,\n"
        "    \"usertime\"        : %f,\n"
        "    \"systime\"         : %f,\n"
        "    \"numprimes\"       : %u";
    const char SIMPLE_FORMAT[] = "%lu, %lu, %lu, %lu, [%s], %d, %f, %f, %f, %u";
    const char *format_str;
    FILE *f = stdout;
    if (simpleoutput) {
        format_str = SIMPLE_FORMAT;
    } else {
        format_str = STDOUT_FORMAT;
    }
    if (tofile) {
        f = fopen(filename.c_str(), "w");
        if (!f) f = stdout;
    }
    if (!simpleoutput) {
        fprintf(f, "[\n");
    }
    for (int i = 0; i < numIterations; ++i) {
        TestResults timeresults = SieveTest(options);
        fprintf(f, format_str,
                (unsigned long)options.maxprime,
                options.queuesize,
                options.threshold,
                options.numPrimesSource,
                ppf.c_str(),
                options.zerocopy,
                timeresults.realtime,
                timeresults.usertime,
                timeresults.systime,
                (unsigned)results.size());
        if (verbose) {
            if (!simpleoutput) {
                fprintf(f, ",\n    \"primes\"          : [");
            }
            for (size_t j = 0; j < results.size(); ++j) {
                if (j < results.size() -1) {
                    fprintf(f, "%lu, ", (unsigned long)results[j]);
                } else {
                    fprintf(f, "%lu", (unsigned long)results[j]);
                }
            }
            if (!simpleoutput) {
                fprintf(f, "]\n");
            } else {
                fprintf(f, "\n");
            }
        } else { fprintf(f, "\n"); }
        if (!simpleoutput) {
            if (i == numIterations - 1) {
                fprintf(f, "}\n");
            } else {
                fprintf(f, "},\n");
            }
        }
        results.clear();
    }
    if (!simpleoutput) {
        fprintf(f, "]\n");
    }
    if (f != stdout) {
        fclose(f);
    }
    return 0;
}


TestResults SieveTest(ThresholdSieveOptions options) {
    CPN::Kernel kernel(CPN::KernelAttr("Kernel name"));
    CPN::NodeAttr attr("controller", THRESHOLDSIEVECONTROLLER_TYPENAME);
    attr.SetParam(StaticBuffer(&options, sizeof(options)));
    tms tmsStart;
    tms tmsStop;
    times(&tmsStart);
    Time start;
    kernel.CreateNode(attr);
    kernel.WaitNodeTerminate("controller");
    Time stop;
    times(&tmsStop);
    TestResults result;
    TimeInterval rduration = start - stop;
    result.realtime = rduration.Seconds() + (double)(rduration.Microseconds())/1000000.0;
    result.usertime = (double)(tmsStop.tms_utime - tmsStart.tms_utime)/(double)sysconf(_SC_CLK_TCK);
    result.systime = (double)(tmsStop.tms_stime - tmsStart.tms_stime)/(double)sysconf(_SC_CLK_TCK);
    return result;
}



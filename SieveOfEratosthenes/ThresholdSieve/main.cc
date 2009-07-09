/** \file
 */

#include "Kernel.h"
#include "ThresholdQueue.h"
#include "ThresholdSieveOptions.h"
#include "ThresholdSieveController.h"
#include "Time.h"
#include <sys/times.h>
#include <unistd.h>
#include <cstdio>

const char* const VALID_OPTS = "Mm:q:t:hf:i:p:v";
const char* const HELP_OPTS = "Usage: %s -hv -m maxprime -q queuesize -t threshold -f filename -p primes per filter\n"
"\t-h\tPrint out this message\n"
"\t-v\tBe verbose, print out the primes found\n"
"\t-m\tSpecify the maximum number to consider for primes (default 100)\n"
"\t-q\tSpecify the queue size to use (default 100)\n"
"\t-t\tSpecify the threshold to use (default 2)\n"
"\t-f\tSpecify a file to use instead of stdout (appends)\n"
"\t-p\tSpecify the number of primes per filter (default 1)\n";

struct TestResults {
	double usertime;
	double systime;
	double realtime;
};

TestResults SieveTest(ThresholdSieveOptions options);

int main(int argc, char **argv) {
	CPN::ThresholdQueue::RegisterQueueType();
	ThresholdSieveController::RegisterNodeType();
	std::vector<ThresholdSieveOptions::NumberT> results;
	ThresholdSieveOptions options;
	options.maxprime = 100;
	options.queuesize = 100;
	options.threshold = 2;
	options.primesPerFilter = 1;
	options.queueTypeName = CPN_QUEUETYPE_THRESHOLD;
	options.results = &results;
	int numIterations = 1;
	bool multitest = false;
	bool verbose = false;
	bool tofile = false;
	std::string filename = "";
	bool procOpts = true;
	while (procOpts) {
		switch (getopt(argc, argv, VALID_OPTS)) {
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
		case 'p':
			options.primesPerFilter = atoi(optarg);
			if (options.primesPerFilter < 1) options.primesPerFilter = 1;
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
	FILE *f = stdout;
	if (tofile) {
		f = fopen(filename.c_str(), "a");
		if (!f) f = stdout;
	}
	if (multitest) {
		for (int m = 1000000; m <= 10000000; m += 1000000) {
			options.maxprime = m;
			for (int t = 2; t < m/10; t *=2) {
				options.queuesize = 2*t;
				options.threshold = t;
				for (int i = 0; i < numIterations; ++i) {
					TestResults timeresults = SieveTest(options);
					fprintf(f, "%lu\t%lu\t%lu\t%f\t%f\t%f\n",
							(unsigned long)options.maxprime,
							options.queuesize,
							options.threshold,
							timeresults.realtime,
							timeresults.usertime,
							timeresults.systime);
					results.clear();
				}
			}
		}
	} else {
		for (int i = 0; i < numIterations; ++i) {
			TestResults timeresults = SieveTest(options);
			fprintf(f, "%lu\t%lu\t%lu\t%lu\t%f\t%f\t%f\n",
					(unsigned long)options.maxprime,
					options.queuesize,
					options.threshold,
					options.primesPerFilter,
					timeresults.realtime,
					timeresults.usertime,
					timeresults.systime);
			if (verbose) {
				fprintf(f, "%u primes found\n", (unsigned)results.size());
				for (size_t j = 0; j < results.size(); ++j) {
					fprintf(f, "%lu, ", (unsigned long)results[j]);
				}
				fprintf(f, "\n");
			}
			results.clear();
		}
	}
	if (f != stdout) {
		fclose(f);
	}
	return 0;
}


TestResults SieveTest(ThresholdSieveOptions options) {
	CPN::Kernel kernel(CPN::KernelAttr(1, "Kernel name"));
	kernel.CreateNode("controller", THRESHOLDSIEVECONTROLLER_TYPENAME, &options, 0);
	tms tmsStart;
	tms tmsStop;
	times(&tmsStart);
	Time start;
	kernel.Start();
	kernel.Wait();
	Time stop;
	times(&tmsStop);
	TestResults result;
	TimeInterval rduration = start - stop;
	result.realtime = rduration.Seconds() + (double)(rduration.Microseconds())/1000000.0;
	result.usertime = (double)(tmsStop.tms_utime - tmsStart.tms_utime)/(double)sysconf(_SC_CLK_TCK);
	result.systime = (double)(tmsStop.tms_stime - tmsStart.tms_stime)/(double)sysconf(_SC_CLK_TCK);
	return result;
}



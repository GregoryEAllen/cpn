/** \file
 */

#include "Kernel.h"
#include "ThresholdQueue.h"
#include "ThresholdSieveOptions.h"
#include "ThresholdSieveController.h"
#include "Time.h"
#include <unistd.h>
#include <cstdio>

const char* const VALID_OPTS = "m:q:t:hf:";
const char* const HELP_OPTS = "Usage: %s -h -m maxprime -q queuesize -t threshold -f filename\n";

int main(int argc, char **argv) {
	CPN::ThresholdQueue::RegisterQueueType();
	ThresholdSieveController::RegisterNodeType();
	CPN::Kernel kernel(CPN::KernelAttr(1, "Kernel name"));
	std::vector<unsigned long> results;
	ThresholdSieveOptions options;
	options.maxprime = 100;
	options.queuesize = 100;
	options.threshold = 2;
	options.queueTypeName = CPN_QUEUETYPE_THRESHOLD;
	options.results = &results;
	bool tofile = false;
	std::string filename = "";
	bool procOpts = true;
	while (procOpts) {
		switch (getopt(argc, argv, VALID_OPTS)) {
		case 'm':
			options.maxprime = atoi(optarg);
			if (options.maxprime < 2) options.maxprime = 2;
			break;
		case 'q':
			options.queuesize = atoi(optarg);
			if (options.queuesize < 1) options.queuesize = 1;
			break;
		case 't':
			options.threshold = atoi(optarg);
			if (options.threshold < 2) options.threshold = 2;
			break;
		case 'f':
			filename = optarg;
			tofile = true;
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

	kernel.CreateNode("controller", THRESHOLDSIEVECONTROLLER_TYPENAME, &options, 0);
	Time start;
	kernel.Start();
	kernel.Wait();
	Time stop;
	printf("Duration: %s\n", (start - stop).ToString().c_str());
	FILE *f = stdout;
	if (tofile) {
		f = fopen(filename.c_str(), "w");
		if (!f) f = stdout;
	}
	for (int i = 0; i < results.size(); i++) {
		fprintf(f, "%lu\n", results[i]);
	}
	if (f != stdout) {
		fclose(f);
	}
	return 0;
}


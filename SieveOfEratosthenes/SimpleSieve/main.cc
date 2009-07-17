

#include "Kernel.h"
#include "SieveControllerNode.h"
#include "SimpleQueue.h"
#include "ThresholdQueue.h"
#include "Time.h"
#include <unistd.h>
#include <vector>
#include <cstdio>
#include <cstdlib>


const char* const VALID_OPS = "m:q:";

int main(int argc, char **argv) {
	int maxprime = 100;
	int queueSize = 100;
	int threshold = 1;
	bool procOpts = true;
	while (procOpts) {
		int opt = getopt(argc, argv, VALID_OPS);
		switch (opt) {
		case 'm':
			maxprime = atoi(optarg);
			if (maxprime < 2) maxprime = 2;
			break;
		case 'q':
			queueSize = atoi(optarg);
			if (queueSize < 1) queueSize = 1;
			break;
		case -1:
			procOpts = false;
			break;
		default:
			printf("Usage: %s -m maxprime -q queuesize\n", argv[0]);
			return 0;
		}
	}
	CPN::SimpleQueue::RegisterQueueType();
	CPN::ThresholdQueue::RegisterQueueType();
	SieveControllerNode::RegisterNodeType();
	CPN::Kernel kernel(CPN::KernelAttr(1, "SimpleSieveKernel"));
	std::vector<unsigned long> results;
	SieveControllerNode::Param param;
	param.results = &results;
	param.primeBound = maxprime;
	param.numberBound = maxprime;
	param.queueSize = queueSize;
	if (threshold == 1) {
		param.queueTypeName = CPN_QUEUETYPE_SIMPLE;
	} else {
		param.queueTypeName = CPN_QUEUETYPE_THRESHOLD;
	}
	param.threshold = threshold;
	kernel.CreateNode("controller", SIEVECONTROLLERNODE_TYPENAME, &param, sizeof(param));
	Time start;
	kernel.Start();
	kernel.Wait();
	Time stop;
	printf("Duration: %s\n", (start - stop).ToString().c_str());
	for (int i = 0; i < results.size(); i++) {
		printf(" %lu,", results[i]);
	}
	printf("\n");
	return 0;
}



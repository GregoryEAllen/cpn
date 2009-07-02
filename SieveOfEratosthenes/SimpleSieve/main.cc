

#include "Kernel.h"
#include "SieveControllerNode.h"
#include "SimpleQueue.h"
#include <vector>
#include <cstdio>
#include <cstdlib>



int main(int argc, char **argv) {
	int maxprime = 100;
	int queueSize = 100;
	if (argc > 1) {
		maxprime = atoi(argv[1]);
	}
	if (argc > 2) {
		queueSize = atoi(argv[2]);
	}
	CPN::SimpleQueue::RegisterQueueType();
	SieveControllerNode::RegisterNodeType();
	CPN::Kernel kernel(CPN::KernelAttr(1, "SimpleSieveKernel"));
	std::vector<unsigned long> results;
	SieveControllerNode::Param param;
	param.results = &results;
	param.primeBound = maxprime;
	param.numberBound = maxprime;
	param.queueSize = queueSize;
	param.queueTypeName = CPN_QUEUETYPE_SIMPLE;
	kernel.CreateNode("controller", SIEVECONTROLLERNODE_TYPENAME, &param, 0);
	kernel.Start();
	kernel.Wait();
	for (int i = 0; i < results.size(); i++) {
		printf(" %lu,", results[i]);
	}
	printf("\n");
	return 0;
}



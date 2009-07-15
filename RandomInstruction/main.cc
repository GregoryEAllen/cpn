

#include "RandomInstructionNode.h"
#include "Kernel.h"
#include "SimpleQueue.h"
#include <unistd.h>


int main(int argc, char **argv) {
	CPN::SimpleQueue::RegisterQueueType();
	RandomInstructionNode::RegisterNodeType();

	unsigned iterations = 10;
	unsigned numNodes = 10;
	int debugLevel = 0;

	bool procOpts = true;
	while (procOpts) {
		switch (getopt(argc, argv, "i:d:n:")) {
		case 'i':
			iterations = atoi(optarg);
			break;
		case 'd':
			debugLevel = atoi(optarg);
			break;
		case 'n':
			numNodes = atoi(optarg);
			break;
		case -1:
			procOpts = false;
			break;
		default:
			printf("Invald option %s\n", optarg);
		}
	}

	CPN::Kernel kernel(CPN::KernelAttr(1, "test kernel"));
	RandomInstructionNode::CreateRIN(kernel, iterations, numNodes, debugLevel);
	kernel.Start();
	kernel.Wait();
	return 0;
}



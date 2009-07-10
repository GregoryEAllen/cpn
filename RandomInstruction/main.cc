

#include "RandomInstructionNode.h"
#include "Kernel.h"
#include "ThresholdQueue.h"
#include <unistd.h>

void GetOptions(int argc, char **argv, RINState &state);

int main(int argc, char **argv) {
	CPN::ThresholdQueue::RegisterQueueType();
	RandomInstructionNode::RegisterNodeType();
	RINState state;
	state.iterations = 10000;
	state.debugLevel = 1;
	GetOptions(argc, argv, state);
	state.state.numNodes = 1;
	CPN::Kernel kernel(CPN::KernelAttr(1, "test kernel"));
	RandomInstructionNode::CreateNode(kernel, state);
	kernel.Start();
	kernel.Wait();
	return 0;
}


void GetOptions(int argc, char **argv, RINState &state) {
	bool procOpts = true;
	while (procOpts) {
		switch (getopt(argc, argv, "i:d:")) {
		case 'i':
			state.iterations = atoi(optarg);
			break;
		case 'd':
			state.debugLevel = atoi(optarg);
			break;
		case -1:
			procOpts = false;
			break;
		default:
			printf("Invald option %s\n", optarg);
		}
	}
}

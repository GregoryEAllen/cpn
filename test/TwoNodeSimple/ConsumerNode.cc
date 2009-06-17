
#include "ConsumerNode.h"
#include "QueueReaderAdaptor.h"

const ::std::string ConsumerNode::inputs[] = { "in" };
const ::std::string ConsumerNode::outputs[] = {};

void ConsumerNode::Process(void) {
	CPN::QueueReaderAdaptor<unsigned long> in = GetReader("in");
	unsigned long i = 0;
	while (1) {
		in.Dequeue(&i, 1);
		printf("ConsumerNode: dequeued(%u)\n", i);
	}
}


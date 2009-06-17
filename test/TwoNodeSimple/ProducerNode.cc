

#include "ProducerNode.h"
#include "QueueWriterAdaptor.h"

const ::std::string ProducerNode::inputs[] = {};
const ::std::string ProducerNode::outputs[] = { "out" };

void ProducerNode::Process(void) {
	CPN::QueueWriterAdaptor<unsigned long> out = GetWriter("out");
	unsigned long i = 0;
	while (1) {
		printf("ProducerNode: enqueued(%u)\n", i);
		out.Enqueue(&i, 1);
		i++;
	}
}


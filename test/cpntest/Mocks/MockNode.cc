
#include "MockNode.h"
#include "MockNodeFactory.h"
#include "NodeAttr.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "Kernel.h"
#include <cstdio>
#include <string>

static MockNodeFactory theFactory("MockNode");

void MockNode::Process(void) {
	std::string ourname = GetAttr().GetName();
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(ourname, "y");
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(ourname, "x");
	unsigned long counter = 0;
	unsigned long threshold = 10;
	switch (mode) {
		case MODE_SOURCE:
			while (counter < threshold) {
				out.Enqueue(&counter, 1);
				++counter;
			}
			break;
		case MODE_TRANSMUTE:
			while (counter < threshold) {
				unsigned long value = 0;
				in.Dequeue(&value, 1);
				value += counter;
				out.Enqueue(&value, 1);
				++counter;
			}
			break;
		case MODE_SINK:
			while (counter < threshold) {
				unsigned long value = 0;
				in.Dequeue(&value, 1);
				printf("Sink %s got %lu\n", ourname.c_str(), value);
				++counter;
			}
			break;
		default:
			break;
	}
}


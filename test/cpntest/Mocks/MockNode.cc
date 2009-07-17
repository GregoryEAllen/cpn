
#include "MockNode.h"
#include "MockNodeFactory.h"
#include "NodeAttr.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include "Kernel.h"
#include <cstdio>
#include <string>

#if _DEBUG
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

void MockNode::Process(void) {
	std::string ourname = GetAttr().GetName();
	DEBUG("%s started\n", ourname.c_str());
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(ourname, "y");
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(ourname, "x");
	unsigned long counter = 0;
	unsigned long threshold = 10;
	switch (mode) {
		case MODE_SOURCE:
		    	DEBUG("%s acting as producer\n", ourname.c_str());
			while (counter < threshold) {
				out.Enqueue(&counter, 1);
				DEBUG("Source %s sent %lu\n", ourname.c_str(), counter);
				++counter;
			}
			break;
		case MODE_TRANSMUTE:
		    	DEBUG("%s acting as transmuter\n", ourname.c_str());
			while (counter < threshold) {
				unsigned long value = 0;
				in.Dequeue(&value, 1);
				DEBUG("Transmuter %s got %lu\n", ourname.c_str(), value);
				value += counter;
				out.Enqueue(&value, 1);
				DEBUG("Transmuter %s sent %lu\n", ourname.c_str(), value);
				++counter;
			}
			break;
		case MODE_SINK:
		    	DEBUG("%s acting as sink\n", ourname.c_str());
			while (counter < threshold) {
				unsigned long value = 0;
				in.Dequeue(&value, 1);
				DEBUG("Sink %s got %lu\n", ourname.c_str(), value);
				++counter;
			}
			break;
		case MODE_NOP:
		    	DEBUG("%s acting as nop\n", ourname.c_str());
			// Do nothing!
			break;
		default:
			break;
	}
	DEBUG("%s stopped\n", ourname.c_str());
}


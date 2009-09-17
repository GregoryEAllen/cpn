
#include "MockNode.h"
#include "QueueWriterAdapter.h"
#include "QueueReaderAdapter.h"
#include <string>

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

MockNode::MockNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
    : CPN::NodeBase(ker, attr)
{
    mode = GetMode(attr.GetParam());
}

std::string MockNode::GetModeName(Mode_t mode) {
    switch (mode) {
    case MODE_SOURCE:
        return "source";
    case MODE_TRANSMUTE:
        return "transmute";
    case MODE_SINK:
        return "sink";
    default:
        return "nop";
    }
}

MockNode::Mode_t MockNode::GetMode(const std::string &param) {
    if (param == "source") return MODE_SOURCE;
    else if (param == "transmute") return MODE_TRANSMUTE;
    else if (param == "sink") return MODE_SINK;
    else return MODE_NOP;
}
void MockNode::Process() {
	std::string ourname = GetName();
	DEBUG("%s started\n", ourname.c_str());
	unsigned long counter = 0;
	unsigned long threshold = 10;
	switch (mode) {
		case MODE_SOURCE:
            {
                CPN::QueueWriterAdapter<unsigned long> out = GetWriter("y");
                DEBUG("%s acting as producer\n", ourname.c_str());
                while (counter < threshold) {
                    out.Enqueue(&counter, 1);
                    DEBUG("Source %s sent %lu\n", ourname.c_str(), counter);
                    ++counter;
                }
            }
			break;
		case MODE_TRANSMUTE:
            {
                CPN::QueueWriterAdapter<unsigned long> out = GetWriter("y");
                CPN::QueueReaderAdapter<unsigned long> in = GetReader("x");
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
            }
			break;
		case MODE_SINK:
            {
                CPN::QueueReaderAdapter<unsigned long> in = GetReader("x");
		    	DEBUG("%s acting as sink\n", ourname.c_str());
                while (counter < threshold) {
                    unsigned long value = 0;
                    in.Dequeue(&value, 1);
                    DEBUG("Sink %s got %lu\n", ourname.c_str(), value);
                    ++counter;
                }
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


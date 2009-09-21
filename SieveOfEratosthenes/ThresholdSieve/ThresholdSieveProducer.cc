/** \file
 */

#include "ThresholdSieveProducer.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "Assert.h"
#include <stdexcept>


#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

using CPN::shared_ptr;

typedef ThresholdSieveOptions::NumberT NumberT;

class ProducerFactory : public CPN::NodeFactory {
public:
	ProducerFactory() : CPN::NodeFactory(THRESHOLDSIEVEPRODUCER_TYPENAME) {}
	shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
        ASSERT(attr.GetArg().GetSize() == sizeof(ThresholdSieveOptions));
		ThresholdSieveOptions *opts = (ThresholdSieveOptions*)attr.GetArg().GetBuffer();
		return shared_ptr<CPN::NodeBase>(new ThresholdSieveProducer(ker, attr, *opts));
	}
};


void ThresholdSieveProducer::Process(void) {
	DEBUG("%s started\n", GetName().c_str());
	CPN::QueueWriterAdapter<NumberT> out = GetWriter(OUT_PORT);
	const NumberT cutoff = opts.maxprime;
	const unsigned long threshold = opts.threshold;
	NumberT counter = 2;
	while (counter < cutoff) {
		NumberT index = 1;
		NumberT *outbuff = out.GetEnqueuePtr(threshold);
		while (index < threshold && counter < cutoff) {
			outbuff[index] = counter;
			++index;
			++counter;
		}
		outbuff[0] = index - 1;
		out.Enqueue(index);
	}
	counter = 0;
	out.Enqueue(&counter, 1);
    out.Release();
	DEBUG("%s stopped\n", GetName().c_str());
}


void ThresholdSieveProducer::RegisterNodeType(void) {
	CPNRegisterNodeFactory(shared_ptr<CPN::NodeFactory>(new ProducerFactory));
}


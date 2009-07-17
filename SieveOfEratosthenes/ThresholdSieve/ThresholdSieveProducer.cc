/** \file
 */

#include "ThresholdSieveProducer.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include <stdexcept>


#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

typedef ThresholdSieveOptions::NumberT NumberT;

class ProducerFactory : public CPN::NodeFactory {
public:
	ProducerFactory() : CPN::NodeFactory(THRESHOLDSIEVEPRODUCER_TYPENAME) {}
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
			const void* const arg, const unsigned long argsize) {
		ThresholdSieveOptions *opts = (ThresholdSieveOptions*)arg;
		return new ThresholdSieveProducer(ker, attr, *opts);
	}
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
		throw std::invalid_argument("ThresholdSieveProducer requires a ThresholdSieveOptions parameter.");
	}
	void Destroy(CPN::NodeBase *node) {
		delete node;
	}
};

static ProducerFactory factoryInstance;

void ThresholdSieveProducer::Process(void) {
	DEBUG("%s started\n", GetName().c_str());
	CPN::QueueWriterAdapter<NumberT> out = kernel.GetWriter(GetName(), OUT_PORT);
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
	DEBUG("%s stopped\n", GetName().c_str());
}


void ThresholdSieveProducer::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&factoryInstance);
}


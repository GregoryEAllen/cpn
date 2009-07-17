/** \file
 */

#include "ThresholdSieveController.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "ThresholdSieveProducer.h"
#include "ThresholdSieveFilter.h"
#include "ToString.h"
#include <stdexcept>

#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

typedef ThresholdSieveOptions::NumberT NumberT;

class ControllerFactory : public CPN::NodeFactory {
public:
	ControllerFactory() : CPN::NodeFactory(THRESHOLDSIEVECONTROLLER_TYPENAME) {}
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
			const void* const arg, const unsigned long argsize) {
		ThresholdSieveOptions *opts = (ThresholdSieveOptions*)arg;
		return new ThresholdSieveController(ker, attr, *opts);
	}
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) {
		throw std::invalid_argument("ThresholdSieveController requires a ThresholdSieveOptions parameter.");
	}
	void Destroy(CPN::NodeBase *node) {
		delete node;
	}
};

static ControllerFactory factoryInstance;

void ThresholdSieveController::Process(void) {
	DEBUG("%s started\n", GetName().c_str());
	CPN::QueueReaderAdapter<NumberT> in = kernel.GetReader(GetName(), IN_PORT);
	Initialize();
	std::vector<NumberT> *results = opts.results;
	NumberT inCount = 0;
	do {
		in.Dequeue(&inCount, 1);
		DEBUG("Consumer Reading %lu values\n", inCount);
		const NumberT *inBuff = in.GetDequeuePtr(inCount);
		results->insert(results->end(), inBuff, inBuff + inCount);
		in.Dequeue(inCount);
	} while (inCount != 0);
	DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveController::Initialize(void) {
	ThresholdSieveProducer::RegisterNodeType();
	ThresholdSieveFilter::RegisterNodeType();
	std::string nodename = ToString(FILTER_FORMAT, 1);
	kernel.CreateNode(nodename, THRESHOLDSIEVEFILTER_TYPENAME, &opts, 0);
	kernel.CreateQueue(CONSUMERQ_NAME, opts.queueTypeName,
		       opts.queuesize * sizeof(NumberT),
		       opts.threshold * sizeof(NumberT), 1);
	kernel.ConnectReadEndpoint(CONSUMERQ_NAME, GetName(), IN_PORT);
	kernel.ConnectWriteEndpoint(CONSUMERQ_NAME, nodename, OUT_PORT);
	kernel.CreateNode(PRODUCER_NAME, THRESHOLDSIEVEPRODUCER_TYPENAME, &opts, 0);
	std::string queuename = ToString(QUEUE_FORMAT, 1);
	kernel.CreateQueue(queuename, opts.queueTypeName,
		       opts.queuesize * sizeof(NumberT),
		       opts.threshold * sizeof(NumberT), 1);
	kernel.ConnectReadEndpoint(queuename, nodename, IN_PORT);
	kernel.ConnectWriteEndpoint(queuename, PRODUCER_NAME, OUT_PORT);
}

void ThresholdSieveController::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&factoryInstance);
}


/** \file
 */

#include "ThresholdSieveFilter.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "ToString.h"
#include <cmath>


#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

class FilterFactory : public CPN::NodeFactory {
public:
	FilterFactory() : CPN::NodeFactory(THRESHOLDSIEVEFILTER_TYPENAME) {}
	CPN::NodeBase* Create(CPN::Kernel &ker, const CPN::NodeAttr &attr,
			const void* const arg, const unsigned long argsize) {
		ThresholdSieveOptions *opts = (ThresholdSieveOptions*)arg;
		return new ThresholdSieveFilter(ker, attr, *opts);
	}
	void Destroy(CPN::NodeBase *node) {
		delete node;
	}
};

static FilterFactory factoryInstance;

void ThresholdSieveFilter::Process(void) {
	DEBUG("%s started\n", GetName().c_str());
	CPN::QueueReaderAdapter<unsigned long> in = kernel.GetReader(GetName(), IN_PORT);
	CPN::QueueWriterAdapter<unsigned long> out = kernel.GetWriter(GetName(), OUT_PORT);
	const unsigned long threshold = opts.threshold;
	unsigned long inCount = 0;
	unsigned long inIdx = 0;
	unsigned long outIdx = 0;
	unsigned long filterval = 0;
	unsigned long filternext = 0;
	const unsigned long *inBuff = 0;
	unsigned long *outBuff = 0;
	inIdx = 0;
	in.Dequeue(&inCount, 1);
	inBuff = in.GetDequeuePtr(inCount);
	DEBUG("%s Dequeued %lu elements first is %lu\n", GetName().c_str(), inCount, inBuff[0]);
	if (inCount > 0) {
		filterval = inBuff[inIdx];
		inIdx++;
		filternext = filterval + filterval;
		outIdx = 1;
		out.Enqueue(&outIdx, 1);
		out.Enqueue(&filterval, 1);
		DEBUG("%s Enqueued %lu elements first is %lu\n", GetName().c_str(), outIdx, filterval);
		if (ceil(sqrt(opts.maxprime)) >= filterval) {
			CreateNewFilter(filterval);
		}
	}
	do {
		if (outBuff == 0) {
			outIdx = 1;
			outBuff = out.GetEnqueuePtr(threshold);
		}
		if (inBuff == 0) {
			inIdx = 0;
			in.Dequeue(&inCount, 1);
			inBuff = in.GetDequeuePtr(inCount);
			DEBUG("%s Dequeued %lu elements first is %lu\n", GetName().c_str(), inCount, inBuff[0]);
		}
		while (inIdx < inCount && outIdx < threshold) {
			if (inBuff[inIdx] == filternext) {
				filternext += filterval;
			} else  {
				outBuff[outIdx] = inBuff[inIdx];
				++outIdx;
				if (inBuff[inIdx] > filternext) {
					filternext += filterval;
				}
			}
			++inIdx;
		}
		if (inIdx >= inCount) {
			in.Dequeue(inCount);
			inBuff = 0;
		}
		if (outIdx >= threshold || inCount == 0) {
			outBuff[0] = outIdx - 1;
			DEBUG("%s Enqueued %lu elements first is %lu\n", GetName().c_str(), outBuff[0], outBuff[1]);
			out.Enqueue(outIdx);
			outBuff = 0;
		}
	} while (inCount != 0);
	unsigned long endMarker = 0;
	out.Enqueue(&endMarker, 1);
	DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveFilter::CreateNewFilter(unsigned long lastprime) {
	std::string nodename = ToString(FILTER_FORMAT, lastprime);
	kernel.CreateNode(nodename, THRESHOLDSIEVEFILTER_TYPENAME, &opts, 0);
	std::string queuename = ToString(QUEUE_FORMAT, lastprime);
	kernel.CreateQueue(queuename, opts.queueTypeName,
		      opts.queuesize * sizeof(unsigned long),
		      opts.threshold * sizeof(unsigned long), 1);
	kernel.ConnectWriteEndpoint(queuename, GetName(), OUT_PORT);
	kernel.ConnectReadEndpoint(queuename, nodename, IN_PORT);
	kernel.ConnectWriteEndpoint(CONSUMERQ_NAME, nodename, OUT_PORT);
}

void ThresholdSieveFilter::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&factoryInstance);
}


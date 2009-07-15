/** \file
 */

#include "ThresholdSieveFilter.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "ToString.h"
#include "PrimeSieve.h"
#include <cmath>
#include <cassert>


#if _DEBUG
#include <cstdio>
#define DEBUG(frmt, ...) printf(frmt, __VA_ARGS__)
#else
#define DEBUG(frmt, ...)
#endif

typedef ThresholdSieveOptions::NumberT NumberT;

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
	CPN::QueueReaderAdapter<NumberT> in = kernel.GetReader(GetName(), IN_PORT);
	CPN::QueueWriterAdapter<NumberT> out = kernel.GetWriter(GetName(), OUT_PORT);
	const unsigned long threshold = opts.threshold;
	const NumberT cutoff = (NumberT)(ceil(sqrt(opts.maxprime)));
	PrimeSieve sieve(opts.primesPerFilter);
	const NumberT* inbuff = 0;
	NumberT incount = 0;
	unsigned long inidx = 0;
	NumberT *outbuff = 0;
	unsigned long outidx = 0;
	bool createFilter = false;
	bool filterCreated = false;
	do {
		if (0 == inbuff) {
			in.Dequeue(&incount, 1);
			inbuff = in.GetDequeuePtr(incount);
			inidx = 0;
//DEBUG("%s read %lu values: %lu\n", GetName().c_str(), incount, inbuff[0]);
		}
		if (0 == outbuff) {
			outbuff = out.GetEnqueuePtr(threshold);
			outidx = 1;
		}
		while ( (inidx < incount) &&
			(outidx < threshold) &&
		       	(0 != incount) &&
		        (createFilter == false) ) {

			NumberT inval = inbuff[inidx];
			switch (sieve.TryCandidate(inval)) {
			case -1:
//	-1 if pr passed the sieve, but the sieve is full (still possible prime)
				if (filterCreated == false && inval < cutoff) {
					createFilter = true;
				} else {
					outbuff[outidx] = inval;
					++outidx;
					++inidx;
				}
				break;
			case 0:
//	0 if pr was stopped by the sieve
				++inidx;
				break;
			case 1:
//	1 if pr is prime, and has been added to the sieve
				outbuff[outidx] = inval;
				++outidx;
				++inidx;
				break;
			default:
				assert(false);
			}
		}
		if (inidx >= incount) {
			in.Dequeue(incount);
			inbuff = 0;
		}
		if (outidx >= threshold || 0 == incount || createFilter) {
			outbuff[0] = outidx - 1;
			if (outidx > 1 || !createFilter) {
//DEBUG("%s wrote %lu values: %lu\n", GetName().c_str(), outidx -1, outbuff[1]);
				out.Enqueue(outidx);
			} else {
				out.Enqueue(0);
			}
			outbuff = 0;
			if (createFilter) {
				createFilter = false;
				filterCreated = true;
				CreateNewFilter(inbuff[inidx]);
			}
		}
	} while (0 != incount);
	NumberT endMarker = 0;
	out.Enqueue(&endMarker, 1);
	DEBUG("%s stopped\n", GetName().c_str());
}

void ThresholdSieveFilter::CreateNewFilter(NumberT lastprime) {
	std::string nodename = ToString(FILTER_FORMAT, lastprime);
	kernel.CreateNode(nodename, THRESHOLDSIEVEFILTER_TYPENAME, &opts, 0);
	std::string queuename = ToString(QUEUE_FORMAT, lastprime);
	kernel.CreateQueue(queuename, opts.queueTypeName,
		      opts.queuesize * sizeof(NumberT),
		      opts.threshold * sizeof(NumberT), 1);
	kernel.ConnectReadEndpoint(queuename, nodename, IN_PORT);
	kernel.ConnectWriteEndpoint(CONSUMERQ_NAME, nodename, OUT_PORT);
	kernel.ConnectWriteEndpoint(queuename, GetName(), OUT_PORT);
}

void ThresholdSieveFilter::RegisterNodeType(void) {
	CPNRegisterNodeFactory(&factoryInstance);
}


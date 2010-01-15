/** \file
 */

#include "ThresholdSieveProducer.h"
#include "NodeFactory.h"
#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "Assert.h"
#include "PrimeSieveSource.h"
#include <stdexcept>
#include <sstream>


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

void PrintArray(std::ostream &os, NumberT *arr, unsigned len, const char *sep) {
    for (unsigned i = 0; i < len; ++i) {
        os << arr[i];
        if (i < len -1) {
            os << sep;
        }
    }
}

void ThresholdSieveProducer::Process(void) {
    DEBUG("%s started\n", GetName().c_str());
    CPN::QueueWriterAdapter<NumberT> out = GetWriter(CONTROL_PORT);
    const NumberT cutoff = opts.maxprime;
    const unsigned long threshold = opts.threshold;
    if (opts.numPrimesSource > 0) {
        DEBUG("%s using PrimeSieveSource\n", GetName().c_str());
        NumberT primes[opts.numPrimesSource];
        PrimeSieveSource source(opts.numPrimesSource, primes);

#if _DEBUG
        {
            std::ostringstream oss;
            oss << GetName() << " primes: ";
            PrintArray(oss, primes, opts.numPrimesSource, ", ");
            puts(oss.str().c_str());
        }
#endif

        out.Enqueue(primes, opts.numPrimesSource);
        out.Release();
        CreateNewFilter(kernel, opts, GetKey());
        out = GetWriter(OUT_PORT);
        unsigned roundLength = source.RoundLength();
        bool loop = true;
        // TODO change to only top implementation when auto increasing of threshold
        // is implemented!
        // have enough threshold to send it all...
        if (roundLength <= threshold) {
            while (loop) {
                NumberT *outbuff = out.GetEnqueuePtr(roundLength);
                unsigned len = source.GetNextRound(outbuff);
                if (outbuff[len-1] > cutoff) {
                    loop = false;
                }
#if _DEBUG
        {
            std::ostringstream oss;
            oss << GetName() << " candidates: ";
            PrintArray(oss, outbuff, len, ", ");
            puts(oss.str().c_str());
        }
#endif
                out.Enqueue(len);
            }
        } else {
            while (loop) {
                std::vector<NumberT> buff;
                buff.resize(roundLength, 0);
                unsigned len = source.GetNextRound(&buff[0]);
                if (buff[len - 1] > cutoff) {
                    loop = false;
                }
#if _DEBUG
        {
            std::ostringstream oss;
            oss << GetName() << " candidates: ";
            PrintArray(oss, &buff[0], len, ", ");
            puts(oss.str().c_str());
        }
#endif
                out.Enqueue(&buff[0], len);
            }
        }
    } else {
        DEBUG("%s using simple loop\n", GetName().c_str());
        out.Release();
        CreateNewFilter(kernel, opts, GetKey());
        out = GetWriter(OUT_PORT);
        NumberT counter = 2;
        while (counter < cutoff) {
            NumberT index = 0;
            NumberT *outbuff = out.GetEnqueuePtr(threshold);
            while (index < threshold && counter < cutoff) {
                outbuff[index] = counter;
                ++index;
                ++counter;
            }
            out.Enqueue(index);
        }
    }

    out.Release();
    DEBUG("%s stopped\n", GetName().c_str());
}


void ThresholdSieveProducer::RegisterNodeType(void) {
    CPNRegisterNodeFactory(shared_ptr<CPN::NodeFactory>(new ProducerFactory));
}


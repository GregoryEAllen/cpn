
#pragma once

#include "KernelBase.h"
#include "MockContext.h"

class MockKernel : public CPN::KernelBase {
public:
    MockKernel() : useD4R(true), context(new MockContext()) {}
    MockKernel(CPN::shared_ptr<CPN::Context> ctx) : useD4R(true), context(ctx) {}

    bool IsTerminated() {
        return false;
    }
    void CheckTerminated() {}
    CPN::shared_ptr<CPN::Context> GetContext() const { return context; }

    bool UseD4R(bool enable) {
        useD4R = enable;
        return useD4R;
    }
    bool UseD4R() { return useD4R; }
    bool GrowQueueMaxThreshold() { return  true; }
    bool SwallowBrokenQueueExceptions() { return false; }
    unsigned CalculateGrowSize(unsigned currentsize, unsigned request) { return currentsize + request; }
    bool useD4R;
    CPN::shared_ptr<CPN::Context> context;
};

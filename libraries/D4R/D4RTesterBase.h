
#pragma once

#include "Variant.h"

namespace D4R {

    class TestNodeBase;

    class TesterBase {
    public:
        TesterBase();
        virtual ~TesterBase();

        void Setup(const Variant &v);

        virtual void Deadlock(TestNodeBase *tnb) = 0;
        virtual void Failure(TestNodeBase *tnb, const std::string &msg);
        virtual void Complete(TestNodeBase *tnb) = 0;

    protected:
        virtual void CreateNode(const Variant &noded) = 0;
        virtual void CreateQueue(const Variant &queued) = 0;
    };
}

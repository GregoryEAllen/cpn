
#pragma once

#include "Variant.h"
#include "Logger.h"

namespace D4R {

    class TestNodeBase;

    class TesterBase : public Logger {
    public:
        TesterBase();
        virtual ~TesterBase();

        void Setup(const Variant &v);

        virtual void Deadlock(TestNodeBase *tnb) = 0;
        virtual void Failure(TestNodeBase *tnb, const std::string &msg);
        virtual void Complete(TestNodeBase *tnb) = 0;
        virtual void Abort() = 0;

        bool Success() const { return success; }
    protected:
        virtual void CreateNode(const Variant &noded) = 0;
        virtual void CreateQueue(const Variant &queued) = 0;

        bool success;
    };
}

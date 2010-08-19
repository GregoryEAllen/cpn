
#pragma once
#include "RunnableFuture.h"

namespace Sync {

    template<typename Arg, typename Return>
    class Callable : public RunnableFuture<Return> {
    public:
        Callable(const Arg &a) : arg(a) {}
        virtual ~Callable() {}

        void Run() {
            if (!RunnableFuture<Return>::IsCanceled()) {
                RunnableFuture<Return>::Set(Call(arg));
            }
        }

    protected:
        virtual Return Call(Arg) = 0;
    private:
        Arg arg;
    };

    template<typename Arg>
    class Callable<Arg, void> : public RunnableFuture<void> {
    public:
        Callable(const Arg &a) : arg(a) {}
        virtual ~Callable() {}

        void Run() {
            if (!RunnableFuture<void>::IsCanceled()) {
                Call(arg);
                RunnableFuture<void>::Set();
            }
        }

    protected:
        virtual void Call(Arg) = 0;
    private:
        Arg arg;
    };
}

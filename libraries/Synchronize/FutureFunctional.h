
#pragma once
#include "Callable.h"
#include <memory>

namespace Sync {

    template<typename Return, typename Arg>
    class FutureFunctor : public Callable<Arg, Return> {
    public:
        explicit FutureFunctor(Return (*f)(Arg), const Arg &arg) : Callable<Arg, Return>(arg), func(f) {}

    private:
        Return Call(Arg arg) {
            return func(arg);
        }
        Return (*func)(Arg);
    };

    template<typename Return, typename Klass>
    class FutureFunctorKlass : public Callable<Klass*, Return> {
    public:
        explicit FutureFunctorKlass(Klass *klass, Return (Klass::*f)()) : Callable<Klass*, Return>(klass), func(f) {}

    private:
        Return Call(Klass *arg) {
            return (arg->*func)();
        }
        Return (Klass::*func)();
    };

    template<typename Return, typename Arg>
    inline std::auto_ptr< RunnableFuture<Return> > CreateRunnableFuture(Return (*f)(Arg), const Arg &arg) {
        return std::auto_ptr< RunnableFuture<Return> >(new FutureFunctor<Return, Arg>(f, arg));
    }

    template<typename Return, typename Klass>
    inline std::auto_ptr< RunnableFuture<Return> > CreateRunnableFuture(Klass *klass, Return (Klass::*f)()) {
        return std::auto_ptr< RunnableFuture<Return> >(new FutureFunctorKlass<Return, Klass>(klass, f));
    }
}



#pragma once
#include "FutureFunctional.h"
#include <tr1/memory>

namespace Sync {

    using std::tr1::shared_ptr;
    class Executor {
    public:

        Executor();
        virtual ~Executor();

        template<typename Return, typename Arg>
        shared_ptr<Future<Return> > Execute(Return (*func)(Arg), const Arg &arg) {
            std::auto_ptr< RunnableFuture<Return> > aret = CreateRunnableFuture(func, arg);
            shared_ptr< RunnableFuture<Return> > ret = 
                shared_ptr< RunnableFuture<Return> >(aret);
            Execute(ret);
            return ret;
        }

        template<typename Return, typename Klass>
        shared_ptr<Future<Return> > Execute(Klass *klass, Return (Klass::*f)()) {
            std::auto_ptr< RunnableFuture<Return> > aret = CreateRunnableFuture(klass, f);
            shared_ptr< RunnableFuture<Return> > ret = 
                shared_ptr< RunnableFuture<Return> >(aret);
            Execute(ret);
            return ret;
        }

        virtual void Execute(shared_ptr<Runnable> r) = 0;
    };
}

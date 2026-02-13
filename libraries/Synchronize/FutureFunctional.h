//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */
#ifndef SYNC_FUTUREFUNCTIONAL_H
#define SYNC_FUTUREFUNCTIONAL_H
#pragma once
#include "Callable.h"
#include <memory>
#include <tr1/memory>

namespace Sync {
    using std::tr1::shared_ptr;

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

    template<typename Return, typename Klass>
    class FutureFunctorShared : public Callable<shared_ptr<Klass>, Return> {
    public:
        explicit FutureFunctorShared(shared_ptr<Klass> klass, Return (Klass::*f)())
            : Callable<shared_ptr<Klass>, Return>(klass), func(f) {}
    private:
        Return Call(shared_ptr<Klass> klass) {
            return (klass.get()->*func)();
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

    template<typename Return, typename Klass>
    inline std::auto_ptr< RunnableFuture<Return> > CreateRunnableFuture(shared_ptr<Klass> klass, Return (Klass::*f)()) {
        return std::auto_ptr< RunnableFuture<Return> >(new FutureFunctorShared<Return, Klass>(klass, f));
    }
}
#endif

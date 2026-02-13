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
#ifndef SYNC_EXECUTOR_H
#define SYNC_EXECUTOR_H
#pragma once
#include "FutureFunctional.h"
#include <tr1/memory>

namespace Sync {

    using std::tr1::shared_ptr;
    /**
     * The executor executes things...
     */
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

        template<typename Return, typename Klass>
        shared_ptr<Future<Return> > Execute(shared_ptr<Klass> klass, Return (Klass::*f)()) {
            std::auto_ptr< RunnableFuture<Return> > aret = CreateRunnableFuture(klass, f);
            shared_ptr< RunnableFuture<Return> > ret = 
                shared_ptr< RunnableFuture<Return> >(aret);
            Execute(ret);
            return ret;
        }


        virtual void Execute(shared_ptr<Runnable> r) = 0;
    };
}
#endif

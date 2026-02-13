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
#ifndef SYNC_CALLABLE_H
#define SYNC_CALLABLE_H
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
#endif

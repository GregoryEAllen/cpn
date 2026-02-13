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
#ifndef SYNC_FUTURE_H
#define SYNC_FUTURE_H
#pragma once
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "AutoLock.h"

namespace Sync {
    /**
     * The basic idea with the future is an object which will provide a value
     * at some time in the future.
     */
    template<typename T>
    class Future {
    public:
        Future() : done(false), canceled(false), ret() {}
        virtual ~Future() {}

        bool Done() {
            AutoLock<PthreadMutex> al(future_lock);
            return done || canceled;
        }

        void Cancel() {
            AutoLock<PthreadMutex> al(future_lock);
            canceled = true;
            future_cond.Broadcast();
        }

        void Wait() {
            AutoLock<PthreadMutex> al(future_lock);
            InternalWait();
        }

        bool IsCanceled() {
            AutoLock<PthreadMutex> al(future_lock);
            return canceled;
        }

        virtual void Set(const T &r) {
            AutoLock<PthreadMutex> al(future_lock);
            InternalSet(r);
        }

        virtual T Get() {
            AutoLock<PthreadMutex> al(future_lock);
            InternalWait();
            return ret;
        }

    protected:
        void InternalWait() {
            while (!canceled && !done) { future_cond.Wait(future_lock); }
        }

        void InternalSet(const T &r) {
            ret = r;
            done = true;
            future_cond.Broadcast();
        }

        PthreadMutex future_lock;
        PthreadCondition future_cond;
        bool done;
        bool canceled;
        T ret;
    };

    template<>
    class Future<void> {
    public:
        Future() : done(false), canceled(false) {}
        virtual ~Future() {}

        bool Done() {
            AutoLock<PthreadMutex> al(future_lock);
            return done || canceled;
        }

        void Cancel() {
            AutoLock<PthreadMutex> al(future_lock);
            canceled = true;
            future_cond.Broadcast();
        }

        void Wait() {
            AutoLock<PthreadMutex> al(future_lock);
            InternalWait();
        }

        bool IsCanceled() {
            AutoLock<PthreadMutex> al(future_lock);
            return canceled;
        }

        virtual void Set() {
            AutoLock<PthreadMutex> al(future_lock);
            InternalSet();
        }

        virtual void Get() {
            AutoLock<PthreadMutex> al(future_lock);
            InternalWait();
        }

    protected:
        void InternalWait() {
            while (!canceled && !done) { future_cond.Wait(future_lock); }
        }

        void InternalSet() {
            done = true;
            future_cond.Broadcast();
        }

        PthreadMutex future_lock;
        PthreadCondition future_cond;
        bool done;
        bool canceled;
    };

}
#endif

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
 * \brief A reentrant lock implementation.
 * \author John Bridgman
 */

#ifndef SYNC_REENTRANTLOCK_H
#define SYNC_REENTRANTLOCK_H
#pragma once

#include "AutoLock.h"
#include "Assert.h"
#include <pthread.h>

namespace Sync {

    template<class Status_t>
    class StatusHandler;
    class ReentrantCondition;

    namespace Internal {
        class ScopeMutex {
        public:
            ScopeMutex(pthread_mutex_t &l) : lock(l) { ENSURE_ABORT(!pthread_mutex_lock(&lock)); }
            ~ScopeMutex() { ENSURE_ABORT(!pthread_mutex_unlock(&lock)); }
            pthread_mutex_t &lock;
        };
    }

    /**
     * A reentrant lock.
     */
    class ReentrantLock {
    public:
        ReentrantLock() {
            pthread_mutexattr_t attr;
            pthread_mutexattr_init(&attr);
            pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
            ENSURE(!pthread_mutex_init(&lock, &attr));
            pthread_mutexattr_destroy(&attr);
        }
        ~ReentrantLock() {
            ENSURE_ABORT(!pthread_mutex_destroy(&lock));
        }

        void Unlock() const {
            ENSURE_ABORT(!pthread_mutex_unlock(&lock));
        }

        void Lock() const {
            ENSURE_ABORT(!pthread_mutex_lock(&lock));
        }

    private:
        mutable pthread_mutex_t lock;

        template<class T> friend class StatusHandler;
        friend class ReentrantCondition;
    };

    typedef AutoLock<const ReentrantLock> AutoReentrantLock;

    /**
     * \brief Works just like a pthread condition
     * but works with the ReentrantLock
     */
    class ReentrantCondition {
    public:
        ReentrantCondition() { ENSURE(!pthread_cond_init(&cond, 0)); }
        ~ReentrantCondition() { ENSURE(!pthread_cond_destroy(&cond)); }
        void Signal() { pthread_cond_signal(&cond); }
        void Broadcast() { pthread_cond_broadcast(&cond); }
        void Wait(ReentrantLock &lock) const {
            pthread_cond_wait(&cond, &lock.lock);
        }
    private:
        mutable pthread_cond_t cond;
    };
}
#endif

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
#include "ThrowingAssert.h"
#include <pthread.h>
#ifdef SYNC_PROFILE
#include <sys/time.h>
#endif

namespace Sync {

    template<class Status_t>
    class StatusHandler;
    class ReentrantCondition;

    namespace Internal {
#ifdef SYNC_PROFILE
        inline double getTime() {
            timeval tv;
            gettimeofday(&tv, 0);
            return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
        }
#endif
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
#ifdef SYNC_PROFILE
            wait_time = 0;
            lock_time = 0;
            lock_count = 0;
#endif
        }
        ~ReentrantLock() {
            ENSURE_ABORT(!pthread_mutex_destroy(&lock));
        }

        void Unlock() const {
#ifdef SYNC_PROFILE
            lock_count--;
            if (lock_count == 0) {
                lock_time += Internal::getTime() - lock_last;
            }
#endif
            ENSURE_ABORT(!pthread_mutex_unlock(&lock));
        }

        void Lock() const {
#ifdef SYNC_PROFILE
            double start = Internal::getTime();
#endif
            ENSURE_ABORT(!pthread_mutex_lock(&lock));
#ifdef SYNC_PROFILE
            double time = Internal::getTime();
            if (lock_count == 0) {
                lock_last = time;
            }
            lock_count++;
            wait_time += time - start;
#endif
        }

#ifdef SYNC_PROFILE
        double GetWaitTime() const { return wait_time; }
        double GetLockTime() const { return lock_time; }
#endif

    private:
#ifdef SYNC_PROFILE
        mutable double wait_time;
        mutable double lock_time;
        mutable double lock_last;
        mutable unsigned lock_count;
#endif

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
        ReentrantCondition() {
            ENSURE(!pthread_cond_init(&cond, 0));
#ifdef SYNC_PROFILE
            wait_time = 0;
#endif
        }
        ~ReentrantCondition() { ENSURE_ABORT(!pthread_cond_destroy(&cond)); }
        void Signal() { pthread_cond_signal(&cond); }
        void Broadcast() { pthread_cond_broadcast(&cond); }
        void Wait(ReentrantLock &lock) const {
#ifdef SYNC_PROFILE
            double time = Internal::getTime();
            unsigned lock_count = lock.lock_count;
            lock.lock_count = 0;
            lock.lock_time += time - lock.lock_last;
#endif
            pthread_cond_wait(&cond, &lock.lock);
#ifdef SYNC_PROFILE
            double end = Internal::getTime();
            lock.lock_count = lock_count;
            lock.lock_last = end;
            wait_time += end - time;
#endif
        }
#ifdef SYNC_PROFILE
        double GetWaitTime() const { return wait_time; }
#endif
    private:
#ifdef SYNC_PROFILE
        mutable double wait_time;
#endif
        mutable pthread_cond_t cond;
    };
}
#endif

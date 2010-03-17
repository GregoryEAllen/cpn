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
        ReentrantLock() : count(0), owner(0) {
            ENSURE(!pthread_mutex_init(&lock, 0));
            ENSURE(!pthread_cond_init(&cond, 0));
        }
        ~ReentrantLock() {
            ASSERT_ABORT(count == 0, "Mutex still owned by %llu", (unsigned long long) owner);
            ENSURE_ABORT(!pthread_mutex_destroy(&lock));
            ENSURE_ABORT(!pthread_cond_destroy(&cond));
        }

        void Unlock() const {
            Internal::ScopeMutex l(lock);
            ASSERT_ABORT(pthread_equal(owner, pthread_self()),
                    "%llu trying to unlock a mutex owned by %llu",
                    (unsigned long long)pthread_self(), (unsigned long long) owner);
            ASSERT_ABORT(count > 0, "Unlocking a mutex that is not locked.");
            --count;
            if (count == 0) pthread_cond_signal(&cond);
        }

        void Lock() const;

        /**
         * Use only for testing purposes and asserts.
         * \return true if the calling thread has the lock.
         */
        bool HaveLock() const;

    private:
        void Wait(pthread_cond_t& c) const;
        mutable pthread_mutex_t lock;
        mutable pthread_cond_t cond;
        mutable long count;
        mutable pthread_t owner;

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
            Internal::ScopeMutex l(lock.lock);
            lock.Wait(cond);
        }
    private:
        mutable pthread_cond_t cond;
    };
}
#endif

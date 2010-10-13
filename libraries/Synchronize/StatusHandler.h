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
 * \brief Declaration and implementation for the StatusHandler.
 * \author John Bridgman
 */

#ifndef SYNC_STATUSHANDLER_H
#define SYNC_STATUSHANDLER_H
#pragma once

#include "ReentrantLock.h"
#include "Assert.h"

namespace Sync {

    namespace Internal {
        class ScopeMutex {
        public:
            ScopeMutex(pthread_mutex_t &l) : lock(l) { ENSURE_ABORT(!pthread_mutex_lock(&lock)); }
            ~ScopeMutex() { ENSURE_ABORT(!pthread_mutex_unlock(&lock)); }
            pthread_mutex_t &lock;
        };
    }

    /**
     * The StatusHandler template is meant to be used to simplify
     * having a status variable in a multi threaded application.
     * The StatusHandler can be used much like an atomic variable
     * or a blocking queue with size 1.
     *
     * Future improvements to this might be to use actual
     * atomic compare and set primatives for these operations rather
     * than a lock and condition.
     *
     * The ReentrantLock passed in the constructor must be valid
     * for the lifetime of this object and you must hold the
     * lock when calling any of the wait functions.
     */
    template<class Status_t>
    class StatusHandler {
    public:
        StatusHandler(Status_t initialStatus)
               : status(initialStatus) {
            ENSURE(!pthread_mutex_init(&lock, 0));
            ENSURE(!pthread_cond_init(&cond, 0));
        }

        ~StatusHandler() {
            ENSURE_ABORT(!pthread_cond_destroy(&cond));
            ENSURE_ABORT(!pthread_mutex_destroy(&lock));
        }

        /**
         * Post a change in status.
         * \param newStatus the new status
         */
        void Post(Status_t newStatus) {
            Internal::ScopeMutex l(lock);
            status = newStatus;
            pthread_cond_broadcast(&cond);
        }

        /**
         * \return the current status
         */
        Status_t Get() const {
            Internal::ScopeMutex l(lock);
            return status;
        }

        /**
         * Post a change in status only if the current
         * status is oldStatus.
         * \param oldStatus the status to compare to
         * \param newStatus the new status to change to
         * \return true if the status changed
         */
        bool CompareAndPost(Status_t oldStatus, Status_t newStatus) {
            Internal::ScopeMutex l(lock);
            if (oldStatus == status) {
                status = newStatus;
                pthread_cond_broadcast(&cond);
                return true;
            }
            return false;
        }

        template <class Comparator>
        bool CompareAndPost(Status_t oldStatus, Status_t newStatus, Comparator comp) {
            Internal::ScopeMutex l(lock);
            if (comp(oldStatus, status)) {
                status = newStatus;
                pthread_cond_broadcast(&cond);
                return true;
            }
            return false;
        }

        /**
         * This function waits until the status is
         * different than oldStatus.
         * \param oldStatus the status to compare to the
         * current status
         * \return the new status
         */
        Status_t CompareAndWait(Status_t oldStatus) const {
            Internal::ScopeMutex l(lock);
            while (oldStatus == status) { pthread_cond_wait(&cond, &lock); }
            return status;
        }

        template<class Comparator>
        Status_t CompareAndWait(Status_t oldStatus, Comparator comp) const {
            Internal::ScopeMutex l(lock);
            while (comp(oldStatus, status)) { pthread_cond_wait(&cond, &lock); }
            return status;
        }

        /**
         * This function compares status to oldStatus and if the same
         * sets the status to newStatus. Then waits until the status is
         * different than newStatus.
         * \param oldStatus the status to compare
         * \param newStatus the status to set the status to
         * \return the status as of the return of this function
         */
        Status_t ComparePostAndWait(Status_t oldStatus, Status_t newStatus) {
            Internal::ScopeMutex l(lock);
            if (oldStatus == status) {
                status = newStatus;
                pthread_cond_broadcast(&cond);
                while (status == newStatus) { pthread_cond_wait(&cond, &lock); }
            }
            return status;
        }

        template<class Comparator>
        Status_t ComparePostAndWait(Status_t oldStatus, Status_t newStatus, Comparator comp) {
            Internal::ScopeMutex l(lock);
            if (comp(oldStatus, status)) {
                status = newStatus;
                pthread_cond_broadcast(&cond);
                while (newStatus == status) { pthread_cond_wait(&cond, &lock); }
            }
            return status;
        }

        /**
         * Wait for the status to become theStatus then set to newStatus.
         */
        void CompareWaitAndPost(Status_t theStatus, Status_t newStatus) {
            Internal::ScopeMutex l(lock);
            while (theStatus != status) { pthread_cond_wait(&cond, &lock); }
            status = newStatus;
            pthread_cond_broadcast(&cond);
        }

    private:
        Status_t status;
        mutable pthread_mutex_t lock;
        mutable pthread_cond_t cond;
    };
}
#endif


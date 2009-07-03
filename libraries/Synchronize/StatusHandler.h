/** \file
 * \brief Declaration and implementation for the StatusHandler.
 *
 */

#ifndef SYNC_STATUSHANDLER_H
#define SYNC_STATUSHANDLER_H

#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "ReentrantLock.h"
#include "AutoLock.h"

namespace Sync {
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
		StatusHandler(Status_t initialStatus, ReentrantLock& lock_)
		       : status(initialStatus), lock(lock) {};

		/**
		 * Post a change in status.
		 * \param newStatus the new status
		 */
		void Post(Status_t newStatus) {
			PthreadMutexProtected l(lock.lock);
			status = newStatus;
			cond.Broadcast();
		}

		/**
		 * \return the current status
		 */
		Status_t Get(void) const {
			PthreadMutexProtected l(lock.lock);
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
			PthreadMutexProtected l(lock.lock);
			if (oldStatus == status) {
				status = newStatus;
				cond.Broadcast();
				return true;
			}
			return false;
		}

		template <class Comparator>
		bool CompareAndPost(Status_t oldStatus, Status_t newStatus, Comparator comp) {
			PthreadMutexProtected l(lock.lock);
			if (comp(oldStatus, status)) {
				status = newStatus;
				cond.Broadcast();
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
			PthreadMutexProtected l(lock.lock);
			while (oldStatus == status) { lock.Wait(cond); }
			return status;
		}

		template<class Comparator>
		Status_t CompareAndWait(Status_t oldStatus, Comparator comp) const {
			PthreadMutexProtected l(lock.lock);
			while (comp(oldStatus, status)) { lock.Wait(cond); }
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
			PthreadMutexProtected l(lock.lock);
			if (oldStatus == status) {
				status = newStatus;
				cond.Broadcast();
				while (status == newStatus) { lock.Wait(cond); }
			}
			return status;
		}

		template<class Comparator>
		Status_t ComparePostAndWait(Status_t oldStatus, Status_t newStatus, Comparator comp) {
			PthreadMutexProtected l(lock.lock);
			if (comp(oldStatus, status)) {
				status = newStatus;
				cond.Broadcast();
				while (newStatus == status) { lock.Wait(cond); }
			}
			return status;
		}

		/**
		 * Wait for the status to become theStatus then set to newStatus.
		 */
		void CompareWaitAndPost(Status_t theStatus, Status_t newStatus) {
			PthreadMutexProtected l(lock.lock);
			while (theStatus != status) { lock.Wait(cond); }
			status = newStatus;
			cond.Broadcast();
		}

	private:
		Status_t status;
		mutable ReentrantLock lock;
		mutable PthreadCondition cond;
	};
}
#endif


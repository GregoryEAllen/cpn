/** \file
 * \brief Declaration and implementation for the StatusHandler.
 *
 */

#ifndef SYNC_STATUSHANDLER_H
#define SYNC_STATUSHANDLER_H

#include "PthreadMutex.h"
#include "PthreadCondition.h"

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
	 */
	template<class Status_t>
	class StatusHandler {
	public:
		StatusHandler(Status_t initialStatus) : status(initialStatus) {};

		/**
		 * Post a change in status.
		 * \param newStatus the new status
		 */
		void Post(Status_t newStatus) {
			PthreadMutexProtected l(lock);
			status = newStatus;
			cond.Broadcast();
		}

		/**
		 * \return the current status
		 */
		Status_t Get(void) const {
			PthreadMutexProtected l(lock);
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
			PthreadMutexProtected l(lock);
			if (oldStatus == status) {
				status = newStatus;
				cond.Broadcast();
				return true;
			}
			return false;
		}

		template <class Comparator>
		bool CompareAndPost(Status_t oldStatus, Status_t newStatus, Comparator comp) {
			PthreadMutexProtected l(lock);
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
			PthreadMutexProtected l(lock);
			while (oldStatus == status) { cond.Wait(lock); }
			return status;
		}

		template<class Comparator>
		Status_t CompareAndWait(Status_t oldStatus, Comparator comp) const {
			PthreadMutexProtected l(lock);
			while (comp(oldStatus, status)) { cond.Wait(lock); }
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
			PthreadMutexProtected l(lock);
			if (oldStatus == status) {
				status = newStatus;
				cond.Broadcast();
				while (status == newStatus) { cond.Wait(lock); }
			}
			return status;
		}

		template<class Comparator>
		Status_t ComparePostAndWait(Status_t oldStatus, Status_t newStatus, Comparator comp) {
			PthreadMutexProtected l(lock);
			if (comp(oldStatus, status)) {
				status = newStatus;
				cond.Broadcast();
				while (newStatus == status) { cond.Wait(lock); }
			}
			return status;
		}

	private:
		Status_t status;
		mutable PthreadMutex lock;
		mutable PthreadCondition cond;
	};
}
#endif


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

		/**
		 * \return the current status
		 */
		Status_t Get(void) const {
			PthreadMutexProtected l(lock);
			return status;
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

	private:
		Status_t status;
		mutable PthreadMutex lock;
		mutable PthreadCondition cond;
	};
}
#endif


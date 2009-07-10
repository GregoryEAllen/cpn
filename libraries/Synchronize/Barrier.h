/** \file
 */

#ifndef SYNC_BARRIER_H
#define SYNC_BARRIER_H

#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <deque>

namespace Sync {

	/**
	 * When waited upon cause the thread to block
	 * until at least count threads are currently blocked.
	 * Then resets.
	 *
	 * Count may be 1 in which case all waits succeed.
	 * Count may not be 0.
	 *
	 * Count may be modified while waits are in progress.
	 * If count goes under the number of threads waiting
	 * then wait will return count times.
	 *
	 */
	class Barrier {
	public:
		explicit Barrier(unsigned long count_)
		       	: count(count_), numwaiters(0), numwakeup(0) {
				if (count == 0) count = 1;
			}
		~Barrier() {}

		void IncrCount(unsigned long change = 1) {
			PthreadMutexProtected p(lock);
			count += change;
		}
		void DecrCount(unsigned long change = 1) {
			PthreadMutexProtected p(lock);
			count -= change;
			if (count == 0) count = 1;
			WakeUp();
		}
		unsigned long GetCount(void) {
			PthreadMutexProtected p(lock);
			return count;
		}
		unsigned long GetNumWaiter(void) {
			PthreadMutexProtected p(lock);
			return numwaiters;
		}
		void Wait(void) {
			PthreadMutexProtected p(lock);
			WakeStatus wakeStat;
			waiters.push_front(&wakeStat);
			WakeUp();
			while (wakeStat.wakeup == false) {
				cond.Wait(lock);
			}
		}
	private:
		Barrier(const Barrier&);
		Barrier operator=(const Barrier&) const;
		void WakeUp(void) {
			if (waiters.size() >= count) {
				for (unsigned num = count; num > 0; --num) {
					waiters.back()->wakeup = true;
					waiters.pop_back();
				}
				cond.Broadcast();
			}

		}
		unsigned long count;
		PthreadMutex lock;
		PthreadCondition cond;
		struct WaitStatus {
			WaitStatus() : wakeup(false) {}
			bool wakeup;
		}
		std::deque<WaitStatus*> waiters;
	};
}

#endif


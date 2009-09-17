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
 * \brief A barrier implementation
 * \author John Bridgman
 */

#ifndef SYNC_BARRIER_H
#define SYNC_BARRIER_H
#pragma once

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
		       	: count(count_) {
            if (count == 0) count = 1;
        }

		~Barrier() {}

        /**
         * Increment the count of threads that must come to the barrier
         * \param change how many to increament
         */
		void IncrCount(unsigned long change = 1) {
			PthreadMutexProtected p(lock);
			count += change;
		}

        /**
         * Descrement the count of the number of threads
         * that must come to the barrier to continue.
         * Will release if the number of currently waiting
         * threads is greater than the resulting number.
         * \param change how many to decrement
         */
		void DecrCount(unsigned long change = 1) {
			PthreadMutexProtected p(lock);
			count -= change;
			if (count == 0) count = 1;
			WakeUp();
		}
        /// \return the current count
		unsigned long GetCount() {
			PthreadMutexProtected p(lock);
			return count;
		}
        /// \return the number of threads currently waiting
		unsigned long GetNumWaiter() {
			PthreadMutexProtected p(lock);
			return waiters.size();
		}
        /**
         * Wait for the count number threads
         * to come to this barrier.
         */
		void Wait();

	private:
		Barrier(const Barrier&);
		Barrier operator=(const Barrier&) const;
		void WakeUp();

		unsigned long count;
		PthreadMutex lock;
		PthreadCondition cond;
		struct WaitStatus {
			WaitStatus() : wakeup(false) {}
			bool wakeup;
		};
		std::deque<WaitStatus*> waiters;
	};
}

#endif


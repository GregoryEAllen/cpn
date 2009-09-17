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
 * \brief Provide a very simple semaphore class.
 * \author John Bridgman
 */

#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#pragma once

#include "PthreadMutex.h"
#include "PthreadCondition.h"

namespace Sync {
	/**
	 * A very simple semaphore implemented using a mutex
	 * and a condition variable.
	 * No timed wait or try wait functionality provided.
	 *
	 * This implementation allows one to do bulk post to
	 * the semaphore.
	 */
	class Semaphore {
	public:
		typedef unsigned long ulong;
		Semaphore() throw() : value(0) { }

		Semaphore(ulong value) throw() : value(value) { }

		/**
		 * Post to num to the semaphore.
		 *
		 * \param num how many to add to the semaphore (default 1)
		 */
		void Post(ulong num = 1) throw() {
			PthreadMutexProtected l(lock);
			value += num;
			// broadcast because we might have
			// more than one thread waiting
			cond.Broadcast();
		}

		void Release(void) throw() { Post(); }

        void Unlock() throw() { Post(); }

        void Lock() throw() { Wait(); }

		/**
		 * Wait for the semaphore to be greater than 0
		 * and then decrement by one.
		 */
		void Wait(void) throw();

		/**
		 * \return the current value of the semaphore.
		 */
		ulong GetValue(void) throw() {
			PthreadMutexProtected l(lock);
			return value;
		}
	private:
		ulong value;
		PthreadMutex lock;
		PthreadCondition cond;
	};
}
#endif


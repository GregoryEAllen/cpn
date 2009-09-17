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
 * \brief A very simple "event" like synchronization
 * object. Very similar in some respects to the
 * windows Event object.
 * \author John Bridgman
 */

#ifndef SYNC_EVENT_H
#define SYNC_EVENT_H
#pragma once

#include "PthreadMutex.h"
#include "PthreadCondition.h"

namespace Sync {
	/**
	 * A very simple "event" similar in some ways to the
	 * windows Event object.
	 *
	 * An Event can be in one of two states. Signaled and
	 * unsignaled. A call to Wait will cause the thread
	 * to suspend until the Event becomes signaled.
	 * If the event is automatic then after a successful
	 * Wait the event will automatically become non signaled.
	 * If the Event is not automatic then all threads Waiting
	 * on this event will succeed and all future Waits will
	 * succeed immediately until Reset is called.
	 * 
     * Note that an Event(true, true) acts almost like
     * a non reentrant lock. The difference is that the
     * aquiring thread does not have to release the lock.
	 */
	class Event {
	public:
		/**
		 * Default constructor creates a manual reset
		 * event that is not signaled.
		 */
		Event() throw() : automatic(false), signaled(false) {}

		/**
		 * Create an event.
		 * \param automatic true will be automatic, false will be manual
		 * \param signaled true will start out signaled, false will start out not signaled
		 */
		Event(bool automatic, bool signaled = false) throw() :
			automatic(automatic), signaled(signaled) {}

		/**
		 * Signal this event.
		 */
		void Signal() throw() {
			PthreadMutexProtected l(lock);
			signaled = true;
			cond.Broadcast();
		}

		/**
		 * Reset this event.
		 */
		void Reset() throw() {
			PthreadMutexProtected l(lock);
			signaled = false;
		}

		/**
		 * Wait for this event to become signaled.
		 * If it is already signaled then return immediately.
		 * This method will reset the event if the event
		 * is an automatic event.
		 */
		void Wait() throw();

	private:
		const bool automatic;
		bool signaled;
		PthreadMutex lock;
		PthreadCondition cond;
	};
}
#endif


//=============================================================================
//	PthreadBase class
//-----------------------------------------------------------------------------
//	POSIX Pthread class library
//	Copyright (C) 1997-1999  The University of Texas
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

#ifndef PthreadBase_h
#define PthreadBase_h
#pragma once

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"
#include "PthreadScheduleParam.h"
#include "PthreadAttr.h"
#include <signal.h>


class PthreadBase : public PthreadErrorHandler {
  public:
	PthreadBase(void);				// use "self"
	PthreadBase(pthread_t thread);	// use an existing thread
	
	// Actually create threads...
	// (You probably should sub-class Pthread instead)
	PthreadBase(void* (*entryFunc)(void*), void* arg = 0);
	PthreadBase(PthreadAttr& attr, void* (*entryFunc)(void*), void* arg = 0);

	operator pthread_t*(void)		{ return &theThread; }
	operator pthread_t(void)		{ return theThread; }

	int operator == (pthread_t t2)	{ return pthread_equal(theThread, t2); }

	void*	Join(void);

	int Detach(void)				{ return pthread_detach(theThread); }
	int Cancel(void)				{ return pthread_cancel(theThread); }

	static pthread_t Self(void)		{ return pthread_self(); }

	static void	TestCancel(void)	{ pthread_testcancel(); }
	static void Exit(void* value)	{ pthread_exit(value); }

#ifndef MERCURY
	int SendSignal(int sig)			{ return pthread_kill(theThread, sig); }
	int Kill(int sig)				{ return pthread_kill(theThread, sig); }

	static int SendSignal(pthread_t thd, int sig)	{ return pthread_kill(thd, sig); }
	static int Kill(pthread_t thd, int sig)			{ return pthread_kill(thd, sig); }

	static int GetSignalMask(sigset_t* oldSet)
									{ return pthread_sigmask(SIG_SETMASK, 0, oldSet); }
	static int SetSignalMask(const sigset_t* set, sigset_t* oldSet = 0)
									{ return pthread_sigmask(SIG_SETMASK, set, oldSet); }
	static int BlockSignals(const sigset_t* set, sigset_t* oldSet = 0)
									{ return pthread_sigmask(SIG_BLOCK, set, oldSet); }
	static int UnblockSignals(const sigset_t* set, sigset_t* oldSet = 0)
									{ return pthread_sigmask(SIG_UNBLOCK, set, oldSet); }
#endif

	static int CancelEnable(void);
	static int CancelDisable(void);
	
	class CancelProtected {
	  public:
		CancelProtected(void)	{ pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState); }
	   ~CancelProtected(void)	{ pthread_setcancelstate(oldState, &oldState); }
	  private:
		int oldState;
	};

	static int CancelDeferred(void);
	static int CancelAsynchronous(void);

  #if defined(_POSIX_PRIORITY_SCHEDULING) || defined(_POSIX_THREAD_PRIORITY_SCHEDULING)
	int	SchedulePolicy(void);
	int SchedulePriority(void);
	
	static void Yield(void)		{ sched_yield(); }
  #endif
  
  #ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
	//	Thread Scheduling Policies
	//	SCHED_FIFO:	 A thread runs until it blocks (then end of priority line).
	//	SCHED_RR:	 Time-sliced threads (with fixed priorities).
	//	SCHED_OTHER: Implementation dependent (generally Unix-like)

	void GetScheduleParams(int& policy, int& priority);
	void SetScheduleParams(int policy, int priority);
	
	void GetScheduleParams(int& policy, PthreadScheduleParam& sp);
	void SetScheduleParams(int policy, PthreadScheduleParam& sp);
  #endif

  #ifdef _POSIX_PRIORITY_SCHEDULING
	int PriorityMax(void)	{ return sched_get_priority_max(SchedulePolicy()); }
	int PriorityMin(void)	{ return sched_get_priority_min(SchedulePolicy()); }

	static int PriorityMax(int policy)	{ return sched_get_priority_max(policy); }
	static int PriorityMin(int policy)	{ return sched_get_priority_min(policy); }
  #endif

  protected:
	pthread_t		theThread;
};


#endif
#endif

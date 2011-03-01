//=============================================================================
//	PthreadAttr class
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

#ifndef PthreadAttr_h
#define PthreadAttr_h
#pragma once

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"
#include "PthreadScheduleParam.h"
#include <assert.h>


class PthreadAttr : public PthreadErrorHandler {
  public:
	PthreadAttr(int systemScope=0);
   ~PthreadAttr(void);

	PthreadAttr(const PthreadAttr&);	// simple referenceCount version

	operator pthread_attr_t* (void)				{ return &attr; }
	operator const pthread_attr_t* (void) const	{ return &attr; }

	//	Thread Detach State
	void CreateDetached(void)	{ DetachState(PTHREAD_CREATE_DETACHED); }
	void CreateJoinable(void)	{ DetachState(PTHREAD_CREATE_JOINABLE); }
	int  Detached(void)			{ return DetachState() == PTHREAD_CREATE_DETACHED; }
	int  Joinable(void)			{ return DetachState() == PTHREAD_CREATE_JOINABLE; }

  #if defined _XOPEN_REALTIME_THREADS
  #if defined __USE_XOPEN2K
	//	Thread Stack Address and Size
	void*	Stack(void* stackaddr, size_t stacksize);
	void*	Stack(size_t& stackSize);
  #endif
  #elif defined _POSIX_THREAD_ATTR_STACKADDR
	//	Thread Stack Address
	void*	StackAddress(void* stackaddr);
	void*	StackAddress(void);
  #endif

  #ifdef _POSIX_THREAD_ATTR_STACKSIZE
	//	Thread Stack Size
	size_t	StackSize(size_t stacksize);
	size_t	StackSize(void);
  #endif

  #ifdef _POSIX_THREAD_ATTR_PRIORITY_SCHEDULING
	//	Thread Scheduling Inheritance
	void InheritScheduling(void)	{ ScheduleInherit(PTHREAD_INHERIT_SCHED); }
	void ExplicitScheduling(void)	{ ScheduleInherit(PTHREAD_EXPLICIT_SCHED); }
	int  ScheduleInherited(void)	{ return ScheduleInherit() == PTHREAD_INHERIT_SCHED; }
	int  ScheduleExplicit(void)		{ return ScheduleInherit() == PTHREAD_EXPLICIT_SCHED; }

	//	Thread Scheduling Policy
	//	SCHED_FIFO:	 A thread runs until it blocks (then end of priority line).
	//	SCHED_RR:	 Time-sliced threads (with fixed priorities).
	//	SCHED_OTHER: Implementation dependent (generally Unix-like)
	int SchedulePolicy(int policy);
	int SchedulePolicy(void);

	// (using PthreadScheduleParam class)
	void GetScheduleParam(PthreadScheduleParam& sp);
	void SetScheduleParam(PthreadScheduleParam& sp);
  #endif

	//	Thread Scheduling Scope
	void	SystemScope(void)		{ ScheduleScope(PTHREAD_SCOPE_SYSTEM); }
	void	ProcessScope(void)		{ ScheduleScope(PTHREAD_SCOPE_PROCESS); }
	int		InSystemScope(void)		{ return ScheduleScope() == PTHREAD_SCOPE_SYSTEM; }
	int		InProcessScope(void)	{ return ScheduleScope() == PTHREAD_SCOPE_PROCESS; }

  private:
	pthread_attr_t		attr;
	int					referenceCount;
	
	// PTHREAD_CREATE_DETACHED, PTHREAD_CREATE_JOINABLE
	int DetachState(int detState);	
	int DetachState(void);

  #ifdef _POSIX_THREAD_ATTR_PRIORITY_SCHEDULING
	// PTHREAD_INHERIT_SCHED, PTHREAD_EXPLICIT_SCHED
	int ScheduleInherit(int inherit);
	int ScheduleInherit(void);
  #endif

	// PTHREAD_SCOPE_SYSTEM, PTHREAD_SCOPE_PROCESS
	int	ScheduleScope(int scope);
	int	ScheduleScope(void);
};


#endif
#endif

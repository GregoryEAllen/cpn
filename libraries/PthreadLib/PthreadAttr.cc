//=============================================================================
//	$Id: s.PthreadAttr.cc 1.10 03/11/07 17:23:40-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
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

#ifdef EXTERNAL_TEMPLATES
#     pragma implementation "PthreadAttr.h"
#endif

#include "PthreadAttr.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
PthreadAttr::PthreadAttr(int systemScope)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_init(&attr));
	if (systemScope) SystemScope();
	referenceCount = 1;
//	printf("PthreadAttr::PthreadAttr()\n");
}

//-----------------------------------------------------------------------------
PthreadAttr::~PthreadAttr(void)
//-----------------------------------------------------------------------------
{
	referenceCount--;
//	printf("PthreadAttr::~PthreadAttr: referenceCount = %d\n", referenceCount);
	if (!referenceCount)
		TrapError(pthread_attr_destroy(&attr));
}


//-----------------------------------------------------------------------------
PthreadAttr::PthreadAttr(const PthreadAttr& cpAttr)
//-----------------------------------------------------------------------------
{
	attr = cpAttr.attr;
	referenceCount = cpAttr.referenceCount+1;
//	printf("PthreadAttr::PthreadAttr(const PthreadAttr&): referenceCount = %d\n", referenceCount);
}


//-----------------------------------------------------------------------------
int PthreadAttr::DetachState()
//	Gets the thread's detach state.  This is one of:
//		PTHREAD_CREATE_DETACHED: The thread is detached.  It will destroy
//			itself when it exits or terminates.
//		PTHREAD_CREATE_JOINABLE: The thread is joinable.  It will remain 
//					when it exits or terminates, and wait for its parent to
//					rejoin with it (see Thread::Join()).
//-----------------------------------------------------------------------------
{
	int detState;
	TrapError(pthread_attr_getdetachstate(&attr, &detState)); 
	return detState;
}


//-----------------------------------------------------------------------------
int PthreadAttr::DetachState(int detState)
//	Sets the thread's detach state.  See above for a description of the two 
//		detach states.
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setdetachstate(&attr, detState));
	return detState;
}


#if defined _XOPEN_REALTIME_THREADS
#if defined __USE_XOPEN2K
//-----------------------------------------------------------------------------
void* PthreadAttr::Stack(void* stackaddr, size_t stacksize)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setstack(&attr, stackaddr, stacksize));
	return stackaddr;
}

//-----------------------------------------------------------------------------
void* PthreadAttr::Stack(size_t& stackSize)
//-----------------------------------------------------------------------------
{	void* stackaddr;
	TrapError(pthread_attr_getstack(&attr, &stackaddr, &stackSize));
	return stackaddr;
}
#endif
#elif defined _POSIX_THREAD_ATTR_STACKADDR
//-----------------------------------------------------------------------------
void* PthreadAttr::StackAddress(void* stackaddr)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setstackaddr(&attr, stackaddr));
	return stackaddr;
}

//-----------------------------------------------------------------------------
void* PthreadAttr::StackAddress(void)
//-----------------------------------------------------------------------------
{	void* stackaddr;
	TrapError(pthread_attr_getstackaddr(&attr, &stackaddr));
	return stackaddr;
}
#endif


#ifdef _POSIX_THREAD_ATTR_STACKSIZE
//-----------------------------------------------------------------------------
size_t PthreadAttr::StackSize(size_t stacksize)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setstacksize(&attr, stacksize));
	return stacksize;
}

//-----------------------------------------------------------------------------
size_t PthreadAttr::StackSize(void)
//-----------------------------------------------------------------------------
{	size_t stacksize;
	TrapError(pthread_attr_getstacksize(&attr, &stacksize));
	return stacksize;
}
#endif


#ifdef _POSIX_THREAD_ATTR_PRIORITY_SCHEDULING
//-----------------------------------------------------------------------------
int PthreadAttr::ScheduleInherit(int inherit)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setinheritsched(&attr, inherit));
	return inherit;
}

//-----------------------------------------------------------------------------
int PthreadAttr::ScheduleInherit(void)
//-----------------------------------------------------------------------------
{	int inherit;
	TrapError(pthread_attr_getinheritsched(&attr, &inherit));
	return inherit;
}
#endif


#ifdef _POSIX_THREAD_ATTR_PRIORITY_SCHEDULING
//-----------------------------------------------------------------------------
void PthreadAttr::SetScheduleParam(PthreadScheduleParam& sp)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setschedparam(&attr, sp));
}

//-----------------------------------------------------------------------------
void PthreadAttr::GetScheduleParam(PthreadScheduleParam& sp)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_getschedparam(&attr, sp));
}
#endif


#ifdef _POSIX_THREAD_ATTR_PRIORITY_SCHEDULING
//-----------------------------------------------------------------------------
int PthreadAttr::SchedulePolicy(int policy)
//	SCHED_FIFO:	 A thread runs until it blocks (then end of priority line).
//	SCHED_RR:	 Time-sliced threads (with fixed priorities).
//	SCHED_OTHER: Implementation dependent (generally Unix-like)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setschedpolicy(&attr, policy));
	return policy;
}

//-----------------------------------------------------------------------------
int PthreadAttr::SchedulePolicy(void)
//-----------------------------------------------------------------------------
{	int policy;
	TrapError(pthread_attr_getschedpolicy(&attr, &policy));
	return policy;
}
#endif


//#ifdef _POSIX_THREAD_ATTR_PRIORITY_SCHEDULING
//-----------------------------------------------------------------------------
int PthreadAttr::ScheduleScope(int scope)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_attr_setscope(&attr, scope));
	return scope;
}

//-----------------------------------------------------------------------------
int PthreadAttr::ScheduleScope(void)
//-----------------------------------------------------------------------------
{	int scope;
	TrapError(pthread_attr_getscope(&attr, &scope));
	return scope;
}
//#endif


#if 0
//-----------------------------------------------------------------------------
int PthreadAttr::Priority(int priority)
//-----------------------------------------------------------------------------
{
	PthreadScheduleParam sp.Priority(priority);
	SetScheduleParam(sp);
	return sp.Priority();
}

//-----------------------------------------------------------------------------
int PthreadAttr::Priority(void)
//-----------------------------------------------------------------------------
{
	PthreadScheduleParam sp;
	GetScheduleParam(sp);
	return sp.Priority();
}
#endif


#endif
static const char rcsid[] = "@(#) $Id: s.PthreadAttr.cc 1.10 03/11/07 17:23:40-06:00 gallen@ph.arlut.utexas.edu $";

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

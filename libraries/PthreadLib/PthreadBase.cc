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

#ifdef EXTERNAL_TEMPLATES
#     pragma implementation "PthreadErrorHandler.h"
#     pragma implementation "PthreadBase.h"
#     pragma implementation "PthreadScheduleParam.h"
#endif

#include "PthreadLib.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
PthreadBase::PthreadBase(void)
//-----------------------------------------------------------------------------
{
	theThread = Self();
}


//-----------------------------------------------------------------------------
PthreadBase::PthreadBase(pthread_t thread)
//-----------------------------------------------------------------------------
{
	theThread = thread;
}


//-----------------------------------------------------------------------------
PthreadBase::PthreadBase(void* (*entryFunc)(void*), void* arg)
//-----------------------------------------------------------------------------
{
	TrapError( pthread_create( (pthread_t*)&theThread, 0, entryFunc, arg) );
}


//-----------------------------------------------------------------------------
PthreadBase::PthreadBase(PthreadAttr& attr, void* (*entryFunc)(void*), void* arg)
//-----------------------------------------------------------------------------
{
	TrapError( pthread_create( (pthread_t*)&theThread, (pthread_attr_t*)attr, entryFunc, arg) );
}


//-----------------------------------------------------------------------------
void* PthreadBase::Join(void)
//-----------------------------------------------------------------------------
{
	void* result = 0;
	TrapError( pthread_join(theThread, &result) );
	return result;
}


//-----------------------------------------------------------------------------
int PthreadBase::CancelEnable(void)
//-----------------------------------------------------------------------------
{
	int oldState;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldState);
	return oldState;
}


//-----------------------------------------------------------------------------
int PthreadBase::CancelDisable(void)
//-----------------------------------------------------------------------------
{
	int oldState;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldState);
	return oldState;
}

//-----------------------------------------------------------------------------
int PthreadBase::CancelDeferred(void)
//-----------------------------------------------------------------------------
{
	int oldState;
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, &oldState);
	return oldState;
}


//-----------------------------------------------------------------------------
int PthreadBase::CancelAsynchronous(void)
//-----------------------------------------------------------------------------
{
	int oldState;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldState);
	return oldState;
}


#ifdef _POSIX_THREAD_PRIORITY_SCHEDULING
//-----------------------------------------------------------------------------
int PthreadBase::SchedulePolicy(void)
//-----------------------------------------------------------------------------
{
	int policy;
	PthreadScheduleParam	sp;
	TrapError( pthread_getschedparam(theThread, &policy, sp) );
	return policy;
}


//-----------------------------------------------------------------------------
int PthreadBase::SchedulePriority(void)
//-----------------------------------------------------------------------------
{
	int policy;
	PthreadScheduleParam	sp;
	TrapError( pthread_getschedparam(theThread, &policy, sp) );
	return sp.Priority();
}


//-----------------------------------------------------------------------------
void PthreadBase::GetScheduleParams(int& policy, int& priority)
//-----------------------------------------------------------------------------
{
	PthreadScheduleParam	sp;
	TrapError( pthread_getschedparam(theThread, &policy, sp) );
	priority = sp.Priority();
}


//-----------------------------------------------------------------------------
void PthreadBase::SetScheduleParams(int policy, int priority)
//-----------------------------------------------------------------------------
{
	PthreadScheduleParam	sp;
	sp.Priority(priority);
	TrapError( pthread_setschedparam(theThread, policy, sp) );
}


//-----------------------------------------------------------------------------
void PthreadBase::GetScheduleParams(int& policy, PthreadScheduleParam& sp)
//-----------------------------------------------------------------------------
{
	TrapError( pthread_getschedparam(theThread, &policy, sp) );
}


//-----------------------------------------------------------------------------
void PthreadBase::SetScheduleParams(int policy, PthreadScheduleParam& sp)
//-----------------------------------------------------------------------------
{
	TrapError( pthread_setschedparam(theThread, policy, sp) );
}
#endif


#endif

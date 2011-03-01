//=============================================================================
//	PthreadMutexAttr class
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
#     pragma implementation "PthreadMutexAttr.h"
#endif

#include "PthreadMutexAttr.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
PthreadMutexAttr::PthreadMutexAttr(void)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_mutexattr_init(&theAttr));
}

//-----------------------------------------------------------------------------
PthreadMutexAttr::~PthreadMutexAttr(void)
//-----------------------------------------------------------------------------
{
    try {
        TrapError(pthread_mutexattr_destroy(&theAttr));
    } catch (...) {
        std::terminate();
    }
}


#if defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT+0>0)
//-----------------------------------------------------------------------------
int PthreadMutexAttr::PriorityCeiling(void)
//-----------------------------------------------------------------------------
{	int ceiling;
	TrapError(pthread_mutexattr_getprioceiling(&theAttr, &ceiling));
	return ceiling;
}

//-----------------------------------------------------------------------------
PthreadMutexAttr& PthreadMutexAttr::PriorityCeiling(int ceiling)
//-----------------------------------------------------------------------------
{	PriorityCeiling();
	TrapError(pthread_mutexattr_setprioceiling(&theAttr, ceiling));
	return *this;
}
#endif


#if (defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT+0>0)) \
 || (defined(_POSIX_THREAD_PRIO_INHERIT) && (_POSIX_THREAD_PRIO_INHERIT+0>0))
//-----------------------------------------------------------------------------
int PthreadMutexAttr::Protocol(void)
//-----------------------------------------------------------------------------
{	int protocol;
	TrapError(pthread_mutexattr_getprotocol(&theAttr, &protocol));
	return protocol;
}

//-----------------------------------------------------------------------------
PthreadMutexAttr& PthreadMutexAttr::Protocol(int protocol)
//-----------------------------------------------------------------------------
{	TrapError(pthread_mutexattr_setprotocol(&theAttr, protocol));
	return *this;
}
#endif


#if defined(_POSIX_THREAD_PROCESS_SHARED) && (_POSIX_THREAD_PROCESS_SHARED+0>0)
//-----------------------------------------------------------------------------
int PthreadMutexAttr::ProcessShared(void)
//-----------------------------------------------------------------------------
{	int pshared;
	TrapError(pthread_mutexattr_getpshared(&theAttr, &pshared));
	return pshared == PTHREAD_PROCESS_SHARED;
}

//-----------------------------------------------------------------------------
PthreadMutexAttr& PthreadMutexAttr::ProcessShared(int pshared)
//-----------------------------------------------------------------------------
{	TrapError(pthread_mutexattr_setpshared(&theAttr, pshared ? 
		PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE));
	return *this;
}
#endif


#endif

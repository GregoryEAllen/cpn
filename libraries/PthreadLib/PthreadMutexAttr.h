//=============================================================================
//	$Id: PthreadMutexAttr.h 1.10 2007/04/30 15:14:21-05:00 koffer@rowlf.arlut.utexas.edu $
//-----------------------------------------------------------------------------
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


#ifndef PthreadMutexAttr_h
#define PthreadMutexAttr_h

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"


class PthreadMutexAttr : public PthreadErrorHandler {
  public:
	PthreadMutexAttr(void);
   ~PthreadMutexAttr(void);
	operator pthread_mutexattr_t* (void)				{ return &theAttr; }
	operator const pthread_mutexattr_t* (void) const	{ return &theAttr; }

	#if (defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT+0>0)) \
	 || (defined(_POSIX_THREAD_PRIO_INHERIT) && (_POSIX_THREAD_PRIO_INHERIT+0>0))
		int					UsingNoPriorityProtocol()	{ return Protocol() == POSIX_PRIO_NONE; }
		PthreadMutexAttr&	NoPriorityProtocol()		{ return Protocol(POSIX_PRIO_NONE); }
	#endif

	#if defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT+0>0)
		int		PriorityCeiling(void);
		int		UsingPriorityCeiling(void)				{ return Protocol() == POSIX_PRIO_PROTECT; }
		PthreadMutexAttr&	PriorityCeiling(int ceiling);
	#endif

	#if defined(_POSIX_THREAD_PRIO_INHERIT) && (_POSIX_THREAD_PRIO_INHERIT+0>0)
		PthreadMutexAttr&	PriorityInheritance()		{ return Protocol(POSIX_PRIO_INHERIT); }
		int					UsingPriorityInheritance()	{ return Protocol() == POSIX_PRIO_INHERIT; }
	#endif

	#if defined(_POSIX_THREAD_PROCESS_SHARED) && (_POSIX_THREAD_PROCESS_SHARED+0>0)
		int					ProcessShared(void);
		PthreadMutexAttr&	ProcessShared(int pshared);
	#endif

  private:
	pthread_mutexattr_t			theAttr;

	#if (defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT+0>0)) \
	 || (defined(_POSIX_THREAD_PRIO_INHERIT) && (_POSIX_THREAD_PRIO_INHERIT+0>0))
		int					Protocol(void);
		PthreadMutexAttr&	Protocol(int protocol);
	#endif

	friend class PthreadMutex;
};


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

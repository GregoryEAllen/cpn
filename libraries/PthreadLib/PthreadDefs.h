//=============================================================================
//	$Id: PthreadDefs.h 1.17 2007/04/30 15:14:20-05:00 koffer@rowlf.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	Definitions used throughout the Pthread class library
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


#ifndef	PthreadDefs_h
#define PthreadDefs_h

#ifdef MERCURY
	// Mercury's pthread support is unmaintained and broken in several ways...
	#include <mcos.h>
	#include <unistd.h>
	#include <pthread.h>
	#define POSIX_PRIO_NONE			NO_PRIO_INHERIT
	#define POSIX_PRIO_INHERIT		PRIO_INHERIT
	#define POSIX_PRIO_PROTECT		PRIO_PROTECT
	#define PTHREAD_CREATE_JOINABLE	PTHREAD_CREATE_UNDETACHED

#elif defined(OS_HPUX_1100) || defined(OS_HPUX_1111)
	// HP's pthread support isn't standard...need to define _POSIX_THREADS
	#define _POSIX_THREADS 1
	#include <unistd.h>
	#include <pthread.h>

#elif defined(OS_HPUX_1020)
	// HP's pthread support is non-existent.
        // (may be able to use GNU pth library instead)
	#include <sched.h>
#else

	#include <unistd.h>
	#ifdef _POSIX_THREADS

		#include <pthread.h>

		#if defined(_POSIX_THREAD_PRIO_INHERIT) && (_POSIX_THREAD_PRIO_INHERIT < 0)
                	#undef _POSIX_THREAD_PRIO_INHERIT
                #endif

 		#if defined(_POSIX_THREAD_PRIO_PROTECT) && (_POSIX_THREAD_PRIO_PROTECT < 0)
                	#undef _POSIX_THREAD_PRIO_PROTECT
                #endif
               
		#if !defined POSIX_PRIO_NONE
			#define POSIX_PRIO_NONE		PTHREAD_PRIO_NONE
			#define POSIX_PRIO_INHERIT	PTHREAD_PRIO_INHERIT
			#define POSIX_PRIO_PROTECT	PTHREAD_PRIO_PROTECT
		#endif


		// AIX has some bad ones, too
		#if defined(OS_AIX)
			#define PTHREAD_CREATE_JOINABLE	PTHREAD_CREATE_UNDETACHED
			inline int pthread_sigmask(int operation, const sigset_t *set, 
				sigset_t *old_set)
				{ return sigthreadmask(operation, set, old_set); }
			inline int sched_yield (void)
				{ pthread_yield(); return 0; }
		#endif

		// Darwin (MacOS X) had problems prior to gcc-3.3
		#if defined(OS_DARWIN) && !( ((__GNUC__==3) && (__GNUC_MINOR__>=3)) || (__GNUC__>3) )
			inline int pthread_kill(pthread_t thread, int sig)
				{ return -1; }
			inline int pthread_sigmask(int operation, const sigset_t *set, sigset_t *old_set)
				{ return -1; }
			#define POSIX_PRIO_NONE     PTHREAD_PRIO_NONE
			#define POSIX_PRIO_INHERIT  PTHREAD_PRIO_INHERIT
			#define POSIX_PRIO_PROTECT  PTHREAD_PRIO_PROTECT
			#undef pthread_cleanup_push
    		#undef pthread_cleanup_pop
			#define pthread_cleanup_push(routine, args)
			#define pthread_cleanup_pop(execute)
			inline int pthread_condattr_init(pthread_condattr_t *attr)
				{ return -1; }
			inline int pthread_condattr_destroy(pthread_condattr_t *attr)
				{ return -1; }
		#endif

	#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================
#endif

//=============================================================================
//	PthreadMutex class
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
#     pragma implementation "PthreadMutex.h"
#endif

#include "PthreadMutex.h"
#ifdef _POSIX_THREADS
#include <errno.h>


//-----------------------------------------------------------------------------
PthreadMutex::PthreadMutex(void)
//-----------------------------------------------------------------------------
//:	theMutex(PTHREAD_MUTEX_INITIALIZER)
{
	TrapError(pthread_mutex_init(&theMutex, 0));
}


//-----------------------------------------------------------------------------
PthreadMutex::PthreadMutex(const PthreadMutexAttr& mAttr)
//-----------------------------------------------------------------------------
{	
	//	It should simply be:
//	TrapError(pthread_mutex_init(&theMutex, mAttr));

	//	This is necessary because
	//		SGI's pthread_mutex_init is not const correct!!!

	pthread_mutexattr_t* myAttr = (PthreadMutexAttr&)mAttr;
	TrapError(pthread_mutex_init(&theMutex, myAttr));
}


//-----------------------------------------------------------------------------
PthreadMutex::~PthreadMutex(void)
//-----------------------------------------------------------------------------
{
    try {
        TrapError(pthread_mutex_destroy(&theMutex));
    } catch (...) {
        std::terminate();
    }
}


//-----------------------------------------------------------------------------
bool PthreadMutex::Poll(void)
//-----------------------------------------------------------------------------
{	int err = pthread_mutex_trylock(&theMutex);
	if(err == EBUSY)
		return false;
	TrapError(err);
	return true;
}


#endif

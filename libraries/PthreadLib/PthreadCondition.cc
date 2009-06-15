//=============================================================================
//	$Id: s.PthreadCondition.cc 1.9 03/11/07 17:23:41-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadCondition class
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


#include "PthreadCondition.h"
#include "PthreadConditionAttr.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
PthreadCondition::PthreadCondition(void)
//-----------------------------------------------------------------------------
//:	theCond(PTHREAD_COND_INITIALIZER)
{
	TrapError(pthread_cond_init(&theCond, 0));
}


//-----------------------------------------------------------------------------
PthreadCondition::PthreadCondition(const PthreadConditionAttr& cAttr)
//-----------------------------------------------------------------------------
{
	//	It should simply be:
//	TrapError(pthread_cond_init(&theCond, cAttr));

	//	This is necessary because
	//		SGI's pthread_cond_init is not const correct!!!
	
	pthread_condattr_t* myAttr = (PthreadConditionAttr&)cAttr;
	TrapError(pthread_cond_init(&theCond, myAttr));
}


//-----------------------------------------------------------------------------
PthreadCondition::~PthreadCondition()
//-----------------------------------------------------------------------------
{
	TrapError(pthread_cond_destroy(&theCond));
}


//-----------------------------------------------------------------------------
PthreadCondition& PthreadCondition::Signal()
//-----------------------------------------------------------------------------
{	TrapError(pthread_cond_signal(&theCond));
	return *this;
}


//-----------------------------------------------------------------------------
PthreadCondition& PthreadCondition::Broadcast()
//-----------------------------------------------------------------------------
{	TrapError(pthread_cond_broadcast(&theCond));
	return *this;
}


//-----------------------------------------------------------------------------
PthreadCondition& PthreadCondition::Wait(PthreadMutex& mutex)
//-----------------------------------------------------------------------------
{	TrapError(pthread_cond_wait(&theCond, mutex));
	return *this;
}

//-----------------------------------------------------------------------------
PthreadCondition& PthreadCondition::TimedWait(PthreadMutex& mutex, const timespec* abstime)
//-----------------------------------------------------------------------------
{	TrapError(pthread_cond_timedwait(&theCond, mutex, abstime));
	return *this;
}


#endif
static const char rcsid[] = "@(#) $Id: s.PthreadCondition.cc 1.9 03/11/07 17:23:41-06:00 gallen@ph.arlut.utexas.edu $";

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

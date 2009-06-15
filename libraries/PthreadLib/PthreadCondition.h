//=============================================================================
//	$Id: s.PthreadCondition.h 1.10 03/11/07 17:23:41-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadCondition and PthreadConditionProtected classes
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


#ifndef PthreadCondition_h
#define PthreadCondition_h

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"
#include "PthreadMutex.h"
#include "PthreadConditionAttr.h"

class PthreadCondition : public PthreadErrorHandler {
  public:

	PthreadCondition(void);
	PthreadCondition(const PthreadConditionAttr& condattr);
   ~PthreadCondition(void);

	inline	operator pthread_cond_t* (void)	{ return &theCond; }

	PthreadCondition&	Signal(void);
	PthreadCondition&	Broadcast(void);
	PthreadCondition&	Wait(PthreadMutex& mutex);
	
	// BEWARE!  abstime is the time in the future to wake up, not the interval
	// i.e.  usleep(100) is equivalent to:   struct timespec ts;
	//       clock_gettime(CLOCK_REALTIME, &ts); ts.tv_nsec += 100000;
	PthreadCondition&	TimedWait(PthreadMutex& mutex, const timespec* abstime);

  private:
	pthread_cond_t		theCond;
};


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

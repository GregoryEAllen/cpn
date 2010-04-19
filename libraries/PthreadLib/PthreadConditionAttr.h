//=============================================================================
//	$Id: s.PthreadConditionAttr.h 1.8 03/11/07 17:23:42-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadConditionAttr class
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


#ifndef PthreadConditionAttr_h
#define PthreadConditionAttr_h

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"


class PthreadConditionAttr : public PthreadErrorHandler {
  public:
	PthreadConditionAttr(void);
   ~PthreadConditionAttr(void);

	operator pthread_condattr_t* (void)				{ return &condAttr; }
	operator const pthread_condattr_t* (void) const	{ return &condAttr; }

	#ifdef _POSIX_THREAD_PROCESS_SHARED
		int						ProcessShared(void);
		PthreadConditionAttr&	ProcessShared(int pshared);
	#endif

  private:
	pthread_condattr_t	condAttr;
};


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

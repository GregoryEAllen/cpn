//=============================================================================
//	$Id: s.PthreadMutex.h 1.9 03/11/07 17:23:42-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadMutex and PthreadMutexProtected classes
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


#ifndef PthreadMutex_h
#define PthreadMutex_h

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"
#include "PthreadMutexAttr.h"


class PthreadMutex : public PthreadErrorHandler {
  public:
	PthreadMutex(void);
	PthreadMutex(const PthreadMutexAttr& attr);
   ~PthreadMutex(void);
   
	inline operator pthread_mutex_t* ()				{ return &theMutex; }
	inline operator const pthread_mutex_t* () const	{ return &theMutex; }

	PthreadMutex&	Lock(void)		{ TrapError(pthread_mutex_lock(&theMutex)); return *this; }
	PthreadMutex&	Unlock(void)	{ TrapError(pthread_mutex_unlock(&theMutex)); return *this; }
	PthreadMutex&	Take(void)		{ return Lock(); }
	PthreadMutex&	Give(void)		{ return Unlock(); }

	bool			Poll(void);

private:
	pthread_mutex_t	theMutex;
};


class PthreadMutexProtected {
  public:
	PthreadMutexProtected(PthreadMutex& tm): theMutex(&tm)	{ theMutex->Take(); }
   ~PthreadMutexProtected()									{ theMutex->Give(); }
  private:
	PthreadMutex*	theMutex;
};


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

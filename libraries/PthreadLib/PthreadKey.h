//=============================================================================
//	$Id: s.PthreadKey.h 1.5 03/11/07 17:23:42-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadKey template class.
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

#ifndef PthreadKey_h
#define PthreadKey_h


#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"


template<class T>
class PthreadKey: public PthreadErrorHandler {
  public:
	PthreadKey(void (*destructor)(T) = 0)
		{ TrapError(pthread_key_create(&theKey, (void(*)(void*))destructor)); }
   ~PthreadKey(void)
   		{ try { TrapError(pthread_key_delete(theKey)); } catch (...) { std::terminate(); } }

	operator pthread_key_t* (void)				{ return &theKey; }
	operator const pthread_key_t* (void) const	{ return &theKey; }

	T				Get(void) const
					{ return (T)pthread_getspecific(theKey); }

	PthreadKey<T>&	Set(T val)
					{	TrapError(pthread_setspecific(theKey, (void*)val));
						return *this;
					}

  private:
	pthread_key_t	theKey;
};


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

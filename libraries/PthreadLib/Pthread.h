//=============================================================================
//	$Id: s.Pthread.h 1.8 03/11/07 17:23:40-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	Pthread class
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

#ifndef Pthread_h
#define Pthread_h

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadBase.h"
#include "PthreadMutex.h"


class Pthread : public PthreadBase {
  public:
	Pthread(void);
	Pthread(const PthreadAttr& attr);
	virtual ~Pthread(void);

	void	Start(void)				{ state = started; startMutex.Unlock(); }
	int		Running(void)			{ return state == running; }
	int		Done(void)				{ return state == done; }

  protected:
	virtual void* EntryPoint(void) = 0;
//	virtual void  Cleanup(void)		{ }

  private:
	PthreadMutex	startMutex;

	enum PthreadState { uninitialized = 0, created, started, running, done };
	PthreadState	state;

	static void*	PthreadEntryPoint(void* arg);
	static void		PthreadCleanup(void* arg);
};


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

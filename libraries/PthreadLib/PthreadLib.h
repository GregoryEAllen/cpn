//=============================================================================
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
#pragma once

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"

//=============================================================================
//  WARNING: subclassing class Pthread can lead to race conditions.
//  It is recommended that you instead use PthreadFunctional.
//-----------------------------------------------------------------------------
//  This Pthread class was written in the style of the Java Thread class.
//  Unfortunately, C++ has some behaviors that can cause problems when you
//  use inheritance with a thread class. Some simple rules can mitigate:
//  1) NEVER call Start() in the constructor of a derived class. There are
//      no virtual methods inside of C++ constructors, and Pthread::EntryPoint()
//      must be virtual. This can lead to attempted execution before the
//      vtables are initialized.
//  2) ALWAYS call Join() before doing anything else in the destructor of EVERY
//      class that is an inheritance descendent of the Pthread class. Because of
//      the calling order of destructors, your destructor could be executing at
//      the same time as your spawned thread. This could lead to your thread
//      operating on descructed data and deallocated memory.
//=============================================================================

class Pthread : public PthreadBase {
  public:
	Pthread(void);
	Pthread(const PthreadAttr& attr);
	virtual ~Pthread(void);

	void Start(void);

	int Running(void) {
		PthreadMutexProtected p(mutex);
		return state == running;
	}

    /**
     * \return true if Joining would not block.
     */
	int Done(void) {
		PthreadMutexProtected p(mutex);
		return state == done || state == joined;
	}

    void *Join(void);

  protected:
	virtual void* EntryPoint(void) = 0;
//	virtual void  Cleanup(void)		{ }

  private:
	PthreadMutex	mutex;
	PthreadCondition cond;
    void *returnResult;
    bool inJoin;

	enum PthreadState { uninitialized = 0, created, started, running, done, joined, canceled };
	PthreadState	state;

	static void*	PthreadEntryPoint(void* arg);
	static void		PthreadCleanup(void* arg);
};


#endif
#endif

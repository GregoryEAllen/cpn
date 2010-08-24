//=============================================================================
//	$Id: s.Pthread.cc 1.10 03/11/07 17:23:40-06:00 gallen@ph.arlut.utexas.edu $
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

#ifdef EXTERNAL_TEMPLATES
#     pragma implementation "Pthread.h"
#endif

#include "Pthread.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
Pthread::Pthread(void)
//-----------------------------------------------------------------------------
{
	PthreadMutexProtected p(mutex);
    returnResult = 0;
    inJoin = false;
	state = uninitialized;
	TrapError( pthread_create( &theThread, 0, PthreadEntryPoint, this) );
    state = created;
    cond.Broadcast();
}


//-----------------------------------------------------------------------------
Pthread::Pthread(const PthreadAttr& attr)
//-----------------------------------------------------------------------------
{
	PthreadMutexProtected p(mutex);
    returnResult = 0;
    inJoin = false;
	state = uninitialized;

	//	It should simply be:
//	TrapError( pthread_create(&theThread, attr, PthreadEntryPoint, this) );

	//	This is necessary because
	//		SGI's pthread_create is not const correct!!!
	pthread_attr_t* myAttr = (PthreadAttr&)attr;
	TrapError( pthread_create(&theThread, myAttr, PthreadEntryPoint, this) );
    state = created;
    cond.Broadcast();
}


//-----------------------------------------------------------------------------
Pthread::~Pthread(void)
//-----------------------------------------------------------------------------
{
    try {
        // Force the thread to terminate if it has not already done so.
        // Is it safe to do this to a thread that has already terminated?

        bool need_start = false;
        {
            PthreadMutexProtected p(mutex);
            switch (state) {
                case uninitialized:
                    // should be impossible
                    return;
                case created:
                    need_start = true;
                case started:
                case running:
                    state = canceled;
                    cond.Broadcast();
                default:
                    break;
            }
        }

        if (need_start) { Start(); }
        // Now wait.
        Join();
    } catch (...) {
        std::terminate();
    }
}

void Pthread::Start(void) {
    PthreadMutexProtected p(mutex);
    if (state == created) {
        state = started;
        cond.Broadcast();
    }
}

void *Pthread::Join(void) {
    void *result = 0;
    {
        PthreadMutexProtected p(mutex);
        if (inJoin || state == joined) {
            while (state != joined) {
                cond.Wait(mutex);
            }
            return returnResult;
        }
        inJoin = true;
    }
    TrapError( pthread_join(theThread, &result) );
    {
        PthreadMutexProtected p(mutex);
        returnResult = result;
        state = joined;
        cond.Broadcast();
        return returnResult;
    }
}

//-----------------------------------------------------------------------------
void* Pthread::PthreadEntryPoint(void* arg)
//-----------------------------------------------------------------------------
{
	void* result = 0;
	Pthread* ptr = (Pthread*) arg;
	{
		PthreadMutexProtected p(ptr->mutex);
		while (ptr->state == created) {
			ptr->cond.Wait(ptr->mutex);
		}
        if (ptr->state != started) return 0;
		ptr->state = running;
        ptr->cond.Broadcast();
	}
	TestCancel();
	pthread_cleanup_push( PthreadCleanup, ptr);
	result = ptr->EntryPoint();
	pthread_cleanup_pop(1);
	return result;
}


//-----------------------------------------------------------------------------
void Pthread::PthreadCleanup(void* arg)
//-----------------------------------------------------------------------------
{
	Pthread* ptr = (Pthread*) arg;
	PthreadMutexProtected p(ptr->mutex);
//	ptr->Cleanup();		// the sub-classes have already been destructed
	ptr->state = done;
    ptr->cond.Broadcast();
}


#endif

static const char rcsid[] = "@(#) $Id: s.Pthread.cc 1.10 03/11/07 17:23:40-06:00 gallen@ph.arlut.utexas.edu $";

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

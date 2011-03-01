//=============================================================================
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
#ifdef EXTERNAL_TEMPLATES
#     pragma implementation "PthreadConditionAttr.h"
#endif

#include "PthreadConditionAttr.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
PthreadConditionAttr::PthreadConditionAttr(void)
//-----------------------------------------------------------------------------
{
	TrapError(pthread_condattr_init(&condAttr));
}


//-----------------------------------------------------------------------------
PthreadConditionAttr::~PthreadConditionAttr(void)
//-----------------------------------------------------------------------------
{
    try {
        TrapError(pthread_condattr_destroy(&condAttr));
    } catch (...) {
        std::terminate();
    }
}


#ifdef _POSIX_THREAD_PROCESS_SHARED
//-----------------------------------------------------------------------------
int PthreadConditionAttr::ProcessShared(void)
//-----------------------------------------------------------------------------
{	int pshared;
	TrapError(pthread_condattr_getpshared(&condAttr, &pshared));
	return pshared == PTHREAD_PROCESS_SHARED;
}


//-----------------------------------------------------------------------------
PthreadConditionAttr& PthreadConditionAttr::ProcessShared(int pshared)
//-----------------------------------------------------------------------------
{	TrapError(pthread_condattr_setpshared(&condAttr, pshared ? 
		PTHREAD_PROCESS_SHARED : PTHREAD_PROCESS_PRIVATE));
	return *this;
}
#endif


#endif

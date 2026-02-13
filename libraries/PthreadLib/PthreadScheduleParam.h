//=============================================================================
//	PthreadScheduleParam class
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

#ifndef PthreadScheduleParam_h
#define PthreadScheduleParam_h
#pragma once

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"

#if defined(OS_AIX)
	#include <sys/sched.h>
#else
	#include <sched.h>
#endif


class PthreadScheduleParam {
  public:
	PthreadScheduleParam(void) {  }
	PthreadScheduleParam(int priority) { param.sched_priority = priority; }
	operator sched_param* (void) 		{ return &param; }

	int Priority(int priority)	{ return param.sched_priority = priority; }
	int Priority(void) const	{ return param.sched_priority; }

  #ifdef _POSIX_PRIORITY_SCHEDULING
	static int PriorityMax(int policy)	{ return sched_get_priority_max(policy); }
	static int PriorityMin(int policy)	{ return sched_get_priority_min(policy); }

	static void Yield(void)		{ sched_yield(); }
  #endif

  private:
	sched_param		param;
};


#endif

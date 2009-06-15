//=============================================================================
//	$Id: s.PthreadReadWriteLock.cc 1.6 03/11/07 17:23:43-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadReadWriteLock and protection classes
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


#include "PthreadReadWriteLock.h"
#ifdef _POSIX_THREADS


//-----------------------------------------------------------------------------
PthreadReadWriteLock::PthreadReadWriteLock(int _priorityWrites)
//-----------------------------------------------------------------------------
:	numReaders(0),
	writing(0),
	noNewReaders(0),
	priorityWrites(_priorityWrites)
//	mutex(), lockFree()
{
}


//-----------------------------------------------------------------------------
void PthreadReadWriteLock::ReadLock(void)
//-----------------------------------------------------------------------------
{
	PthreadMutexProtected protect(mutex);
	while (noNewReaders)
		lockFree.Wait(mutex);
	numReaders++;
}


//-----------------------------------------------------------------------------
void PthreadReadWriteLock::WriteLock(void)
//-----------------------------------------------------------------------------
{
	PthreadMutexProtected protect(mutex);
	if (priorityWrites)
		noNewReaders = 1;
	while (writing || numReaders)
		lockFree.Wait(mutex);
	writing++;
	noNewReaders = 1;
}


//-----------------------------------------------------------------------------
void PthreadReadWriteLock::ReadUnlock(void)
//-----------------------------------------------------------------------------
{
	PthreadMutexProtected protect(mutex);

	if (numReaders<=0) {
		TrapError(-1);
		return;
	}

	numReaders--;
	if (!numReaders)
		lockFree.Signal();
}


//-----------------------------------------------------------------------------
void PthreadReadWriteLock::WriteUnlock(void)
//-----------------------------------------------------------------------------
{
	PthreadMutexProtected protect(mutex);

	if (!writing) {
		TrapError(-1);
		return;
	}
	
	writing = 0;
	noNewReaders = 0;
	lockFree.Broadcast();
}


#endif
static const char rcsid[] = "@(#) $Id: s.PthreadReadWriteLock.cc 1.6 03/11/07 17:23:43-06:00 gallen@ph.arlut.utexas.edu $";

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

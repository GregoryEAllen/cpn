//=============================================================================
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


#ifndef PthreadReadWriteLock_h
#define PthreadReadWriteLock_h
#pragma once

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadErrorHandler.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"


class PthreadReadWriteLock : public PthreadErrorHandler {
  public:
	// if (priorityWrites) is true, readers must wait for a pending writer
	// otherwise, the writer must have some lucky timing! 
	PthreadReadWriteLock(int _priorityWrites = 1);
//   ~PthreadReadWriteLock(void);

	void ReadLock(void);
	void WriteLock(void);
	void ReadUnlock(void);
	void WriteUnlock(void);
		
  private:
	PthreadMutex		mutex;
	PthreadCondition	lockFree;
	int					numReaders;
	int					writing;
	int					noNewReaders;
	int					priorityWrites;
};


class PthreadReadLockProtected {
  public:
	PthreadReadLockProtected(PthreadReadWriteLock& tl): theReadWriteLock(&tl)
									{ theReadWriteLock->ReadLock(); }
   ~PthreadReadLockProtected(void)	{ theReadWriteLock->ReadUnlock(); }
  private:
	PthreadReadWriteLock* theReadWriteLock;
};


class PthreadWriteLockProtected {
  public:
	PthreadWriteLockProtected(PthreadReadWriteLock& tl): theReadWriteLock(&tl)
									{ theReadWriteLock->WriteLock(); }
   ~PthreadWriteLockProtected(void)	{ theReadWriteLock->WriteUnlock(); }
  private:
	PthreadReadWriteLock* theReadWriteLock;
};


#endif
#endif

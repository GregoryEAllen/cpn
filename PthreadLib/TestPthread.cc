//=============================================================================
//	$Id: s.TestPthread.cc 1.8 03/11/07 17:23:43-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	Pthread testing program
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

#include "Pthread.h"
#include "PthreadAttr.h"
#include "PthreadMutex.h"
#include "PthreadMutexAttr.h"
#ifdef _POSIX_THREADS

#include <stdio.h>


//-----------------------------------------------------------------------------
class MyThread : public Pthread
//-----------------------------------------------------------------------------
{
  public:
	MyThread(char* str, PthreadAttr& attr);
   ~MyThread(void);

	virtual void* EntryPoint(void);

  private:
	char *name;
	int i; // current increment value
};


//-----------------------------------------------------------------------------
MyThread::MyThread(char* str, PthreadAttr& attr)
//-----------------------------------------------------------------------------
:	Pthread(attr), i(-1)
{
	name = str;
	printf("Spawning pThread-%s\n", name);
	Start();
}


//-----------------------------------------------------------------------------
MyThread::~MyThread(void)
//-----------------------------------------------------------------------------
{
	Join();
}


//-----------------------------------------------------------------------------
void* MyThread::EntryPoint(void)
//-----------------------------------------------------------------------------
{
	for (i = 0;  i < 3; i++) {
		printf("%s = %d\n", name, i);
		Yield();
	}
	return (void*)0;
}


//-----------------------------------------------------------------------------
int main(int, char* [])
//-----------------------------------------------------------------------------
{
	printf("### TestPthread executing...\n");
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	PthreadAttr pthreadAttr;
	pthreadAttr.SystemScope();

#ifndef MERCURY
	PthreadBase thisThread;
#endif

	MyThread pThreadA("A", pthreadAttr);
	MyThread pThreadB("B", pthreadAttr);
	MyThread pThreadC("C", pthreadAttr);
	MyThread pThreadD("D", pthreadAttr);
	MyThread pThreadE("E", pthreadAttr);

	for (int i=0; i<20; i++) { 
		// exit loop when all threads are done
		if (pThreadA.Done() &&
			pThreadB.Done() &&
			pThreadC.Done() &&
			pThreadD.Done() &&
			pThreadE.Done())
			break;

		printf("Still going...\n");

		Pthread::Yield();
	}

	pThreadA.Join();
	pThreadB.Join();
	pThreadC.Join();
	pThreadD.Join();
	pThreadE.Join();

	printf("### TestPthread exiting...\n");
	return(0);
}

#else

#include <stdio.h>
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
//-----------------------------------------------------------------------------
{
	printf("### Error: POSIX Pthreads are not implemented on this system.\n");
}


#endif
static const char rcsid[] = "@(#) $Id: s.TestPthread.cc 1.8 03/11/07 17:23:43-06:00 gallen@ph.arlut.utexas.edu $";

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

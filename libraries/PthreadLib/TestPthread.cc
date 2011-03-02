//=============================================================================
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

#include "PthreadAttr.h"
#include "PthreadMutex.h"
#include "PthreadMutexAttr.h"
#include "PthreadFunctional.h"

#ifdef _POSIX_THREADS

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

//-----------------------------------------------------------------------------
class MyCounter {
//-----------------------------------------------------------------------------
  public:
	MyCounter(const char* name_, unsigned count_)
		: name(name_), count(count_) {}
	void* DoCounting(void);
  private:
	std::string name;
	unsigned count;
};

//-----------------------------------------------------------------------------
void* MyCounter::DoCounting(void)
//-----------------------------------------------------------------------------
{
	for (unsigned i = 0;  i<count; i++) {
		printf("%s = %u\n", name.c_str(), i);
		Pthread::Yield();
	}
	return (void*)0;
}


int main (int argc, char* const argv[]) __attribute__((weak));
//-----------------------------------------------------------------------------
int main(int argc, char* const argv[])
//-----------------------------------------------------------------------------
{
	printf("### %s executing...\n", argv[0]);

	unsigned numThreads = 5;
	unsigned count = 3;
	if (argc>1)
		numThreads = atoi(argv[1]);
	if (argc>2)
		count = atoi(argv[2]);

	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	PthreadAttr pthreadAttr;
	pthreadAttr.SystemScope();

	PthreadBase thisThread;

	std::vector<MyCounter*> counters(numThreads);
	std::vector<PthreadFunctional*> threads(numThreads);

	char name[80];
	for (unsigned i=0; i<numThreads; i++) {
		snprintf(name,80,"p%u",i);
		counters[i] = new MyCounter(name,count);
		threads[i] = CreatePthreadFunctional(counters[i], &MyCounter::DoCounting);
		printf("Spawning %s\n", name);
		threads[i]->Start();
	}

	int done = 0;
	for (int loops=0; !done; loops++) {
		done = 1;
		for (unsigned i=0; i<numThreads && done; i++) {
			done &= threads[i]->Done();
		}

		printf("main = %u\n", loops);
		Pthread::Yield();
	}

	printf("Joining\n");
	for (unsigned i=0; i<numThreads; i++) {
		threads[i]->Join();
		delete threads[i];
		delete counters[i];
	}

	printf("### %s exiting...\n", argv[0]);
	return(0);
}

#else

#include <stdio.h>
//-----------------------------------------------------------------------------
int main(int argc, char* const argv[])
//-----------------------------------------------------------------------------
{
	printf("### Error: POSIX Pthreads are not implemented on this system.\n");
}


#endif

//=============================================================================
//	$Id: MirrorBufferSet.cc,v 1.1 2006/06/12 20:21:44 gallen Exp $
//-----------------------------------------------------------------------------
//	A set of "mirrored" buffers
//=============================================================================


#include "MirrorBufferSet.h"

#define MirrorBufferSet_SUPPORTED 1

#include <unistd.h>

#if defined(_POSIX_SHARED_MEMORY_OBJECTS) && !defined(OS_LINUX) && !defined(OS_DARWIN)
	#define MBS_USE_POSIX_SHM 1
	#define _POSIX_C_SOURCE 199309
	#include <string.h>
#else
	#define MBS_USE_POSIX_SHM 0
//	#warning POSIX shared memory objects not supported
#endif

//-----------------------------------------------------------------------------
int MirrorBufferSet::Supported(void)
//-----------------------------------------------------------------------------
{
	#if !MirrorBufferSet_SUPPORTED
		return eNotSupported;
	#endif

	#if MBS_USE_POSIX_SHM
		return eSupportedPosixShm;
	#else
		return eSupportedTmpFile;
	#endif
}


#if MirrorBufferSet_SUPPORTED


#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/mman.h>


#ifndef MAP_FAILED
	// not defined in AIX, HPUX
	#define MAP_FAILED		((void*)-1)
#endif

#ifndef MAP_NORESERVE
	// not defined in AIX, IRIX, LinuxPPC
	#define MAP_NORESERVE	(0)
#endif

// on AIX and HPUX, this compiles, but give a runtime error from mmap()
// on IRIX, this works, although shm_open is not quite POSIX compliant


//-----------------------------------------------------------------------------
//	flags and definitions for debugging printouts
//-----------------------------------------------------------------------------
#ifndef MirrorBufferSet_VERBOSITY
#define MirrorBufferSet_VERBOSITY 0
#endif

#if MirrorBufferSet_VERBOSITY >= 1
    #define fprintf1(p) fprintf p
#else
    #define fprintf1(p)
#endif

#if MirrorBufferSet_VERBOSITY >= 2
    #define fprintf2(p) fprintf p
#else
    #define fprintf2(p)
#endif


//-----------------------------------------------------------------------------
MirrorBufferSet::MirrorBufferSet(ulong bufferSz, ulong mirrorSz, int nBuffers)
//-----------------------------------------------------------------------------
:	bufferBase(0),
	bufferSize(bufferSz),
	mirrorSize(mirrorSz),
	numBuffers(nBuffers)
{
	fileName[0] = 0;
	ulong pageSize = PageSize();

	// if bufferSize or mirrorSize are not a multiple of pageSize, round them up
	if (bufferSize % pageSize) {
		bufferSize += pageSize - (bufferSize % pageSize);
	}
	if (mirrorSize % pageSize) {
		mirrorSize += pageSize - (mirrorSize % pageSize);
	}

#if 1
	// do a sanity check for our math
	if ( bufferSize % pageSize)
		fprintf(stderr, "### Error: bufferSize = %lu is not a multiple of pageSize = %lu\n",
			bufferSize, pageSize);
	if ( mirrorSize % pageSize)
		fprintf(stderr, "### Error: mirrorSize = %lu is not a multiple of pageSize = %lu\n",
			mirrorSize, pageSize);
#endif

	int fd = GetFileDescriptor();

	// make the file large enough
	if ( ftruncate(fd, bufferSize*numBuffers) ) {
		perror("ftruncate");
		close(fd);
		return;
	}

	// map the entire buffer space, so there will be enough space reserved
	ulong numBytes = (bufferSize+mirrorSize) * numBuffers;
	fprintf1((stderr,"MirrorBufferSet: reserving %d bytes for mapping ", numBytes));
	caddr_t baseAddr = (caddr_t) mmap(0, numBytes, PROT_READ|PROT_WRITE, 
			MAP_SHARED|MAP_NORESERVE, fd, 0);
	if (baseAddr == MAP_FAILED) {
		fprintf1((stderr,"failed\n"));
		perror("mmap");
		return;
	}
	bufferBase = baseAddr;
	fprintf1((stderr,"[0x%08X - 0x%08X)\n", baseAddr, baseAddr+numBytes));
	fprintf1((stderr,"MirrorBufferSet: file \"%s\" is %d bytes\n",
		fileName, bufferSize*numBuffers));

	// now map each of the individual buffers twice
	for (int buf=0; buf<numBuffers; buf++) {
		// once for the buffer
		caddr_t theAddr		= baseAddr+buf*(bufferSize+mirrorSize);
		size_t  theSize		= bufferSize;
		size_t  theOffset	= buf*bufferSize;
		fprintf2((stderr,"mapping 0x%06lX bytes @ offset 0x%06lX ", theSize, theOffset));
		caddr_t addr = (caddr_t) mmap(theAddr, theSize, PROT_READ|PROT_WRITE,
			MAP_SHARED|MAP_FIXED|MAP_NORESERVE, fd, theOffset);
		if (addr == MAP_FAILED) {
			fprintf2((stderr,"failed\n"));
			perror("mmap");
			return;
		}
		fprintf2((stderr,"to [0x%08X - 0x%08X)\n", theAddr, theAddr+theSize));
		if (addr!=theAddr) {
			fprintf(stderr, "### Error: mapped to 0x%p instead of 0x%p\n",
				addr, theAddr );
			return;
		}

		// and again for the mirror
		theAddr		+=bufferSize;
		theSize		= mirrorSize;
		fprintf2((stderr,"mapping 0x%06lX bytes @ offset 0x%06lX ", theSize, theOffset));
		addr = (caddr_t) mmap(theAddr, theSize, PROT_READ|PROT_WRITE,
			MAP_SHARED|MAP_FIXED|MAP_NORESERVE, fd, theOffset);
		if (addr == MAP_FAILED) {
			fprintf2((stderr,"failed\n"));
			perror("mmap");
			return;
		}
		fprintf2((stderr,"to [0x%08X - 0x%08X)\n", theAddr, theAddr+theSize));
		if (addr!=theAddr) {
			fprintf(stderr, "### Error: mapped to 0x%p instead of 0x%p\n", addr, theAddr );
			return;
		}
	}

	// close the file
	if ( close(fd) ) {
		perror("close");
	}
}


//-----------------------------------------------------------------------------
int MirrorBufferSet::GetFileDescriptor(void)
//-----------------------------------------------------------------------------
{
	// get a temporary file name
#ifdef OS_LINUX
	sprintf(fileName, "/tmp/MirrorBufferSet-XXXXXX");
	return mkstemp(fileName);
#endif
	tmpnam(fileName);
	fprintf2((stderr,"tmpnam returned \"%s\"\n",fileName));
#if MBS_USE_POSIX_SHM
	#if !defined(OS_IRIX) && !defined(OS_HPUX)
		// these platforms don't do shm_open as defined in the spec 
		char* base = strrchr(fileName,'/');
		assert(base);
		strcpy(fileName,base);
	#endif
	fprintf2((stderr,"calling shm_open with \"%s\"\n",fileName));
	int fd = shm_open(fileName,O_RDWR|O_CREAT|O_EXCL,0600);
	if (fd < 0) {
		fprintf(stderr, "shm_open \"%s\": ", fileName);
		perror(0);
	}
#else
	int fd = open(fileName, O_RDWR|O_CREAT, 0666);
	if(fd < 0) {
		fprintf(stderr, "### Error opening file \"%s\": ", fileName);
		perror(0);
	}
#endif
	return fd;
}


//-----------------------------------------------------------------------------
MirrorBufferSet::~MirrorBufferSet(void)
//-----------------------------------------------------------------------------
{
	if (!bufferBase) return;

	// unmap the file
	if ( munmap((caddr_t)bufferBase, (bufferSize+mirrorSize)*numBuffers) ) {
		perror("munmap");
	}
	fprintf2((stderr,"unmapped- \"%s\" [0x%08X - 0x%08X), %d bytes\n",
		fileName, bufferBase,
        (char*)bufferBase+((bufferSize+mirrorSize)*numBuffers),
        ((bufferSize+mirrorSize)*numBuffers)));
	
#if MBS_USE_POSIX_SHM
	// remove the shared memory object
	if ( shm_unlink(fileName) ) {
		fprintf(stderr, "### shm_unlink(\"%s\") failed: ", fileName);
		perror(0);
	}
#else
	// remove the temporary file
	if ( unlink(fileName) ) {
		fprintf(stderr, "### unlink(\"%s\") failed: ", fileName);
		perror(0);
	}
#endif
}


//-----------------------------------------------------------------------------
unsigned long MirrorBufferSet::PageSize(void)
//-----------------------------------------------------------------------------
{
#ifdef OS_DARWIN
	return getpagesize();
#else
	return sysconf(_SC_PAGESIZE);
#endif
}


#endif


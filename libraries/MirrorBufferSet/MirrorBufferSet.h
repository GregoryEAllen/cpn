//=============================================================================
//	$Id: MirrorBufferSet.h,v 1.1 2006/06/12 20:21:44 gallen Exp $
//-----------------------------------------------------------------------------
//	A set of "mirrored" buffers
//=============================================================================


#ifndef MirrorBufferSet_h
#define MirrorBufferSet_h

//-----------------------------------------------------------------------------
//	Implementation notes:
//
//	+------------------+------------+------------------+------------+
//	|    bufferSize    | mirrorSize |    bufferSize    | mirrorSize | ...
//	+------------------+------------+------------------+------------+
//
//	Each buffer has its first mirrorSize bytes mirrored by the VM system.
//	There are numBuffers contiguous buffers.
//	bufferSize and mirrorSize are multiples of the VM page size.
//	mirrorSize <= bufferSize
//-----------------------------------------------------------------------------


class MirrorBufferSet {
  public:
	typedef unsigned long ulong;

	MirrorBufferSet(ulong bufferSz, ulong mirrorSz, int nBuffers = 1);
   ~MirrorBufferSet(void);

	ulong BufferSize(void) const { return bufferSize; }
	ulong MirrorSize(void) const { return mirrorSize; }
	ulong NumBuffers(void) const { return numBuffers; }

	enum { eNotSupported=0, eSupportedPosixShm, eSupportedTmpFile };

	static int Supported(void);
	static ulong PageSize(void);

	operator void* (void) const { return bufferBase; }

  protected:
	int GetFileDescriptor(void);

	void*	bufferBase;
	ulong	bufferSize;
	ulong	mirrorSize;
	int		numBuffers;
	char	fileName[112];
};


#endif

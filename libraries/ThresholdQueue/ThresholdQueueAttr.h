//=============================================================================
//	$Id: ThresholdQueue.h,v 1.1 2006/06/12 20:30:13 gallen Exp $
//-----------------------------------------------------------------------------
//	Attribute class for ThresholdQueue
//=============================================================================


#ifndef ThresholdQueueAttr_h
#define ThresholdQueueAttr_h

class ThresholdQueueBase;

class ThresholdQueueAttr {
  public:
	typedef unsigned long ulong;

	ThresholdQueueAttr(ulong queueLen, ulong maxThresh, ulong numChans=1,
		bool useMBS_=1, ulong chanOffst=0, ulong baseOffst=0)
		: queueLength(queueLen), maxThreshold(maxThresh), numChannels(numChans),
			useMBS(useMBS_), chanOffset(chanOffst), baseOffset(baseOffst) {}

	ulong	QueueLength(void) const			{ return queueLength; }
	ulong	QueueLength(ulong queueLen)		{ return queueLength = queueLen; }
	ulong	MaxThreshold(void) const		{ return maxThreshold; }
	ulong	MaxThreshold(ulong maxThresh)	{ return maxThreshold = maxThresh; }
	ulong	NumChannels(void) const			{ return numChannels; }
	ulong	NumChannels(ulong numChans)		{ return numChannels = numChans; }

	void	UseMBS(bool useMBS_, ulong chanOffst=0, ulong baseOffst=0)
		{ useMBS=useMBS_; chanOffset=chanOffst; baseOffset=baseOffst; }

  protected:
	ulong	queueLength;
	ulong	maxThreshold;
	ulong	numChannels;
	bool	useMBS;
	ulong	chanOffset;
	ulong	baseOffset;
	
	friend class ThresholdQueueBase;
};

#endif

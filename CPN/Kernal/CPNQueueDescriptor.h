//=============================================================================
//	$Id: CPNQueueDescriptor.h 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// CPNQueueDescriptor
//=============================================================================


#ifndef CPNQueueDescriptor_h
#define CPNQueueDescriptor_h

#include "CPNDefs.h"
#include <string>

class CPNQueueDescriptor{
	public:
		// Constructor & Destructor
		CPNQueueDescriptor(const std::string &theQueueName, const std::string &node1, 
			const std::string &node1dir, const std::string &node2, const std::string &node2dir, 
			const TypeID &theTypeID, const int &elemSize=NaN, const int &len=NaN, const int &maxThresh=NaN);
		CPNQueueDescriptor(const CPNQueueDescriptor &toCopy);
		~CPNQueueDescriptor();

		// Update methods
		int SetElementSize(const int &val)	{ elementSize = val; }
		int SetLength(const int &val)		{ length = val; }
		int SetThreshold(const int &val)	{ maxThreshold = val; }

		// Accessor methods
		const std::string GetInputNode()  const;
		const std::string GetOutputNode() const; 
		const std::string GetName()	const { return queueName; }
		const TypeID GetType() 		const { return typeID; }
		const int GetElementSize()	const { return elementSize; }
		const int GetLength() 		const { return length; }
		const int GetThreshold() 	const { return maxThreshold; }
		
		// Debugging aides
		void Print(void);
		void Debug(const std::string &msg);	

	private:
		// Data members
		std::string queueName, node1, node1dir, node2, node2dir;
		TypeID typeID;
		int elementSize, length, maxThreshold;
};

#endif

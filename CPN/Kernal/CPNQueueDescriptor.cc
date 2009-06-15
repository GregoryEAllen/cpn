//=============================================================================
//	$Id: CPNQueueDescriptor.cc 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// CPNQueueDescriptor
//=============================================================================


#include "CPNQueueDescriptor.h"
#include <string>


//-----------------------------------------------------------------------------
CPNQueueDescriptor::CPNQueueDescriptor(const std::string &theQueueName, const std::string &node1, 
		const std::string &node1dir, const std::string &node2, const std::string &node2dir, 
		const TypeID &theTypeID, const int &elemSize, const int &len, const int &maxThresh)
//-----------------------------------------------------------------------------
:	queueName(theQueueName),
	node1(node1),
	node1dir(node1dir),
	node2(node2),
	node2dir(node2dir),
	typeID(theTypeID),
	elementSize(elemSize),
	length(len),
	maxThreshold(maxThresh)
{
	Debug("Constructor called for "+ queueName);
}


//-----------------------------------------------------------------------------
CPNQueueDescriptor::CPNQueueDescriptor(const CPNQueueDescriptor &toCopy)
//-----------------------------------------------------------------------------
:	queueName(toCopy.queueName),
	node1(toCopy.node1),
	node1dir(toCopy.node1dir),
	node2(toCopy.node2),
	node2dir(toCopy.node2dir),
	typeID(toCopy.typeID),
	elementSize(toCopy.elementSize),
	length(toCopy.length),
	maxThreshold(toCopy.maxThreshold)
{
	Debug("Copy called for "+ queueName);
}


//-----------------------------------------------------------------------------
const std::string CPNQueueDescriptor::GetInputNode() const
//-----------------------------------------------------------------------------
{
	if(node1dir=="in") 
		return node1; 
	else 
		return node2; 
}


//-----------------------------------------------------------------------------
const std::string CPNQueueDescriptor::GetOutputNode() const 
//-----------------------------------------------------------------------------
{ 
	if(node1dir=="out")
		return node1; 
	else 
		return node2; 
}


//-----------------------------------------------------------------------------
CPNQueueDescriptor::~CPNQueueDescriptor()
//-----------------------------------------------------------------------------
{
	Debug("Destructor called for "+ queueName);
}



//-----------------------------------------------------------------------------
void CPNQueueDescriptor::Print(void) 
//-----------------------------------------------------------------------------
{
	printf("queueName='%s' node1='%s'<%s> node2='%s'<%s> Type=%d elemSize=%d len=%d maxT=%d\n",
		queueName.c_str(), node1.c_str(), node1dir.c_str(), node2.c_str(), node2dir.c_str(), typeID, 
		elementSize, length, maxThreshold);
}


//-----------------------------------------------------------------------------
void CPNQueueDescriptor::Debug(const std::string &msg) 
//-----------------------------------------------------------------------------
{ 
	if(DEBUG) printf("CPNQueueDescriptor: ### %s\n", msg.c_str()); 
}

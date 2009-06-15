//=============================================================================
//	$Id: CPNNodeDescriptor.cc 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// CPNNodeDescriptor
//=============================================================================


#include "CPNNodeDescriptor.h"


//-----------------------------------------------------------------------------
CPNNodeDescriptor::CPNNodeDescriptor(const std::string &theNodeName, const std::string &theTypeName, 
	const HostID &theHostID, const std::string &theInQ, const std::string &theOutQ)
//-----------------------------------------------------------------------------
:	nodeName(theNodeName),
	typeName(theTypeName),
	hostID(theHostID),
	inQ(theInQ),
	outQ(theOutQ)
{
	Debug("Constructor called for "+ nodeName);
}


//-----------------------------------------------------------------------------
CPNNodeDescriptor::CPNNodeDescriptor(const CPNNodeDescriptor &toCopy)
//-----------------------------------------------------------------------------
:	nodeName(toCopy.nodeName),
	typeName(toCopy.typeName),
	hostID(toCopy.hostID),
	inQ(toCopy.inQ),
	outQ(toCopy.outQ)
{
	Debug("Copy called for "+ toCopy.nodeName);
}


//-----------------------------------------------------------------------------
CPNNodeDescriptor::~CPNNodeDescriptor() 
//-----------------------------------------------------------------------------
{
	Debug("Destructor called for " + nodeName);
}		


//-----------------------------------------------------------------------------
void CPNNodeDescriptor::Print(void) 
//-----------------------------------------------------------------------------
{
	printf("nodeName='%s' typeName='%s' hostID=%d inQ='%s' outQ='%s'\n", 
		nodeName.c_str(), typeName.c_str(), hostID, inQ.c_str(), outQ.c_str()); 
}


//-----------------------------------------------------------------------------
void CPNNodeDescriptor::Debug(const std::string &msg) 
//-----------------------------------------------------------------------------
{ 
	if(DEBUG) 
		printf("CPNNodeDescriptor: ### %s\n", msg.c_str()); 
}

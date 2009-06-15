//=============================================================================
//	$Id: CPNKernel.cc 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// A class that constructs a CPN network.  The CPNkernel runs on every SMP box 
//	in the distributed environment and handles the connections between a 
//	node's input and output & queues.
//=============================================================================


#include "CPNKernel.h"


//-----------------------------------------------------------------------------
CPNKernel::CPNKernel(const std::string &myfilename, const HostID &myID)
//-----------------------------------------------------------------------------
:	filename(myfilename),
	hostID(myID) 
{
	/*
	if(filename!=EMPTY) 
		CPNInitializeFromFile();
	*/ 
}


//-----------------------------------------------------------------------------
CPNKernel::~CPNKernel(void)
//-----------------------------------------------------------------------------
{
	Debug("Destructor called");
}


//-----------------------------------------------------------------------------
int CPNKernel::CreateCPNKernelFromFile(const std::string &filename)
// Build a CPNKernel from an XML file
//----------------------------------------------------------------------------
{
	Debug("Constructing Kernel from file:" +filename);
}


//-----------------------------------------------------------------------------	
int CPNKernel::Execute(void)
// runs the CPNKernel
//-----------------------------------------------------------------------------
{
/*
	- spawn command & control thread
		- Contacts hosts (other necessary nodes) needed by the nodes assigned to this kernel
	- if communication to other hosts is successful 
	- spawn a thread for each node
		- establish communication channels (queues)
		
	If this host has a HostID of 0: {All hosts know which host they are and where and who the other hosts are}
		- send a coordinated "start" signal to everyone and wait for acknowledgement
		{Can we assume errorless channels? data not lost? - TCP (FIFO)? }
		- once acknowledgement received - the other hosts should have initialized their nodes and queues
		- Tell nodes and queues to begin processing
		- handle any errors that may occur & make the decision if the error is fatal
*/
}


//-----------------------------------------------------------------------------
void CPNKernel::DisplayResults(void)
//
//-----------------------------------------------------------------------------
{}


//-----------------------------------------------------------------------------
int CPNKernel::ContactHosts(void)
//
//-----------------------------------------------------------------------------
{}


//-----------------------------------------------------------------------------
int CPNKernel::AddHost(const CPNHost &theHost)
// Adds host information to the CPNKernel
//-----------------------------------------------------------------------------
{
	HostMapInsertResult theResult = hostMap.insert(HostMapEntry(theHost.GetHostID(), theHost));
	return InsertCheck(theResult.second, theHost.GetHostName());
}


//-----------------------------------------------------------------------------
int CPNKernel::AddNode (const CPNNodeDescriptor &theNode)
// Adds an already created node to the CPNKernel
//-----------------------------------------------------------------------------
{
	NodeMapInsertResult theResult = nodeMap.insert(NodeMapEntry(theNode.GetNodeName(), theNode));
	return InsertCheck(theResult.second, theNode.GetNodeName());
}			


//-----------------------------------------------------------------------------
int CPNKernel::AddNode (const std::string &nodeName, const std::string &displayName, const HostID &hostID)
// Adds a user defined node to the CPNKernel
//-----------------------------------------------------------------------------
{
	NodeMapInsertResult theResult = nodeMap.insert(NodeMapEntry(nodeName, CPNNodeDescriptor(nodeName, displayName, hostID)));
	return InsertCheck(theResult.second, nodeName);	
}


/*
//-----------------------------------------------------------------------------
int CPNKernel::AddQueue(const CPNQueueDescriptor &theQueue)
// Not yet implemented
//-----------------------------------------------------------------------------
{
	
}
*/


//-----------------------------------------------------------------------------
int CPNKernel::AddQueue(const std::string &queueName, const std::string &node1, const std::string &node1dir, 
		const std::string &node2, const std::string &node2dir, const enum TypeID &theType)
// Adds a user defined queue to the CPNKernel
//-----------------------------------------------------------------------------
{
	if (node1==node2)	// Can a node have a queue (loop) onto itself?
		return FAILURE; 
	
	// try to update the InputNode's information
	NodeMap::iterator node1Itr = nodeMap.find(node1);
	if(node1Itr != nodeMap.end()) {//-- we have seen this node before
		if(node1dir=="out")
			node1Itr->second.SetOutputQ(queueName);
		else
			node1Itr->second.SetInputQ(queueName);
	}

	// try to update the OutputNode's information
	NodeMap::iterator node2Itr = nodeMap.find(node2);
	if(node2Itr != nodeMap.end()) {//-- we have seen this node before
		if(node2dir=="out")
			node2Itr->second.SetOutputQ(queueName);
		else
			node2Itr->second.SetInputQ(queueName);
	}

	// Add the queue to the list of queues the Kernel knows about
	QueueMapEntry theEntry(queueName, CPNQueueDescriptor(queueName, node1, node1dir, node2, node2dir, theType));
	QueueMapInsertResult theResult = queueMap.insert(theEntry);
	return InsertCheck(theResult.second, queueName);		
}


/*
//-----------------------------------------------------------------------------
int CPNKernel::GetCPNNodeInfo(const CPNNode &theNode)
//-----------------------------------------------------------------------------
{}

//-----------------------------------------------------------------------------
NodeID CPNKernel::GetCPNNodeID(const CPNNode &theNode)
//-----------------------------------------------------------------------------
{
	//theNode.GetNodeName();
	//nodeVector[theNodeID]=CPNNode;
	//Should CPNNodeDescriptor be the same class as CPNNode? - Don't want two copies of the node floating around...
}
//-----------------------------------------------------------------------------
CPNNodeInput* CPNKernel::GetCPNNodeInput(const NodeID &theID)
//-----------------------------------------------------------------------------
{
	// lookup NodeID from host?
	//queueMap[]
	//return queueMap[theID].second.GetInputPtr();  //?
	//std::string Qname = nodeQueueMap[theID].first; // name of InputQ
}
//-----------------------------------------------------------------------------
CPNNodeOutput* CPNKernel::GetCPNNodeOutput(const NodeID &theID)
//-----------------------------------------------------------------------------
{
	//return queueMap[theID].second.GetOutputPtr();  //?
	//std::string Qname = nodeQueueMap[theID].second; // name of OutputQ
	//QueueMap[Qname].first
}
*/


//-----------------------------------------------------------------------------
void CPNKernel::Debug(const std::string &msg)
//-----------------------------------------------------------------------------
{ 
	if(DEBUG) 
		printf("CPNKernel: ### %s\n", msg.c_str()); 
}


//-----------------------------------------------------------------------------
void CPNKernel::PrintKernelState(void)
// Prints the state of the internal lists held by the kernel
//-----------------------------------------------------------------------------
{
	printf("\n---KernelState---\n");

	printf("\tNodeMap\n\t\tSize: %d\n\t\tElements:", nodeMap.size() );
	printf("\n\t\t%8s\t--->%8s\t--->%8s", "inQ", "node", "outQ");
	for(NodeMap::const_iterator itr = nodeMap.begin(); itr!=nodeMap.end(); itr++){
		printf("\n\t\t%8s\t--->%8s\t--->%8s", (itr->second).GetInQ().c_str(),
		(itr->second).GetNodeName().c_str(), (itr->second).GetOutQ().c_str());
	}

	printf("\n\tQueueMap\n\t\tSize: %d\n\t\tElements:\t", queueMap.size() );
	for(QueueMap::const_iterator itr = queueMap.begin(); itr!=queueMap.end(); itr++){
		printf("%s\t", (itr->second).GetName().c_str());
	}

	printf("\n\tHostMap\n\t\tSize: %d\n\t\tElements:\t", hostMap.size() );
	for(HostMap::const_iterator itr = hostMap.begin(); itr!=hostMap.end(); itr++){
		printf("%s\t", (itr->second).GetHostName().c_str());
		//printf("%s\t", (itr->second).GetHostName().c_str());
	}
	printf("\n-----------------\n");
}


//-----------------------------------------------------------------------------
int CPNKernel::InsertCheck(bool const &result, const std::string &name){
// Utility Function
//-----------------------------------------------------------------------------
	if(result == true){
		Debug("Inserted element " + name);
		return SUCCESS;
	}	else	{
		Debug("Insert failed");
		return FAILURE;
	}
}

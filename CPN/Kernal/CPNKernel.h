//=============================================================================
//	$Id: CPNKernel.h 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// A class that constructs a CPN network.  The CPNkernel runs on every SMP box 
//	in the distributed environment and handles the connections between a 
//	node's input and output & queues.
//=============================================================================


#ifndef CPNKernel_h
#define CPNKernel_h


#include "CPNDefs.h"
#include "CPNHost.h"
#include "CPNNodeDescriptor.h"
#include "CPNQueueDescriptor.h"
#include <utility>
#include <string>
#include <map>


class CPNKernel {
	public:
		typedef std::map <HostID, CPNHost> HostMap;
		typedef std::pair <HostID, CPNHost> HostMapEntry;
		typedef std::pair <HostMap::iterator, bool> HostMapInsertResult;

		typedef std::map <std::string, CPNNodeDescriptor> NodeMap;
		typedef std::pair <std::string, CPNNodeDescriptor> NodeMapEntry;
		typedef std::pair <NodeMap::iterator, bool> NodeMapInsertResult;

		typedef std::pair <std::string, CPNQueueDescriptor> QueueMapEntry;
		typedef std::map<std::string, CPNQueueDescriptor> QueueMap;
		typedef std::pair <QueueMap::iterator, bool> QueueMapInsertResult;

		CPNKernel(const std::string &myfilename=EMPTY, const HostID &myID=NaN);
		~CPNKernel(void);
			
		// Control Functions
		int Execute(void); // runs the CPNKernel
		void DisplayResults(void); // benchmarking?

		// Host Stuff
		int ContactHosts(void);
		int AddHost(const CPNHost &theHost);

		// Node & Queue Functions
		int AddNode(const CPNNodeDescriptor &theNode);
		int AddNode(const std::string &nodeName, const std::string &displayName, const HostID &hostID);
		int AddQueue(const CPNQueueDescriptor &theQueue);
		int AddQueue(const std::string &queueName, const std::string &node1, const std::string &node1dir, 
			const std::string &node2, const std::string &node2dir, const enum TypeID &theType);
		
		// Input & Output for Nodes - connection step for a Node
		/*
		int GetCPNNodeInfo(const CPNNode &theNode);
		NodeID GetCPNNodeID(const CPNNode &theNode);
		CPNNodeInput* GetCPNNodeInput(const NodeID &theID);
		CPNNodeOutput* GetCPNNodeOutput(const NodeID &theID);
		*/

		// Is it possible to dynamically remove Hosts, NodeDescriptors, or Queues from the program?
		int DeleteHost(CPNHost &theHost); //?
		int DeleteNode(CPNNodeDescriptor &theNodeDescriptor); //?
		int DeleteQueue(CPNQueueDescriptor &theQueue); //?

		// Debugging aides
		void Debug(const std::string &msg);
		void PrintKernelState(void);
		
	private:
		// Creation/ Parser stuff - should this live oustide & CPNKernel inherit from that?
		int CreateCPNKernelFromFile(const std::string &filename);
		// Disable copying ?
		CPNKernel(const CPNKernel &theCopyCPNK){}
		
		// Private Member Utility Functions
		int InsertCheck(bool const &result, const std::string &name);
			
		// Data members
		std::string filename;
		HostID hostID;
		NodeMap nodeMap; // list of NodeDescriptors based on name?
		// NOTE-should NodeMap be dynamically allocated so tons of copies of CPNNodeDescriptors aren't floating around ?
		QueueMap queueMap; // a list of all the queues in the CPN Kernel
		HostMap hostMap; // host information 
		// this hostMap should be the same for all processors that are running CPNKernels on the network?
		
		// implementationMap?
};


#endif

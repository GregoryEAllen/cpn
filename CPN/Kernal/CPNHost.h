//=============================================================================
//	$Id: CPNHost.h 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// CPNHost
//=============================================================================


#ifndef CPNHost_h
#define CPNHost_h

#include "CPNDefs.h"
#include <string>


class CPNHost {
	public:
		CPNHost(const std::string &myName=EMPTY, const HostID &myID=NaN) ;
		CPNHost(const CPNHost &theCopy) ;
		~CPNHost(void);
		
		const HostID GetHostID(void) const { return hostID; }
		const std::string GetHostName(void) const { return hostName; }
		const NodeID GetStartNode(void) const { return startNode; }

		void Print();
		void Debug(const std::string &msg);

	private:
		std::string hostName;
		HostID hostID;		
		NodeID startNode;
		std::string IPAddr; //?
		int CtrlPort, numProc, maxNodes; //?
		/* Connection Information - 			
			? Should this be a class by itself ?
			? Mutable/ changeable based on connection type ?
			- How are we connected:
				- Connection Media	- IPaddress	- TCP communication port or control port
				- Any other relevant information
			Block of NodeIDs
			start = ((hostID*NODEBLOCKLEN)+1);   // and length known at startup?
			**assuming that the hostID is unique for each host...
			Could also store an array with references to the nodes on the host?
		*/
};


#endif

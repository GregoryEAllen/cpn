//=============================================================================
//	$Id: CPNKernelTester.cc 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// Initial test routine for the CPNKernel
//=============================================================================


//-----------------------------------------------------------------------------
// CPN includes
//-----------------------------------------------------------------------------
#include "CPNDefs.h"
#include "CPNKernel.h"


//-----------------------------------------------------------------------------
// STL includes
//-----------------------------------------------------------------------------
#include <utility>
#include <map>
#include <string>


//-----------------------------------------------------------------------------
void Debug(std::string msg){ printf("main: ### %s\n", msg.c_str()); }
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
int main(int argc, char *argv[] )
//-----------------------------------------------------------------------------
{
#if 1
	CPNKernel mycpnk;

	mycpnk.AddHost(CPNHost("localhost", 1));

	mycpnk.AddNode("A", "myProducerNode", 1);
	mycpnk.AddNode("B", "myTransmuterNode", 2);
	mycpnk.AddNode("C", "myConsumerNode", 3);
	mycpnk.AddNode("D", "myInspectorNode", 4);

	mycpnk.AddQueue("P", "A", "out", "B", "in", BOOL);
	mycpnk.AddQueue("Q", "B", "out", "C", "in", BOOL);
	mycpnk.AddQueue("R", "C", "out", "D", "in", BOOL);

	mycpnk.PrintKernelState();	

	mycpnk.Execute();	// runs until completion ?
	mycpnk.DisplayResults(); // benchmarking?
	mycpnk.Debug("Done");

#endif


#if 0
	// std::map a NodeID to it's input and output queue
	typedef std::map<NodeID, std::pair<std::string, std::string> > NodeQueueMap;
	typedef std::pair<std::string, std::string> StringPair;
	NodeQueueMap nodeQMap;
	nodeQMap[0] = StringPair("P", "Q");
	nodeQMap[1] = StringPair("Q", "W");
	nodeQMap[2] = StringPair("W", "T");
	for(int i=0; i<nodeQMap.size(); i++){
		printf("first: %s\nsecond:%s\n", nodeQMap[i].first.c_str(), nodeQMap[i].second.c_str());
	}	
#endif


	return 0;
}

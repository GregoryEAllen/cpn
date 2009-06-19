/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"
#include "QueueReader.h"
#include "QueueWriter.h"
#include "NodeBase.h"
#include "QueueBase.h"


using namespace CPN;

Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr) {}

Kernel::~Kernel() {}


void Kernel::Wait(void) {}

void RegisterNodeType(const ::std::string &nodetype,
		NodeFactory_t factory,
		NodeDestructor_t destructor) {
	// !!! Add error checking
	factoryMap[nodetype] = :std::pair<NodeFactory_t, NodeDestructor_t>(factory, destructor);
}

void CreateNode(const ::std::string &nodename,
		const ::std::string &nodetype,
		const void* const arg) {
	// !!! Add error checking
	//Generate the node attribute
	NodeInfo* nodeinfo = new NodeInfo(nattr, factoryMap[nodetype], arg);
	// Have NodeInfo manage the actual construction and destruction
	// of the node.
	nodeMap[nodename] = nodeinfo;
}

void CreateQueue(const ::std::string &queuename,
		const ::std::string &queuetype) {
	// One one for now
	// !!! Add erro checking and ability to have new queue types
	queueMap[queuename] = new ThresholdQueue();
}

void ConnectWriteEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
}

void ConnectReadEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
}

QueueReader* GetReader(const ::std::string &nodename,
		const ::std::string &portname) {
}

QueueWriter* GetWriter(const ::std::string &nodename,
		const ::std::string &portname) {
}

void NodeTerminated(const NodeAttr &attr) {
}



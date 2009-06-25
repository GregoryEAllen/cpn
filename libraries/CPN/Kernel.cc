/** \file
 * \brief Implementation for kernel functions
 */

#include "Kernel.h"
#include "QueueReader.h"
#include "QueueWriter.h"
#include "NodeQueueReader.h"
#include "NodeQueueWriter.h"
#include "NodeBase.h"
#include "QueueBase.h"
#include "NodeInfo.h"
#include "QueueInfo.h"
#include <algorithm>
#include <cassert>

template<class thetype>
class Deleter {
public:
	void operator() (std::pair<std::string, thetype*> o) {
		if (o.second) {
			delete o.second;
		}
	}
};

void NodeStarter(std::pair<std::string, CPN::NodeInfo*> o) {
	if (o.second) {
		o.second->GetNode()->Start();
	}
}

CPN::Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr), idcounter(0), started(false) {}

CPN::Kernel::~Kernel() {
	PthreadMutexProtected plock(lock); 
	for_each(nodeMap.begin(), nodeMap.end(), Deleter<CPN::NodeInfo>());
	nodeMap.clear();
	for_each(queueMap.begin(), queueMap.end(), Deleter<CPN::QueueInfo>());
	queueMap.clear();
}

void CPN::Kernel::Start(void) {
	PthreadMutexProtected plock(lock); 
	started = true;
	for_each(nodeMap.begin(), nodeMap.end(), NodeStarter);
}

void CPN::Kernel::Wait(void) {}

void CPN::Kernel::CreateNode(const ::std::string &nodename,
		const ::std::string &nodetype,
		const void* const arg,
		const ulong argsize) {
	PthreadMutexProtected plock(lock); 
	// Verify that nodename doesn't already exist.
	if (nodeMap[nodename]) return;
	// Create the NodeAttr object.
	CPN::NodeAttr attr(GenerateId(nodename), nodename, nodetype);
	// Create the NodeInfo structure (which creates the node)
	CPN::NodeInfo* nodeinfo = new CPN::NodeInfo(*this, attr, arg, argsize);	
	assert(nodeinfo);
	// Put the NodeInfo into our map.
	nodeMap[nodename] = nodeinfo;
	if (started) nodeinfo->GetNode()->Start();
}

void CPN::Kernel::CreateQueue(const ::std::string &queuename,
		const ::std::string &queuetype,
		const ulong queueLength,
		const ulong maxThreshold,
		const ulong numChannels) {
	PthreadMutexProtected plock(lock); 
	// Verify that queuename doesn't already exist.
	if (queueMap[queuename]) return;
	// Generate the QueueAttr object.
	CPN::QueueAttr attr(GenerateId(queuename), queuename, queuetype,
		       	queueLength, maxThreshold, numChannels);
	// Create the QueueInfo which creates the queue
	CPN::QueueInfo* queueinfo = new CPN::QueueInfo(attr);
	assert(queueinfo);
	// Put QueueInfo in our map.
	queueMap[queuename] = queueinfo;
}

void CPN::Kernel::ConnectWriteEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate that qname and nodename exist
	// Lookup the queue
	QueueInfo* qinfo = queueMap[qname];
	NodeInfo* ninfo = nodeMap[nodename];
	assert(qinfo);
	assert(ninfo);
	// Unregister the write end of the queue from it's registered
	// place if it is already registered.
	// Register the write end with the new place.
	ninfo->SetWriter(qinfo, portname);
}

void CPN::Kernel::ConnectReadEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate qname and nodename.
	// Look up the queue
	QueueInfo* qinfo = queueMap[qname];
	NodeInfo* ninfo = nodeMap[nodename];
	assert(qinfo);
	assert(ninfo);
	// Unregister the queue from its read port if applicable.
	// Register the read port with its new port.
	ninfo->SetReader(qinfo, portname);
}

CPN::QueueReader* CPN::Kernel::GetReader(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate nodename
	// Lookup nodeinfo
	NodeInfo* ninfo = nodeMap[nodename];
	assert(ninfo);
	// Check if reader exists.
	// If not create new reader and add it.
	// return the reader.
	QueueReader* qreader = ninfo->GetReader(portname);
	assert(qreader);
	return qreader;
}

CPN::QueueWriter* CPN::Kernel::GetWriter(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	// Validate nodename
	// lookup nodeinfo.
	NodeInfo* ninfo = nodeMap[nodename];
	assert(ninfo);
	// check if writer exists.
	// if not create new writer and add it.
	// return the writer.
	QueueWriter* qwriter = ninfo->GetWriter(portname);
	assert(qwriter);
	return qwriter;
}

/*
void CPN::Kernel::NodeTerminated(const NodeAttr &attr) {
	PthreadMutexProtected plock(lock); 
	// Lookup the nodeinfo
	NodeInfo* ninfo = nodeMap[attr.GetName()];
	assert(ninfo);
	nodeMap.erase(attr.GetName());
	// Unregister all ports.
	// delete the nodeinfo object.
	delete ninfo;
	ninfo = 0;
}
*/

CPN::ulong CPN::Kernel::GenerateId(const ::std::string& name) {
	return idcounter++;
}


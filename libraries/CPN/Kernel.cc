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
#include "KernelShutdownException.h"
#include "MapDeleter.h"
#include "MapInvoke.h"
#include <algorithm>
#include <cassert>
#include <vector>
#include <stdexcept>

CPN::Kernel::Kernel(const KernelAttr &kattr)
	: kattr(kattr), idcounter(0), status(INITIALIZED) {}

CPN::Kernel::~Kernel() {
	Terminate();
	Wait();
	std::vector<std::pair<std::string, CPN::NodeInfo*> > nodelist;
	std::vector<std::pair<std::string, CPN::QueueInfo*> > queuelist;

	{
		PthreadMutexProtected plock(lock); 
		status = SHUTTINGDOWN;
		nodelist.assign(nodeMap.begin(), nodeMap.end());
		nodeMap.clear();
		queuelist.assign(queueMap.begin(), queueMap.end());
		queueMap.clear();
	}
	for_each(nodelist.begin(), nodelist.end(), MapDeleter<std::string, CPN::NodeInfo>());
	for_each(queuelist.begin(), queuelist.end(), MapDeleter<std::string, CPN::QueueInfo>());
}

void CPN::Kernel::Start(void) {
	PthreadMutexProtected plock(lock); 
	if (status != INITIALIZED) return;
	status = STARTED;
	for_each(nodeMap.begin(), nodeMap.end(),
		       	MapInvoke<std::string, CPN::NodeInfo, void(CPN::NodeInfo::*)(void)>(
				&CPN::NodeInfo::Start));
}

void CPN::Kernel::Wait(void) {
	PthreadMutexProtected plock(lock); 
	if (status == INITIALIZED) return;
	while (nodeMap.size()) {
		nodeTermination.Wait(lock);
		while (nodesToDelete.size()) {
			NodeInfo* nodeinfo = nodesToDelete.front();
			nodesToDelete.pop_front();
			delete nodeinfo;
		}
	}
	status = STOPPED;
}

void CPN::Kernel::Terminate(void) {
	PthreadMutexProtected plock(lock); 
	if (status == INITIALIZED) return;
	for_each(nodeMap.begin(), nodeMap.end(),
		       	MapInvoke<std::string, CPN::NodeInfo, void(CPN::NodeInfo::*)(void)>(
				&CPN::NodeInfo::Terminate));
	status = STOPPED;
}

void CPN::Kernel::CreateNode(const ::std::string &nodename,
		const ::std::string &nodetype,
		const void* const arg,
		const ulong argsize) {
	PthreadMutexProtected plock(lock); 
	if (status == SHUTTINGDOWN || status == STOPPED) {
		throw CPN::KernelShutdownException("Cannot create new nodes after kernel shutdown.");
	}
	// Verify that nodename doesn't already exist.
	if (nodeMap[nodename]) return;
	// Create the NodeAttr object.
	CPN::NodeAttr attr(GenerateId(nodename), nodename, nodetype);
	// Create the NodeInfo structure (which creates the node)
	CPN::NodeInfo* nodeinfo = new CPN::NodeInfo(*this, attr, arg, argsize);	
	// Put the NodeInfo into our map.
	nodeMap[nodename] = nodeinfo;
	if (status == STARTED) nodeinfo->GetNode()->Start();
}

void CPN::Kernel::CreateQueue(const ::std::string &queuename,
		const ::std::string &queuetype,
		const ulong queueLength,
		const ulong maxThreshold,
		const ulong numChannels) {
	PthreadMutexProtected plock(lock); 
	if (status == SHUTTINGDOWN || status == STOPPED) {
		throw CPN::KernelShutdownException("Cannot create new queue after kernel shutdown.");
	}
	// Verify that queuename doesn't already exist.
	if (queueMap[queuename]) return;
	// Generate the QueueAttr object.
	CPN::QueueAttr attr(GenerateId(queuename), queuename, queuetype,
		       	queueLength, maxThreshold, numChannels);
	// Create the QueueInfo which creates the queue
	CPN::QueueInfo* queueinfo = new CPN::QueueInfo(attr);
	// Put QueueInfo in our map.
	queueMap[queuename] = queueinfo;
}

void CPN::Kernel::ConnectWriteEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	if (status == SHUTTINGDOWN || status == STOPPED) {
		throw CPN::KernelShutdownException("Cannot create connection while kernel shutdown.");
	}
	// Validate that qname and nodename exist
	// Lookup the queue
	QueueInfo* qinfo = queueMap[qname];
	if (!qinfo) {
		throw std::invalid_argument("No such queue: " + qname);
	}
	NodeInfo* ninfo = nodeMap[nodename];
	if (!ninfo) {
		throw std::invalid_argument("No such node: " + nodename);
	}
	// Unregister the write end of the queue from it's registered
	// place if it is already registered.
	// Register the write end with the new place.
	ninfo->SetWriter(qinfo, portname);
}

void CPN::Kernel::ConnectReadEndpoint(const ::std::string &qname,
		const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	if (status == SHUTTINGDOWN || status == STOPPED) {
		throw CPN::KernelShutdownException("Cannot create connection while kernel shutdown.");
	}
	// Validate qname and nodename.
	// Look up the queue
	QueueInfo* qinfo = queueMap[qname];
	if (!qinfo) {
		throw std::invalid_argument("No such queue: " + qname);
	}
	NodeInfo* ninfo = nodeMap[nodename];
	if (!ninfo) {
		throw std::invalid_argument("No such node: " + nodename);
	}
	// Unregister the queue from its read port if applicable.
	// Register the read port with its new port.
	ninfo->SetReader(qinfo, portname);
}

CPN::QueueReader* CPN::Kernel::GetReader(const ::std::string &nodename,
		const ::std::string &portname) {
	PthreadMutexProtected plock(lock); 
	if (status == SHUTTINGDOWN || status == STOPPED) {
		throw CPN::KernelShutdownException("Kernel shutting down.");
	}
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
	if (status == SHUTTINGDOWN || status == STOPPED) {
		throw CPN::KernelShutdownException("Kernel shutting down.");
	}
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

CPN::ulong CPN::Kernel::GenerateId(const ::std::string& name) {
	return idcounter++;
}

void CPN::Kernel::NodeShutdown(const std::string &nodename) {
	PthreadMutexProtected plock(lock); 
	NodeInfo* ninfo = nodeMap[nodename];
	nodeMap.erase(nodename);
	nodesToDelete.push_back(ninfo);
	nodeTermination.Signal();
}



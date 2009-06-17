/** \file
 * Implementation for the NodeBase class.
 */

#include "NodeBase.h"


void CPN::NodeBase::ConnectWriter(const ::std::string &portname, QueueWriter *qwriter) {
	writerqlist[portname] = qwriter;
}

void CPN::NodeBase::ConnectReader(const ::std::string &portname, QueueReader *qreader) {
	readerqlist[portname] = qreader;
}

QueueWriter *CPN::NodeBase::GetWriter(const ::std::string &portname) {
	return writerqlist[portname];
}

QueueReader *CPN::NodeBase::GetReader(const ::std::string &portname) {
	return readerqlist[portname];
}

virtual void* CPN::NodeBase::EntryPoint(void) {
	Process();
	return 0;
}

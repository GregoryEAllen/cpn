
#ifndef MOCKSYNCNODE_H
#define MOCKSYNCNODE_H
#pragma once

#include "NodeBase.h"

#define MOCKSYNCNODE_TYPENAME "mocksyncnodetypename"

extern "C" {
    CPN::shared_ptr<CPN::NodeFactory> cpninitmocksyncnodetypename(void);
}

class MockSyncNode : public CPN::NodeBase {
public:
    enum Mode_t {
        MODE_SOURCE,
        MODE_SINK
    };
	MockSyncNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);

	void Process();
private:
    Mode_t mode;
    std::string othernode;
};

struct SyncSource {
    public:
    static void Run1(CPN::NodeBase *nb, std::string othernode);
    static void Run2(CPN::NodeBase *nb, std::string othernode);
    static void Run3(CPN::NodeBase *nb, std::string othernode);
    static void Run4(CPN::NodeBase *nb, std::string othernode);
    // goes only with SyncSink::Run3
    static void Run5(CPN::NodeBase *nb, std::string othernode);
};

struct SyncSink {
    public:
    static void Run1(CPN::NodeBase *nb, std::string othernode);
    static void Run2(CPN::NodeBase *nb, std::string othernode);
    // goes only with SyncSource::Run5
    static void Run3(CPN::NodeBase *nb, std::string othernode);
};

#endif



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
    struct Param {
        // Bleh, param needs to be memcpy able
        char othernode[50];
        Mode_t mode;
    };
	MockSyncNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);

	void Process();
private:
    Param param;
};

struct SyncSource {
    public:
    SyncSource(const std::string &onode) : othernode(onode) {}
    void Run1(CPN::NodeBase *nb);
    void Run2(CPN::NodeBase *nb);
    void Run3(CPN::NodeBase *nb);
    void Run4(CPN::NodeBase *nb);
    // goes only with SyncSink::Run3
    void Run5(CPN::NodeBase *nb);
    std::string othernode;
};

struct SyncSink {
    public:
    SyncSink(const std::string &onode) :othernode(onode) {}
    void Run1(CPN::NodeBase *nb);
    void Run2(CPN::NodeBase *nb);
    // goes only with SyncSource::Run5
    void Run3(CPN::NodeBase *nb);
    std::string othernode;
};

#endif


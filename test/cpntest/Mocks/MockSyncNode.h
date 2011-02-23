
#ifndef MOCKSYNCNODE_H
#define MOCKSYNCNODE_H
#pragma once

#include "NodeBase.h"
#include <iosfwd>

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
};

inline std::ostream &operator<<(std::ostream &os, MockSyncNode::Mode_t mode) {
    os << (int)mode;
    return os;
}

inline std::istream &operator>>(std::istream &is, MockSyncNode::Mode_t &mode) {
    int m;
    is >> m;
    mode = (MockSyncNode::Mode_t)m;
    return is;
}


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


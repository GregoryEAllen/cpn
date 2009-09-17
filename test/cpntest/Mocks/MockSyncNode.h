
#ifndef MOCKSYNCNODE_H
#define MOCKSYNCNODE_H
#pragma once

#include "NodeBase.h"

#define MOCKSYNCNODE_TYPENAME "mocksyncnodetypename"

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
    static void RegisterType();
private:
    Param param;
};

#endif



#ifndef MOCKNODE_H
#define MOCKNODE_H
#pragma once

#include "NodeBase.h"

#define MOCKNODE_TYPENAME "MockNode"
/**
 * A mock node for testing general testing.
 * Takes a mode to define it's behavior.
 */
class MockNode : public CPN::NodeBase {
public:
    enum Mode_t { MODE_SOURCE = 1, MODE_TRANSMUTE, MODE_SINK, MODE_NOP };

    MockNode(CPN::Kernel &ker, const CPN::NodeAttr &attr);

    void Process();

    static std::string GetModeName(Mode_t mode);
    static Mode_t GetMode(const std::string &param);
private:
    Mode_t mode;
};


#endif

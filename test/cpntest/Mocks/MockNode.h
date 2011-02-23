
#ifndef MOCKNODE_H
#define MOCKNODE_H
#pragma once

#include "NodeBase.h"
#include <iosfwd>

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
};

inline std::ostream &operator<<(std::ostream &os, MockNode::Mode_t mode) {
    os << MockNode::GetModeName(mode);
    return os;
}

inline std::istream &operator>>(std::istream &is, MockNode::Mode_t &mode) {
    std::string name;
    is >> name;
    mode = MockNode::GetMode(name);
    return is;
}

#endif

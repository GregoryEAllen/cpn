/** \file
 */

#include "DNode.h"
#include "GeneralHashFunctions.h"

using DHT::ulong;
using namespace DHT;

DNode::DNode(const std::string &name_)
: name(name_) {
    id = Hash(name.c_str(), name.length());
}

AutoBuffer Get(const ulong key) {
    DNode* node = FindNode(key);
    return node->data[key];
}

void Put(const ulong key, const AutoBuffer &buff) {
    DNode* node = FindNode(key);
    node->data[key] = buff;
}

DNode* DNode::FindFinger(ulong key) {
    DNode* current = this;
    for (unsigned i = 0; i < DHT_BITS; ++i) {
        if (Distance(current->id, key) > Distance(finger[i], key)) {
            current = finger[i];
        }
    }
    return current;
}
DNode* DNode::FindNode(ulong key) {
    DNode* current = FindFinger(key);
    DNode* next = current->FindFinger(key);
    while (Distance(current->id, key)
            > Distance(next->id, key)) {
        current = next;
        next = current->FindFinger(key);
    }
    return current;
}

ulong DNode::Distance(ulong a, ulong b) {
    return a ^ b;
}

ulong DNode::Hash(const void* const ptr, ulong len) {
    return DJBHash((const char*) ptr, len) & DHT_BITMASK;
}

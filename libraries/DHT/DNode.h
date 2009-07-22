/** \file
 * The basic node for the DHT.
 */

#ifndef DHT_DNODE_H
#define DHT_DNODE_H

#include "AutoBuffer.h"
#include <map>
#include <string>

namespace DHT {
    typedef unsigned long ulong;

    const ulong DHT_BITS = 32;
    const ulong DHT_BITMASK = 0xFFFFFFFF;

    class DNode {
    public:
        DNode(const std::string &name_);

        const AutoBuffer &Get(const ulong key);
        void Put(const ulong key, const AutoBuffer &buff);

        ulong GetID(void) const { return id; }

        static ulong Distance(ulong a, ulong b);
        static ulong Hash(const void* const ptr, ulong len);
    private:
        DNode* FindNode(ulong key);
        DNode* FindFinger(ulong key);

        ulong id;
        const std::string name;
        DNode* finger[DHT_BITS];
        DNode* pred;
        std::map<ulong, AutoBuffer> data;
    };
}

#endif


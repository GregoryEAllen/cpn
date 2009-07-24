/** \file
 * The basic node for the DHT.
 */

#ifndef DHT_DNODE_H
#define DHT_DNODE_H

#include "AutoBuffer.h"
#include <map>
#include <string>

namespace DHT {
    typedef unsigned key_t;

    const int KEY_BITS = 8*sizeof(key_t);
    const key_t MAX_KEY = -1;

    class DNode {
    public:
        DNode(const std::string &name_);

        void Connect(DNode* other);

        const AutoBuffer &Get(const key_t key);
        void Put(const key_t key, const AutoBuffer &buff);

        key_t GetID(void) const { return id; }
        const std::string &GetName(void) { return name; }

        void Verify(void);
        void Update(void);

        static key_t Distance(key_t a, key_t b);
        static key_t Hash(const void* const ptr, unsigned len);
    private:
        DNode* FindNode(key_t key);

        DNode* FindFinger(key_t key);

        key_t FingerID(int i) { return (id + (1<<i))%MAX_KEY; }
        DNode* &Finger(int i) { return finger[i]; }

        void PrintLink(void);

        DNode* &Next() { return succ; }
        DNode* &Prev() { return pred; }

        key_t id;
        const std::string name;
        DNode* finger[KEY_BITS];
        DNode* succ;
        DNode* pred;
        std::map<key_t, AutoBuffer> data;
    };
}

#endif


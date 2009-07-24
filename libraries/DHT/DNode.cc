/** \file
 */

#include "DNode.h"
#include "GeneralHashFunctions.h"
#include <cassert>
#include <cstdio>

namespace DHT {

    DNode::DNode(const std::string &name_)
    : name(name_) {
        id = Hash(name.c_str(), name.length());
        Prev() = Next() = this;
        for (int i = 0; i < KEY_BITS; ++i) {
            Finger(i) = this;
        }
    }

    void DNode::Connect(DNode* other) {
        Next() = other->FindNode(id);
        Prev() = Next()->Prev();
        Next()->Prev() = this;
        Prev()->Next() = this;
        Update();
        //PrintLink();
    }

    const AutoBuffer &DNode::Get(const key_t key) {
        DNode* node = FindNode(key);
        return node->data[key];
    }

    void DNode::Put(const key_t key, const AutoBuffer &buff) {
        DNode* node = FindNode(key);
        node->data[key] = buff;
    }

    DNode* DNode::FindNode(key_t key) {
        if (Distance(key, this->id) > Distance(key, Prev()->id)) {
            return Next()->FindNode(key);
        }
        return this;
    }

    void DNode::Update(void) {
        DNode* current = Next();
        for (int i = 0; i < KEY_BITS; ++i) {
            Finger(i) = current->FindNode(FingerID(i));
            current = Finger(i);
        }
    }

    void DNode::Verify(void) {
        DNode* current = this;
        do {
            assert(current->Next()->Prev() == current);
            assert(current->Prev()->Next() == current);
            assert(Distance(current->id, current->Next()->id)
                    <= Distance(current->id, current->Prev()->id));
            current = current->Next();
        } while (current != this);
    }

    void DNode::PrintLink(void) {
        printf("%u", id);
        DNode *current = Next();
        while (current != this) {
            printf(" -> %u", current->id);
            current = current->Next();
        }
        printf("\n");
    }

    key_t DNode::Distance(key_t a, key_t b) {
        if (a <= b) {
            return b - a;
        } else {
            return b + (MAX_KEY - a);
        }
    }

    key_t DNode::Hash(const void* const ptr, unsigned len) {
        return DJBHash((const char*) ptr, len) & MAX_KEY;
    }

}


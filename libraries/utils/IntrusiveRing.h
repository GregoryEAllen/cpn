//=============================================================================
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \brief An intrusive ring data structure
 * implementation.
 * \author John Bridgman
 */
#ifndef INTRUSIVERING_H
#define INTRUSIVERING_H
#pragma once

/**
 * The intrusive ring.
 */
template<typename T>
class IntrusiveRing {
public:
    IntrusiveRing() : front(0) {}

    T *Front() const { return front; }
    T *Back() const { return front->Prev(); }

    void PushFront(T *e) {
        e->ring = this;
        if (front) {
            PushPrev(front, e);
        }
        front = e;
    }

    void PushBack(T *e) {
        e->ring = this;
        if (front) {
            PushPrev(front, e);
        } else {
            front = e;
        }
    }

    T *PopFront() {
        T *ret = front;
        Advance();
        ret->Remove();
        return ret;
    }

    T *PopBack() {
        T *ret = front->Prev();
        ret->Remove();
        return ret;
    }

    void Advance() {
        front = front->Next();
    }

    void Recede() {
        front = front->Prev();
    }

    T *PushNext(T *cur, T *n) {
        n->Remove();
        n->ring = this;
        cur->next->prev = n;
        n->next = cur->next;
        cur->next = n;
        cur->next->prev = cur->ethis;
        return cur->next;
    }

    T *PushPrev(T *cur, T *p) {
        p->Remove();
        p->ring = this;
        cur->prev->next = p;
        p->prev = cur->prev;
        cur->prev = p;
        cur->prev->next = cur->ethis;
        return cur->prev;
    }

    void Remove(T *val) {
        if (val == front) {
           if (val->next == val->ethis) {
                front = 0;
           } else {
               Advance();
           }
        }
        val->next->prev = val->prev;
        val->prev->next = val->next;
        val->prev = val->next = val->ethis;
        val->ring = 0;
    }

    bool Empty() const { return 0 == front; }
private:
    T *front;
};


/**
 * The InstrusiveRing element. All elements
 * placed in the intrusive ring must publicly
 * enherit this class.
 */
template<typename T>
class IntrusiveRingElement {
    friend class IntrusiveRing<T>;
public:

    IntrusiveRingElement(T *ethis_) : next(ethis_), prev(ethis_),
        ethis(ethis_), ring(0) {}

    virtual ~IntrusiveRingElement() { Remove(); }

    T *Next(T *n) {
        return ring->PushNext(ethis, n);
    }

    T *Next() const { return next; }

    T *Prev(T *p) {
        return ring->PushPrev(ethis, p);
    }

    T *Prev() const { return prev; }

    void Remove() {
        if (ring) {
            ring->Remove(ethis);
        }
    }

    IntrusiveRing<T> *GetRing() const { return ring; }

private:
    T *next;
    T *prev;
    T *ethis;
    IntrusiveRing<T> *ring;
};

#endif

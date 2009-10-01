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
 *
 * \brief Message queue classes.
 *
 * These classes provide a message queue and the chain of responsibility design
 * pattern.
 *
 * \author John Bridgman
 */

#ifndef CPN_MESSAGEQUEUE_H
#define CPN_MESSAGEQUEUE_H
#pragma once

#include "CPNCommon.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include <sigc++/sigc++.h>
#include <list>
#include <deque>

namespace CPN {

    template<typename Type>
    class MsgPut {
    public:
        virtual ~MsgPut() {}
        virtual void Put(Type msg) = 0;
    };

    template<typename Type>
    class MsgQueue : public MsgPut<Type> {
    public:
        static shared_ptr<MsgQueue<Type> > Create();
        virtual ~MsgQueue() {}
        virtual void Put(Type msg) {
            PthreadMutexProtected pl(lock);
            queue.push_back(msg);
            cond.Signal();
        }

        Type Get() {
            PthreadMutexProtected pl(lock);
            while (queue.empty()) { cond.Wait(lock); }
            Type retval = queue.front();
            queue.pop_front();
            return retval;
        }

        bool Empty() {
            PthreadMutexProtected pl(lock);
            return queue.empty();
        }
    protected:
        MsgQueue() {}
    private:
        PthreadMutex lock;
        PthreadCondition cond;
        std::deque<Type> queue;
    };

    template<typename Type>
    class MsgEmptyChain : public MsgPut<Type> {
    public:
        static shared_ptr<MsgEmptyChain<Type> > Create();
        void Put(Type msg) {
            shared_ptr<MsgPut<Type> > thechain(chain);
            thechain->Put(msg);
        }
        void Chain(weak_ptr<MsgPut<Type> > msgq) {
            chain = msgq;
        }
        bool Unchained() { return chain.expired(); }
    private:
        MsgEmptyChain() {}
        weak_ptr<MsgPut<Type> > chain;
    };

    template<typename Type, typename Impl = MsgEmptyChain<Type> >
    class MsgQueueSignal : public Impl {
    public:
        static shared_ptr<MsgQueueSignal<Type, Impl> > Create();
        virtual ~MsgQueueSignal() {}
        virtual void Put(Type msg) {
            Impl::Put(msg);
            signal();
        }
        sigc::connection Connect(sigc::slot<void> slot) {
            return signal.connect(slot);
        }
    private:
        MsgQueueSignal() {}
        sigc::signal<void> signal;
    };

    template<typename Type>
    class MsgChain : public MsgPut<Type> {
    public:
        static shared_ptr<MsgChain<Type> > Create();

        void Put(Type msg) {
            PthreadMutexProtected pl(lock);
            shared_ptr<MsgPut<Type> > thechain = chain.lock();
            if (thechain) {
                thechain->Put(msg);
            } else {
                queue.push_back(msg);
            }
        }

        void Chain(weak_ptr<MsgPut<Type> > msgq) {
            PthreadMutexProtected pl(lock);
            chain = msgq;
            shared_ptr<MsgPut<Type> > thechain = chain.lock();
            if (thechain) {
                while (!queue.empty()) {
                    thechain->Put(queue.front());
                    queue.pop_front();
                }
            }
        }

        bool Unchained() { return chain.expired(); }
    private:
        MsgChain() {}
        weak_ptr<MsgPut<Type> > chain;
        PthreadMutex lock;
        std::deque<Type> queue;
    };

    template<typename Type, typename Mutator>
    class MsgMutator : public MsgPut<Type> {
    public:
        static shared_ptr<MsgMutator<Type, Mutator> > Create(Mutator mut = Mutator());

        void Put(Type msg) {
            shared_ptr<MsgPut<Type> > thechain(chain);
            msg = mutator(msg);
            thechain->Put(msg);
        }

        void Chain(weak_ptr<MsgPut<Type> > msgq) {
            chain = msgq;
        }

        bool Unchained() { return chain.expired(); }
    private:
        MsgMutator(Mutator mut) : mutator(mut) {}
        weak_ptr<MsgPut<Type> > chain;
        Mutator mutator;
    };

    template<typename Type>
    class MsgClonerPassthrough {
    public:
        Type operator() (Type t) { return t; }
    };

    template<typename Type>
    class MsgClonerClonemem {
    public:
        Type operator() (Type t) { return t->Clone(); }
    };

    template<typename Type, typename Cloner = MsgClonerPassthrough<Type> >
    class MsgBroadcaster : public MsgPut<Type> {
        typedef std::list< weak_ptr<MsgPut<Type> > > QueueList;
    public:
        static shared_ptr<MsgBroadcaster<Type, Cloner> > Create(Cloner c = Cloner());
        void Put(Type msg) {
            typename QueueList::iterator current = queues.begin();
            while (current != queues.end()) {
                shared_ptr<MsgPut<Type> > queue = current->lock();
                if (queue) {
                    queue->Put(cloner(msg));
                    ++current;
                } else {
                    current = queues.erase(current);
                }
            }
        }
        void AddQueue(weak_ptr<MsgPut<Type> > msgq) {
            queues.push_back(msgq);
        }
        bool Empty() { return queues.empty(); }
    private:
        MsgBroadcaster(Cloner c) : cloner(c) {}
        QueueList queues;
        Cloner cloner;
    };

    template<typename Type>
    inline shared_ptr<MsgQueue<Type> > MsgQueue<Type>::Create() {
        return shared_ptr<MsgQueue<Type> >(new MsgQueue<Type>());
    }

    template<typename Type, typename Impl>
    inline shared_ptr<MsgQueueSignal<Type, Impl> > MsgQueueSignal<Type, Impl>::Create() {
        return shared_ptr<MsgQueueSignal<Type, Impl> >(new MsgQueueSignal<Type, Impl>());
    }

    template<typename Type>
    inline shared_ptr<MsgEmptyChain<Type> > MsgEmptyChain<Type>::Create() {
        return shared_ptr<MsgEmptyChain<Type> >(new MsgEmptyChain<Type>());
    }

    template<typename Type>
    inline shared_ptr<MsgChain<Type> > MsgChain<Type>::Create() {
        return shared_ptr<MsgChain<Type> >(new MsgChain<Type>());
    }

    template<typename Type, typename Mutator>
    inline shared_ptr<MsgMutator<Type, Mutator> > MsgMutator<Type, Mutator>::Create(Mutator mut) {
        return shared_ptr<MsgMutator<Type, Mutator> >(new MsgMutator<Type, Mutator>(mut));
    }

    template<typename Type, typename Cloner>
    inline shared_ptr<MsgBroadcaster<Type, Cloner> > MsgBroadcaster<Type, Cloner>::Create(Cloner c) {
        return shared_ptr<MsgBroadcaster<Type, Cloner> >(new MsgBroadcaster<Type, Cloner>(c));
    }

}

#endif



#pragma once

#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "AutoLock.h"
#include <deque>
namespace Sync {
    /**
     */
    template<typename T, typename Q = std::deque<T> >
    class BlockingQueue {
    public:

        explicit BlockingQueue(const Q &q = Q()) : queue(q) {}

        bool Empty() const {
            AutoLock<PthreadMutex> al(lock);
            return queue.empty();
        }

        unsigned Size() const {
            AutoLock<PthreadMutex> al(lock);
            return queue.size();
        }

        void Push(const T &t) {
            AutoLock<PthreadMutex> al(lock);
            queue.push_back(t);
            cond.Signal();
        }

        T Pop() {
            AutoLock<PthreadMutex> al(lock);
            while (queue.empty()) cond.Wait(lock);
            T t = queue.front();
            queue.pop_front();
            return t;
        }

    private:
        PthreadMutex lock;
        PthreadCondition cond;
        Q queue; 
    };
}


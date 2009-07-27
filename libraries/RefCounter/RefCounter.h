/** \file
 * A reference counter object to be included for
 * objects that share resources between them.
 */

#ifndef REFCOUNTER_H
#define REFCOUNTER_H

#include <cassert>

class RefCounter {
    public:
        RefCounter() : count(0) {}
        virtual ~RefCounter() { assert(0 == count); }
        void Increment(void) {
            __sync_add_and_fetch(&count, 1);
        }
        bool Decrement(void) {
            return 0 == __sync_sub_and_fetch(&count, 1);
        }
    private:
        RefCounter(const RefCounter&);
        RefCounter& operator=(const RefCounter&);
        unsigned long count;
};

#endif


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
 * \author John Bridgman
 * \brief An object for access to the atomic
 * built ins.
 */
#ifndef SYNC_ATOMIC_H
#define SYNC_ATOMIC_H
#pragma once

namespace Sync {
    /**
     * The template type must be an integral or pointer type.
     */
    template<typename T>
    class Atomic {
    public:
        Atomic() {}
        Atomic(T o) { Set(o); }
        ~Atomic() {}

        Atomic(const Atomic &a) { Set(a.Get()); }

        Atomic<T> &operator=(T o) {
            Set(o);
            return *this;
        }

        // Get and op functions, perform op and return old value
        T GetAndAdd(T o) { return __sync_fetch_and_add(&val, o); }
        T GetAndSub(T o) { return __sync_fetch_and_sub(&val, o); }
        T GetAndOr(T o) { return __sync_fetch_and_or(&val, o); }
        T GetAndAnd(T o) { return __sync_fetch_and_and(&val, o); }
        T GetAndXor(T o) { return __sync_fetch_and_xor(&val, o); }
        T GetAndNand(T o) { return __sync_fetch_and_nand(&val, o); }

        // Op and get functions, perform op and return new value
        T Add(T o) { return __sync_add_and_fetch(&val, o); }
        T Sub(T o) { return __sync_sub_and_fetch(&val, o); }
        T Or(T o) { return __sync_sub_and_fetch(&val, o); }
        T And(T o) { return __sync_sub_and_fetch(&val, o); }
        T Xor(T o) { return __sync_sub_and_fetch(&val, o); }
        T Nand(T o) { return __sync_sub_and_fetch(&val, o); }

        /**
         * Performs essentially: if (val == oldval) { T tmp = val; val = newval; return tmp; } else { return val; }
         * \param oldval the value to compare
         * \param newval the value to set if comparison is true
         * \return the value of the variable before this operation.
         */
        T CompareAndSwap(T oldval, T newval) { return __sync_val_compare_and_swap(&val, oldval, newval); }

        void Set(T o) {
            T oldval;
            do {
                oldval = Get();
            } while (CompareAndSwap(oldval, o) != oldval);
        }

        T Get() const { return __sync_val_compare_and_swap(&val, T(), T()); }

    private:
        mutable volatile T val;
    };
}
#endif

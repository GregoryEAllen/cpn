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
 * \brief Automatic locking on the stack.
 * \author John Bridgman
 */

#ifndef AUTOLOCK_H
#define AUTOLOCK_H
#pragma once

/**
 * An automatic lock that ensures the lock is left in the state
 * at construction when destroyed.
 */
template<class Lockable>
class AutoLock {
public:
    /**
     * Create a new AutoLock and lock the mutex.
     * \param mutex_ the mutex
     */
    AutoLock(Lockable& mutex_) : mutex(mutex_), count(0) {
        Lock();
    }
    /**
     * Create a new AutoLock and only lock the mutex if lock is true
     * \param mutex_ the mutex
     * \param lock true to lock or false to not
     */
    AutoLock(Lockable& mutex_, bool lock) : mutex(mutex_), count(0) {
        if (lock) { Lock(); }
    }
    
    ~AutoLock() {
        while (count > 0)
            Unlock();
    }

    /**
     * Lock the mutex
     */
    void Unlock() {
        --count;
        mutex.Unlock();
    }

    /**
     * Unlock the mutex
     */
    void Lock() {
        mutex.Lock();
        ++count;
    }

private:
    Lockable& mutex;
    int count;
};
#endif

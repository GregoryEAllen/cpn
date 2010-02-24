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
 */

#include "ReentrantLock.h"

void Sync::ReentrantLock::Lock() const {
    Internal::ScopeMutex l(lock);
    if ( count != 0 && ( pthread_equal(owner, pthread_self()) ) ) {
        ++count;
    } else {
        while (count != 0) { pthread_cond_wait(&cond, &lock); }
        ++count;
        owner = pthread_self();
    }
}

void Sync::ReentrantLock::Wait(pthread_cond_t& c) const {
    long oldcount = count;
    pthread_t oldowner = owner;
    count = 0;
    pthread_cond_signal(&cond);
    pthread_cond_wait(&c, &lock);
    while (count != 0) { pthread_cond_wait(&cond, &lock); }
    count = oldcount;
    owner = oldowner;
}

bool Sync::ReentrantLock::HaveLock() const {
    Internal::ScopeMutex l(lock);
    return count != 0 && ( pthread_equal(owner, pthread_self()) );
}

//=============================================================================
//  Computational Process Networks class library
//  Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
//
//  This library is free software; you can redistribute it and/or modify it
//  under the terms of the GNU Library General Public License as published
//  by the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  The GNU Public License is available in the file LICENSE, or you
//  can write to the Free Software Foundation, Inc., 59 Temple Place -
//  Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//  World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 */

#include "Database.h"
#include "LocalDatabase.h"
#include "Exceptions.h"

namespace CPN {
    shared_ptr<Database> Database::Local() {
        return shared_ptr<Database>(new LocalDatabase);
    }

    Database::Database()
        : useD4R(true), swallowbrokenqueue(false),
        growmaxthresh(true)
    {
    }

    Database::~Database() {
    }

    bool Database::RequireRemote() {
        return false;
    }

    bool Database::UseD4R() {
        PthreadMutexProtected al(lock);
        return useD4R;
    }

    bool Database::UseD4R(bool u) {
        PthreadMutexProtected al(lock);
        return useD4R = u;
    }

    bool Database::SwallowBrokenQueueExceptions() {
        PthreadMutexProtected al(lock);
        return swallowbrokenqueue;
    }

    bool Database::SwallowBrokenQueueExceptions(bool sbqe) {
        PthreadMutexProtected al(lock);
        return swallowbrokenqueue = sbqe;
    }

    bool Database::GrowQueueMaxThreshold() {
        PthreadMutexProtected al(lock);
        return growmaxthresh;
    }

    bool Database::GrowQueueMaxThreshold(bool grow) {
        PthreadMutexProtected al(lock);
        return growmaxthresh = grow;
    }

    void Database::CheckTerminated() {
        if (IsTerminated()) {
            throw ShutdownException();
        }
    }


}


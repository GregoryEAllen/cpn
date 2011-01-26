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

#include "Context.h"
#include "LocalContext.h"
#include "Exceptions.h"

namespace CPN {
    shared_ptr<Context> Context::Local() {
        return shared_ptr<Context>(new LocalContext);
    }

    Context::Context()
        : useD4R(true), swallowbrokenqueue(false),
        growmaxthresh(true)
    {
    }

    Context::~Context() {
    }

    bool Context::RequireRemote() {
        return false;
    }

    bool Context::UseD4R() {
        PthreadMutexProtected al(lock);
        return useD4R;
    }

    bool Context::UseD4R(bool u) {
        PthreadMutexProtected al(lock);
        return useD4R = u;
    }

    bool Context::SwallowBrokenQueueExceptions() {
        PthreadMutexProtected al(lock);
        return swallowbrokenqueue;
    }

    bool Context::SwallowBrokenQueueExceptions(bool sbqe) {
        PthreadMutexProtected al(lock);
        return swallowbrokenqueue = sbqe;
    }

    bool Context::GrowQueueMaxThreshold() {
        PthreadMutexProtected al(lock);
        return growmaxthresh;
    }

    bool Context::GrowQueueMaxThreshold(bool grow) {
        PthreadMutexProtected al(lock);
        return growmaxthresh = grow;
    }

    void Context::CheckTerminated() {
        if (IsTerminated()) {
            throw ShutdownException();
        }
    }


}


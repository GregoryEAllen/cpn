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

#include "Database.h"
#include "LocalDatabase.h"
#include "NodeFactory.h"
#include "Exceptions.h"
#include <dlfcn.h>
#include <stdexcept>

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
        factorymap.clear();
        for (LibMap::iterator itr = libmap.begin(); itr != libmap.end(); ++itr) {
            dlclose(itr->second);
        }
        libmap.clear();
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

    void Database::LoadSharedLib(const std::string &libname) {
        PthreadMutexProtected al(lock);
        InternalLoadLib(libname);
    }


    NodeFactory *Database::GetNodeFactory(const std::string &nodetype) {
        PthreadMutexProtected al(lock);
        FactoryMap::iterator entry = factorymap.find(nodetype);
        if (entry == factorymap.end()) {
            // Todo search libraries for the factory
            InternalLoad(nodetype);
            entry = factorymap.find(nodetype);
            if (entry == factorymap.end()) {
                return 0;
            }
        }
        return entry->second.get();
    }

    void Database::RegisterNodeFactory(shared_ptr<NodeFactory> factory) {
        PthreadMutexProtected al(lock);
        std::string name = factory->GetName();
        factorymap.insert(std::make_pair(name, factory));
    }

    void Database::InternalLoadLib(const std::string &lib) {
        if (libmap.find(lib) == libmap.end()) {
            void *handle = dlopen(lib.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            if (!handle) {
                throw std::runtime_error(dlerror());
            } else {
                libmap.insert(std::make_pair(lib, handle));
            }
        }
    }

    void Database::InternalLoad(const std::string &name) {
        std::string sym = CPN_DEFAULT_INIT_SYMBOL_STR;
        sym += name;
        void *handle = 0;

        // Use a union to avoid having to cast
        // from void pointer to function pointer explicitely.
        // Doing so would violate strict aliasing.
        union {
            void *vptr;
            CPNInitPrototype fn;
        } init;

        handle = dlopen(0, RTLD_LAZY);
        if (!handle) {
            throw std::runtime_error(dlerror());
        }
        try {
            init.vptr = dlsym(handle, sym.c_str());
            char *error = 0;
            if ((error = dlerror()) != 0) {
                InternalLoadLib(name);
                dlerror();
                init.vptr = dlsym(handle, sym.c_str());
                char *error = 0;
                if ((error = dlerror()) != 0) {
                    throw std::runtime_error(error);
                }
            }
            shared_ptr<NodeFactory> factory = init.fn();
            factorymap.insert(std::make_pair(factory->GetName(), factory));
        } catch (...) {
            dlclose(handle);
            throw;
        }
        dlclose(handle);
    }
}


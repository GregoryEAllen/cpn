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

#include "NodeLoader.h"
#include "NodeFactory.h"
#include "ThrowingAssert.h"
#include "PathUtils.h"
#include <fstream>
#include <dlfcn.h>
#include <stdexcept>

void Loader(const std::string &ldfile, std::map<std::string, std::string> &data);

namespace CPN {

    NodeLoader::NodeLoader() {
    }

    NodeLoader::~NodeLoader() {
        factorymap.clear();
        for (LibMap::iterator itr = libmap.begin(); itr != libmap.end(); ++itr) {
            dlclose(itr->second);
        }
        libmap.clear();
    }

    void NodeLoader::LoadSharedLib(const std::string &libname) {
        PthreadMutexProtected al(lock);
        InternalLoadLib(libname);
    }

    void NodeLoader::LoadSharedLib(const std::vector<std::string> &list) {
        for (std::vector<std::string>::const_iterator i = list.begin(), e = list.end(); i != e; ++i) {
            LoadSharedLib(*i);
        }
    }

    void NodeLoader::LoadNodeList(const std::string &filename) {
        Loader(filename, nodelibmap);
    }

    void NodeLoader::LoadNodeList(const std::vector<std::string> &list) {
        for (std::vector<std::string>::const_iterator i = list.begin(), e = list.end(); i != e; ++i) {
            LoadNodeList(*i);
        }
    }

    NodeFactory *NodeLoader::GetFactory(const std::string &nodetype) {
        PthreadMutexProtected al(lock);
        FactoryMap::iterator entry = factorymap.find(nodetype);
        if (entry == factorymap.end()) {
            InternalLoad(nodetype);
            entry = factorymap.find(nodetype);
            ASSERT(entry != factorymap.end());
        }
        return entry->second.get();
    }

    void NodeLoader::RegisterFactory(shared_ptr<NodeFactory> factory) {
        PthreadMutexProtected al(lock);
        std::string name = factory->GetName();
        factorymap.insert(std::make_pair(name, factory));
    }

    void NodeLoader::InternalLoadLib(const std::string &lib) {
        if (libmap.find(lib) == libmap.end()) {
            void *handle = dlopen(lib.c_str(), RTLD_LAZY | RTLD_GLOBAL);
            if (!handle) {
                throw std::runtime_error(dlerror());
            } else {
                libmap.insert(std::make_pair(lib, handle));
            }
        }
    }

    void NodeLoader::InternalLoad(const std::string &name) {
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
            char *error = dlerror();
            if (error != 0) {
                NodeLibMap::iterator itr = nodelibmap.find(sym);
                if (itr == nodelibmap.end()) {
                    throw std::runtime_error("Unable to find node type " + name);
                }
                InternalLoadLib(itr->second);
                dlerror();
                init.vptr = dlsym(handle, sym.c_str());
                error = dlerror();
                if (error != 0) {
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

using std::ifstream;
using std::vector;
using std::stringstream;
using std::string;
using std::map;


void ProcessLine(vector<string> &stack, const string &curdir,
        map<string, string> &data, unsigned rep);

void Loader(const string &ldfile, map<string, string> &data, unsigned rep) {
    ASSERT(rep < 100, "Loader include overflow");
    ifstream f;
    f.open(ldfile.c_str());
    ASSERT(f.is_open(), "Failed to open %s", ldfile.c_str());
    string curdir = DirName(RealPath(ldfile));
    vector<char> token_stack;
    vector<string> stack;
    bool in_comment = false;
    bool escape = false;
    bool in_single_quote = false;
    bool in_double_quote = false;
    while (f.good()) {
        int c = f.get();
        if (!f.good()) {
            break;
        }
        if (in_comment) {
            if (c == '\n' || c == '\r') {
                in_comment = false;
            } else {
                continue;
            }
        }
        if (in_single_quote) {
            if (c == '\'') {
                in_single_quote = false;
            } else {
                token_stack.push_back(c);
            }
        } else if (escape) {
            token_stack.push_back(c);
            escape = false;
        } else if (in_double_quote) {
            if (c == '"') {
                in_double_quote = false;
            } else {
                token_stack.push_back(c);
            }
        } else {
            switch (c) {
            case '#':
                in_comment = true;
                break;
            case '\'':
                in_single_quote = true;
                break;
            case '"':
                in_double_quote = true;
                break;
            case '\\':
                escape = true;
                break;
            case ';':
                if (!token_stack.empty()) {
                    stack.push_back(string(&token_stack[0], token_stack.size()));
                    token_stack.clear();
                }
                ProcessLine(stack, curdir, data, rep);
                break;
            case '\n':
            case '\r':
                // end of line
            case ' ':
            case '\t':
            case '\v':
            case '\f':
                // white space
                if (!token_stack.empty()) {
                    // end the current token, otherwise nothing
                    stack.push_back(string(&token_stack[0], token_stack.size()));
                    token_stack.clear();
                }
                break;
            default:
                token_stack.push_back(c);
                break;
            }
        }
    }
    ProcessLine(stack, curdir, data, rep);
}

void ProcessLine(vector<string> &stack, const string &curdir,
        map<string, string> &data, unsigned rep) {
    if (stack.empty()) return;
    vector<string>::iterator itr = stack.begin();
    if (*itr == "lib") {
        ++itr;
        if (itr != stack.end()) {
            string libname = *itr;
            if (!IsAbsPath(libname)) {
                libname = RealPath(PathConcat(curdir, libname));
            }
            if (!libname.empty()) {
                ++itr;
                while (itr != stack.end()) {
                    data[*itr] = libname;
                    ++itr;
                }
            }
        }
    } else if (*itr == "include") {
        ++itr;
        while (itr != stack.end()) {
            string inc = *itr;
            if (!IsAbsPath(inc)) {
                inc = RealPath(PathConcat(curdir, inc));
            }
            Loader(inc, data, rep + 1);
            ++itr;
        }
    }
    stack.clear();
}

void Loader(const string &ldfile, map<string, string> &data) {
    Loader(ldfile, data, 0);
}


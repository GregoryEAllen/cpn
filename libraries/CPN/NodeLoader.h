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
#ifndef NODELOADER_H
#define NODELOADER_H
#pragma once
#include "CPNCommon.h"
#include "PthreadMutex.h"
#include <string>
#include <map>

#define CPN_DEFAULT_INIT_SYMBOL cpninit
#define CPN_DEFAULT_INIT_SYMBOL_STR "cpninit"
namespace CPN {
    /** \brief This is the prototype of the function
     * that is called by the dynamic library loading
     * facility.
     */
    typedef shared_ptr<NodeFactory> (*CPNInitPrototype)(void);

    /**
     * This class encapsulates the necessary data and functions
     * for loading up nodes.
     */
    class CPN_LOCAL NodeLoader {
    public:
        NodeLoader();
        ~NodeLoader();
        /**
         * Loads a shared library for node init function searching.
         * \param libname the shared library file name
         */
        void LoadSharedLib(const std::string &libname);
        /**
         * Loads up a node list. A node list contains information
         * about what shared libraries need to be loaded to get a given node.
         * \param filename the name of the nodelist file.
         */
        void LoadNodeList(const std::string &filename);

        /**
         * Get the NodeFactory for the given node type
         * \param nodename the node type
         * \return a node factory
         */
        NodeFactory *GetFactory(const std::string &nodename);
        
        /**
         * Manually register a node factory with the NodeLoader.
         * \param factory the factory
         */
        void RegisterFactory(shared_ptr<NodeFactory> factory);

    private:
        void InternalLoadLib(const std::string &lib);
        void InternalLoad(const std::string &sym);

        PthreadMutex lock;
        typedef std::map<std::string, void*> LibMap;
        LibMap libmap;
        typedef std::map<std::string, shared_ptr<NodeFactory> > FactoryMap;
        FactoryMap factorymap;
        typedef std::map<std::string, std::string> NodeLibMap;
        NodeLibMap nodelibmap;

    };
}
#endif

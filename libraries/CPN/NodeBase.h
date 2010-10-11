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
 * \brief The base definition of all nodes.
 * \author John Bridgman
 */

#ifndef CPN_NODEBASE_H
#define CPN_NODEBASE_H
#pragma once

#include "CPNCommon.h"
#include "NodeAttr.h"
#include "NodeFactory.h"
#include "PseudoNode.h"

class Pthread;

namespace CPN {

    /**
     * \brief The definition common to all nodes in the process network.
     *
     * A node is a thread of execution which lasts the
     * lifetime of the node object.
     *
     */
    class CPN_API NodeBase : public PseudoNode {
    public:
        NodeBase(Kernel &ker, const NodeAttr &attr);

        virtual ~NodeBase();

        /**
         * \return the type id of this node
         */
        const std::string &GetTypeName() const { return type; }

        /** \brief Return a pointer to the kernel that his node is running under
         */
        Kernel *GetKernel() { return &kernel; }

        /**
         * \brief For use by the CPN::Kernel to start the node.
         */
        void Start();

        void Shutdown();

        /// For debugging ONLY!
        void LogState();

        bool IsPurePseudo();
    protected:

        /** \brief Override this method to implement a node */
        virtual void Process() = 0;

        Kernel &kernel;
    private:
        void* EntryPoint();

        const std::string type;
        auto_ptr<Pthread> thread;
    };

}

#define CPN_DECLARE_NODE_FACTORY(type, klass) class type ## Factory : public CPN::NodeFactory {\
    public: type ## Factory() : CPN::NodeFactory(#type) {}\
    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) { return CPN::shared_ptr<CPN::NodeBase>(new klass(ker, attr)); }};\
extern "C" CPN::shared_ptr<CPN::NodeFactory> cpninit ## type (void);\
CPN::shared_ptr<CPN::NodeFactory> cpninit ## type (void) { return CPN::shared_ptr<CPN::NodeFactory>(new type ## Factory); }

#endif

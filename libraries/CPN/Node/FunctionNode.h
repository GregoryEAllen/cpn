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
 * \brief FunctionNode a node class that takes as a paramater a
 * function object.
 * \author John Bridgman
 */

#ifndef CPN_FUNCTIONNODE_H
#define CPN_FUNCTIONNODE_H
#pragma once

#include "CPNCommon.h"
#include "NodeBase.h"
#include "NodeFactory.h"
#include "Database.h"
#include "Assert.h"
#include <string>

namespace CPN {
    /**
     * A node that takes a function as an argument and then calls it.  This is
     * to simplify when we have rather simple nodes that have very little data.
     */
    template<typename Func>
    class FunctionNode : public NodeBase {
    public:
		FunctionNode(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr), function(*((Func*)attr.GetArg().GetBuffer())) {
            ASSERT(attr.GetArg().GetSize() == sizeof(Func), "Required function parameter missing.");
            ASSERT(attr.GetArg().GetBuffer() != 0, "Required function parameter missing.");
        }
        static void RegisterType(shared_ptr<Database> db, const std::string &name);
    private:
        void Process() {
            function(this);
        }
        Func function;

        class FunctionFactory : public NodeFactory {
        public:
            FunctionFactory(const std::string &name)
                : NodeFactory(name) {}
            shared_ptr<NodeBase> Create(Kernel &ker, const NodeAttr &attr) {
                return shared_ptr<NodeBase>(new FunctionNode<Func>(ker, attr));
            }
        };
    };

    template<typename Func>
    inline void FunctionNode<Func>::RegisterType(shared_ptr<Database> db, const std::string &name) {
        db->RegisterNodeFactory(shared_ptr<NodeFactory>(
                    new FunctionFactory(name)));
    }

    template<typename Klass>
    class MemberFunction {
    public:
        MemberFunction(Klass* k, void (Klass::*f)(NodeBase*))
        : klass(k), func(f) {}
        void operator()(NodeBase* nb) {
            ((klass)->*(func))(nb);
        }
    private:
        Klass *klass;
        void (Klass::*func)(NodeBase*);
    };
}

#endif

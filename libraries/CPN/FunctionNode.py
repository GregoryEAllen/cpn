import sys
out = sys.stdout

maxparam = 10;

out.write("""//=============================================================================
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
/** \\file
 * \\brief The FunctionNodes are node class that takes as a paramater a
 * function object.
 *
 * The end user should never use these but instead be calling Kernel::CreateFunctionNode
 * Don't edit this file, edit FunctionNode.py and then manual recreate this file.
 * Note sections of this file needs to be manually moved to Kernel.h.
 * This file should not need to change very often.
 * Things where done this way because C++ doesn't have vararg templates (well
 * at least the version we are using doesn't).
 * \\author John Bridgman
 */

#ifndef CPN_FUNCTIONNODE_H
#define CPN_FUNCTIONNODE_H
#pragma once

#include "CPNCommon.h"
#include "NodeBase.h"

namespace CPN {
    /**
     * Nodes that takes a function as an argument and then calls it.  This is
     * to simplify when we have rather simple nodes that have very little data.
     * \@{
     */""")

for i in range(maxparam):
    out.write("""
    template<typename Function""")
    for a in range(i):
        out.write(""", typename Argument%d"""%(a + 1));
    out.write(""">
    class FunctionNode%(num)d : public NodeBase {
    public:
        FunctionNode%(num)d(Kernel &ker, const NodeAttr &attr, Function f"""%{'num':i})
    for a in range(i):
        out.write(""", Argument%d a%d"""%(a + 1, a + 1))
    out.write(""")
        : NodeBase(ker, attr), function(f)""")
    for a in range(i):
        out.write(""", arg%d(a%d)"""%(a + 1, a + 1))
    out.write("""
        {}
    private:
        void Process() {
            function(this""")
    for a in range(i):
        out.write(""", arg%d"""%(a+1))
    out.write(""");
        }
        Function function;""")
    for a in range(i):
        out.write("""
        Argument%d arg%d;"""%(a+1, a+1))
    out.write("""
    };
""")
    out.write("""    template<typename Function""")
    for a in range(i):
        out.write(""", typename Argument%d"""%(a + 1))
    out.write(""">
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func""")
    for a in range(i):
        out.write(""", Argument%(num)d arg%(num)d"""%{'num':a + 1})
    out.write(""")
    {
        Key_t nodekey = context->CreateNodeKey(hostkey, nodename);
        NodeAttr attr(nodename, "FunctionNode%(num)d");
        attr.SetKey(nodekey).SetHostKey(hostkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode%(num)d<Function"""%{'num':i})
    for a in range(i):
        out.write(""", Argument%d"""%(a + 1))
    out.write(""">(*this, attr, func""")
    for a in range(i):
        out.write(""", arg%d"""%(a+1))
    out.write("""));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }
""")


out.write("""
    /**
     * \@}
     */
 }
#endif
""")
out.write("""
#if 0
// The following must be copied manually to Kernel.h if it has changed.
""")

for i in range(maxparam):
    out.write("""        template<typename Function""")
    for a in range(i):
        out.write(""", typename Argument%d"""%(a + 1))
    out.write(""">
        Key_t CreateFunctionNode(const std::string &nodename, Function func""")
    for a in range(i):
        out.write(""", Argument%(num)d arg%(num)d"""%{'num':a + 1})
    out.write(""");
""")

out.write("""#endif
""")


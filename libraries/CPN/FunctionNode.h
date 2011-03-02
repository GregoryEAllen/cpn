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
 * \brief The FunctionNodes are node class that takes as a paramater a
 * function object.
 *
 * The end user should never use these but instead be calling Kernel::CreateFunctionNode
 * Don't edit this file, edit FunctionNode.py and then manual recreate this file.
 * Note sections of this file needs to be manually moved to Kernel.h.
 * This file should not need to change very often.
 * Things where done this way because C++ doesn't have vararg templates (well
 * at least the version we are using doesn't).
 * \author John Bridgman
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
     */
    template<typename Function>
    class FunctionNode0 : public NodeBase {
    public:
        FunctionNode0(Kernel &ker, const NodeAttr &attr, Function f)
        : NodeBase(ker, attr), function(f)
        {}
    private:
        void Process() {
            function(this);
        }
        Function function;
    };
    template<typename Function>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode0");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode0<Function>(*this, attr, func));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1>
    class FunctionNode1 : public NodeBase {
    public:
        FunctionNode1(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1)
        : NodeBase(ker, attr), function(f), arg1(a1)
        {}
    private:
        void Process() {
            function(this, arg1);
        }
        Function function;
        Argument1 arg1;
    };
    template<typename Function, typename Argument1>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode1");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode1<Function, Argument1>(*this, attr, func, arg1));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2>
    class FunctionNode2 : public NodeBase {
    public:
        FunctionNode2(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2)
        {}
    private:
        void Process() {
            function(this, arg1, arg2);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
    };
    template<typename Function, typename Argument1, typename Argument2>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode2");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode2<Function, Argument1, Argument2>(*this, attr, func, arg1, arg2));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3>
    class FunctionNode3 : public NodeBase {
    public:
        FunctionNode3(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode3");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode3<Function, Argument1, Argument2, Argument3>(*this, attr, func, arg1, arg2, arg3));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4>
    class FunctionNode4 : public NodeBase {
    public:
        FunctionNode4(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3, Argument4 a4)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3), arg4(a4)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3, arg4);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
        Argument4 arg4;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode4");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode4<Function, Argument1, Argument2, Argument3, Argument4>(*this, attr, func, arg1, arg2, arg3, arg4));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5>
    class FunctionNode5 : public NodeBase {
    public:
        FunctionNode5(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3, Argument4 a4, Argument5 a5)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3), arg4(a4), arg5(a5)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3, arg4, arg5);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
        Argument4 arg4;
        Argument5 arg5;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode5");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode5<Function, Argument1, Argument2, Argument3, Argument4, Argument5>(*this, attr, func, arg1, arg2, arg3, arg4, arg5));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6>
    class FunctionNode6 : public NodeBase {
    public:
        FunctionNode6(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3, Argument4 a4, Argument5 a5, Argument6 a6)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3), arg4(a4), arg5(a5), arg6(a6)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3, arg4, arg5, arg6);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
        Argument4 arg4;
        Argument5 arg5;
        Argument6 arg6;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode6");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode6<Function, Argument1, Argument2, Argument3, Argument4, Argument5, Argument6>(*this, attr, func, arg1, arg2, arg3, arg4, arg5, arg6));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7>
    class FunctionNode7 : public NodeBase {
    public:
        FunctionNode7(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3, Argument4 a4, Argument5 a5, Argument6 a6, Argument7 a7)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3), arg4(a4), arg5(a5), arg6(a6), arg7(a7)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
        Argument4 arg4;
        Argument5 arg5;
        Argument6 arg6;
        Argument7 arg7;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6, Argument7 arg7)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode7");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode7<Function, Argument1, Argument2, Argument3, Argument4, Argument5, Argument6, Argument7>(*this, attr, func, arg1, arg2, arg3, arg4, arg5, arg6, arg7));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7, typename Argument8>
    class FunctionNode8 : public NodeBase {
    public:
        FunctionNode8(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3, Argument4 a4, Argument5 a5, Argument6 a6, Argument7 a7, Argument8 a8)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3), arg4(a4), arg5(a5), arg6(a6), arg7(a7), arg8(a8)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
        Argument4 arg4;
        Argument5 arg5;
        Argument6 arg6;
        Argument7 arg7;
        Argument8 arg8;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7, typename Argument8>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6, Argument7 arg7, Argument8 arg8)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode8");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode8<Function, Argument1, Argument2, Argument3, Argument4, Argument5, Argument6, Argument7, Argument8>(*this, attr, func, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7, typename Argument8, typename Argument9>
    class FunctionNode9 : public NodeBase {
    public:
        FunctionNode9(Kernel &ker, const NodeAttr &attr, Function f, Argument1 a1, Argument2 a2, Argument3 a3, Argument4 a4, Argument5 a5, Argument6 a6, Argument7 a7, Argument8 a8, Argument9 a9)
        : NodeBase(ker, attr), function(f), arg1(a1), arg2(a2), arg3(a3), arg4(a4), arg5(a5), arg6(a6), arg7(a7), arg8(a8), arg9(a9)
        {}
    private:
        void Process() {
            function(this, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
        }
        Function function;
        Argument1 arg1;
        Argument2 arg2;
        Argument3 arg3;
        Argument4 arg4;
        Argument5 arg5;
        Argument6 arg6;
        Argument7 arg7;
        Argument8 arg8;
        Argument9 arg9;
    };
    template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7, typename Argument8, typename Argument9>
    Key_t Kernel::CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6, Argument7 arg7, Argument8 arg8, Argument9 arg9)
    {
        Key_t nodekey = context->CreateNodeKey(kernelkey, nodename);
        NodeAttr attr(nodename, "FunctionNode9");
        attr.SetKey(nodekey).SetKernelKey(kernelkey);
        Sync::AutoReentrantLock arlock(nodelock);
        shared_ptr<NodeBase> node;
        node.reset(new FunctionNode9<Function, Argument1, Argument2, Argument3, Argument4, Argument5, Argument6, Argument7, Argument8, Argument9>(*this, attr, func, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9));
        nodemap.insert(std::make_pair(nodekey, node));
        node->Start();
        return nodekey;
    }

    /**
     * \@}
     */
 }
#endif

#if 0
// The following must be copied manually to Kernel.h if it has changed.
        template<typename Function>
        Key_t CreateFunctionNode(const std::string &nodename, Function func);
        template<typename Function, typename Argument1>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1);
        template<typename Function, typename Argument1, typename Argument2>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6, Argument7 arg7);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7, typename Argument8>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6, Argument7 arg7, Argument8 arg8);
        template<typename Function, typename Argument1, typename Argument2, typename Argument3, typename Argument4, typename Argument5, typename Argument6, typename Argument7, typename Argument8, typename Argument9>
        Key_t CreateFunctionNode(const std::string &nodename, Function func, Argument1 arg1, Argument2 arg2, Argument3 arg3, Argument4 arg4, Argument5 arg5, Argument6 arg6, Argument7 arg7, Argument8 arg8, Argument9 arg9);
#endif

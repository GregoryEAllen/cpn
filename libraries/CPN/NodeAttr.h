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
 * \brief Definition for node attributes.
 * \author John Bridgman
 */
#ifndef CPN_NODEATTR_H
#define CPN_NODEATTR_H
#pragma once

#include "CPNCommon.h"
#include <string>

namespace CPN {

    /**
     * \brief Attributes for a node.
     *
     * This structure contains all the attributes and
     * parameters a node needs to start up. Note that
     * this structure is used to pass to the Kernel to
     * create a node and that fields like key are set by
     * the Kernel.
     *
     * The only required parameters are the name and node type.
     *
     * The other parameters that the user may set are:
     *
     * The host for the node, can be set as ether the host name
     * or the host key. The host key is set host name is ignored.
     * See SetHost and SetHostKey.
     *
     *
     * The parameters sent to the node.
     * See SetParam and GetParam
     *
     * Note that the key attribute are set by the kernel
     * when the node is to be created and such are overwritten.
     */
    class CPN_API NodeAttr {
    public:
        NodeAttr(const std::string &name_,
                const std::string &nodetype_)
            : host(),
            hostkey(0),
            name(name_),
            nodetype(nodetype_),
            param(""),
            key(0)
        {}

        NodeAttr(const std::string &name_,
                const std::string &nodetype_,
                const std::string &param_)
            : host(),
            hostkey(0),
            name(name_),
            nodetype(nodetype_),
            param(param_),
            key(0)
        {}

        NodeAttr &SetName(const std::string &name_) {
            name = name_;
            return *this;
        }

        NodeAttr &SetTypeName(const std::string &nodetype_) {
            nodetype = nodetype_;
            return *this;
        }

        NodeAttr &SetHost(const std::string &host_) {
            host = host_;
            return *this;
        }

        NodeAttr &SetHostKey(Key_t k) {
            hostkey = k;
            return *this;
        }

        NodeAttr &SetParam(const std::string &param_) {
            param = param_;
            return *this;
        }

        NodeAttr &SetKey(Key_t key_) {
            key = key_;
            return *this;
        }

        const std::string &GetHost() const { return host; }
        Key_t GetHostKey() const { return hostkey; }

        const std::string &GetName() const { return name; }

        const std::string &GetTypeName() const { return nodetype; }

        const std::string &GetParam() const { return param; }

        Key_t GetKey() const { return key; }

    private:
        std::string host;
        Key_t hostkey;
        std::string name;
        std::string nodetype;
        std::string param;
        Key_t key;
    };
}
#endif

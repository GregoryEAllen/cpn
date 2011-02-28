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
 * \brief Definition of the kernel attributes.
 * \author John Bridgman
 */

#ifndef CPN_KERNELATTR_H
#define CPN_KERNELATTR_H
#pragma once

#include "CPNCommon.h"
#include <string>
#include <vector>

namespace CPN {
    /**
     * \brief The attribute for the Kernel.
     *
     * Possible parameters to the Kernel are the name, hostname, servname, and context.
     * Only name is required.
     */
    class CPN_API KernelAttr {
    public:
        /** \brief Create a new KernelAttr.
         * There is only one required attributed and that is to give the Kernel a name.
         * \param name_ the name for the Kernel.
         */
        KernelAttr(const std::string &name_)
            : name(name_),
            hostname("localhost"),
            servname(""),
            remote_enabled(false),
            useD4R(true), swallowbrokenqueue(false),
            growmaxthresh(true)
        {}

        KernelAttr(const char* name_)
            : name(name_),
            hostname("localhost"),
            servname(""),
            remote_enabled(false),
            useD4R(true), swallowbrokenqueue(false),
            growmaxthresh(true)
        {}

        KernelAttr &SetName(const std::string &n) {
            name = n;
            return *this;
        }

        KernelAttr &SetHostName(const std::string &hn) {
            hostname = hn;
            return *this;
        }

        KernelAttr &SetServName(const std::string &sn) {
            servname = sn;
            return *this;
        }

        KernelAttr &SetContext(shared_ptr<Context> ctx) {
            context = ctx;
            return *this;
        }

        KernelAttr &SetRemoteEnabled(bool en) {
            remote_enabled = en;
            return *this;
        }

        KernelAttr &UseD4R(bool enable) {
            useD4R = enable;
            return *this;
        }

        KernelAttr &SwallowBrokenQueueExceptions(bool enable) {
            swallowbrokenqueue = enable;
            return *this;
        }

        KernelAttr &GrowQueueMaxThreshold(bool enable) {
            growmaxthresh = enable;
            return *this;
        }

        KernelAttr &AddSharedLib(const std::string &lib) {
            sharedlibs.push_back(lib);
            return *this;
        }

        KernelAttr &AddNodeList(const std::string &list) {
            nodelists.push_back(list);
            return *this;
        }

        const std::string &GetName() const { return name; }

        const std::string &GetHostName() const { return hostname; }

        const std::string &GetServName() const { return servname; }

        shared_ptr<Context> GetContext() const { return context; }

        bool GetRemoteEnabled() const { return remote_enabled; }

        bool UseD4R() const { return useD4R; }

        bool SwallowBrokenQueueExceptions() const { return swallowbrokenqueue; }

        bool GrowQueueMaxThreshold() const { return growmaxthresh; }

        const std::vector<std::string> &GetSharedLibs() const { return sharedlibs; }

        const std::vector<std::string> &GetNodeLists() const { return nodelists; }
    private:
        std::string name;
        std::string hostname;
        std::string servname;
        shared_ptr<Context> context;
        bool remote_enabled;
        bool useD4R;
        bool swallowbrokenqueue;
        bool growmaxthresh;
        std::vector<std::string> sharedlibs;
        std::vector<std::string> nodelists;
    };
}
#endif

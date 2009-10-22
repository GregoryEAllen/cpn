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
 * \brief Definition of the kernel attributes.
 * \author John Bridgman
 */

#ifndef CPN_KERNELATTR_H
#define CPN_KERNELATTR_H
#pragma once

#include "CPNCommon.h"
#include <string>

namespace CPN {
	/**
	 * \brief The attribute for the Kernel.
	 */
	class CPN_API KernelAttr {
	public:
		KernelAttr(const std::string &name_)
			: name(name_),
            hostname("localhost"),
            servname("")
        {}

        KernelAttr &SetHostName(const std::string &hn) {
            hostname = hn;
            return *this;
        }

        KernelAttr &SetServName(const std::string &sn) {
            servname = sn;
            return *this;
        }

        KernelAttr &SetDatabase(shared_ptr<Database> db) {
            database = db;
            return *this;
        }

        const std::string &GetName() const { return name; }

        const std::string &GetHostName() const { return hostname; }

        const std::string &GetServName() const { return servname; }

        shared_ptr<Database> GetDatabase() const { return database; }
    private:
        std::string name;
        std::string hostname;
        std::string servname;
        shared_ptr<Database> database;
	};
}
#endif

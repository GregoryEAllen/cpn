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
 * \brief A XML loader class for the
 * CPN Kernel.
 * \author John Bridgman
 */

#ifndef CPN_KERNELXMLLOADER_H
#define CPN_KERNELXMLLOADER_H
#pragma once

#include "Kernel.h"
#include <string>

namespace xmlpp {
	class Node;
}

namespace CPN {
	class KernelXMLLoader {
	public:
		KernelXMLLoader(const std::string &filename);
		~KernelXMLLoader();
		KernelAttr GetKernelAttr(void);
		void SetupNodes(Kernel &kernel);
		void SetupQueues(Kernel &kernel);
	private:
		void AddNode(Kernel &kernel, xmlpp::Node* node);
		void AddQueue(Kernel &kernel, xmlpp::Node* node);
		class pimpl_t;
		pimpl_t *pimpl;
	};
}

#endif


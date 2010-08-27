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
#include "D4RTesterBase.h"
#include "D4RTestNodeBase.h"
#include "Variant.h"
#include "Assert.h"

namespace D4R {

    TesterBase::TesterBase()
        : success(true)
    {
    }

    TesterBase::~TesterBase() {
    }

    void TesterBase::Setup(const Variant &v) {
        Variant nodes = v["nodes"];
        Variant::ListIterator itr = nodes.ListBegin();
        while (itr != nodes.ListEnd()) {
            CreateNode(*itr);
            ++itr;
        }
        Variant queues = v["queues"];
        itr = queues.ListBegin();
        while (itr != queues.ListEnd()) {
            CreateQueue(*itr);
            ++itr;
        }
    }

    void TesterBase::Failure(TestNodeBase *tnb, const std::string &msg) {
        Error(msg.c_str());
        success = false;
        Abort();
    }
}


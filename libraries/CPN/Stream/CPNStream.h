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
 * \brief Base class for basic stream types.
 * \author John Bridgman
 */

#ifndef CPN_STREAM_H
#define CPN_STREAM_H
#pragma once

#include "CPNCommon.h"
#include "AsyncStream.h"
#include <vector>

namespace CPN {

    class Stream {
    public:
        virtual ~Stream() {};
        virtual void RegisterDescriptor(std::vector<Async::DescriptorPtr> &descriptors) = 0;
        virtual void RunOneIteration() = 0;
        virtual void SetDescriptor(Async::DescriptorPtr desc) = 0;
        virtual Key_t GetKey() const = 0;
    };
}

#endif


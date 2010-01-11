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
#ifndef CPN_RDBMT_H
#define CPN_RDBMT_H
#pragma once

namespace CPN {
    enum RDBMT_t {
        RDBMT_SETUP_HOST,
        RDBMT_DESTROY_HOST_KEY,

        RDBMT_GET_HOST_INFO,
        RDBMT_SIGNAL_HOST_START,

        RDBMT_CREATE_WRITER,
        RDBMT_CREATE_READER,
        RDBMT_CREATE_QUEUE,
        RDBMT_CREATE_NODE,

        RDBMT_CREATE_NODE_KEY,
        RDBMT_SIGNAL_NODE_START,
        RDBMT_SIGNAL_NODE_END,

        RDBMT_GET_NODE_INFO,

        RDBMT_GET_NUM_NODE_LIVE,

        RDBMT_GET_CREATE_READER_KEY,
        RDBMT_DESTROY_READER_KEY,

        RDBMT_GET_READER_INFO,

        RDBMT_GET_CREATE_WRITER_KEY,
        RDBMT_DESTROY_WRITER_KEY,

        RDBMT_GET_WRITER_INFO,

        RDBMT_CONNECT_ENDPOINTS,
        RDBMT_GET_READERS_WRITER,
        RDBMT_GET_WRITERS_READER,

        RDBMT_TERMINATE
    };
}
#endif

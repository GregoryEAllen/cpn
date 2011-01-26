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
#ifndef CPN_RCTXMT_H
#define CPN_RCTXMT_H
#pragma once

namespace CPN {
    /**
     * \brief RCTXMT Remote Context Message Type
     * These are the message types that the remote context uses to send information back and forth.
     */
    enum RCTXMT_t {
        /**
         * Commands for dealing with hosts.
         * @{
         */
        RCTXMT_SETUP_HOST,
        RCTXMT_GET_HOST_INFO,
        RCTXMT_SIGNAL_HOST_START,
        RCTXMT_SIGNAL_HOST_END,
        /** @} */

        /**
         * Commands to tell other hosts to create objects.
         * @{
         */
        RCTXMT_CREATE_WRITER,
        RCTXMT_CREATE_READER,
        RCTXMT_CREATE_QUEUE,
        RCTXMT_CREATE_NODE,
        /** @} */

        /**
         * Commands for creating nodes.
         * @}
         */
        RCTXMT_CREATE_NODE_KEY,
        RCTXMT_SIGNAL_NODE_START,
        RCTXMT_SIGNAL_NODE_END,
        RCTXMT_GET_NODE_INFO,
        RCTXMT_GET_NUM_NODE_LIVE,
        /** @} */

        /**
         * Commands for creating endpoints.
         * @}
         */
        RCTXMT_GET_CREATE_READER_KEY,
        RCTXMT_GET_READER_INFO,
        RCTXMT_GET_CREATE_WRITER_KEY,
        RCTXMT_GET_WRITER_INFO,
        RCTXMT_CONNECT_ENDPOINTS,
        RCTXMT_GET_READERS_WRITER,
        RCTXMT_GET_WRITERS_READER,
        /**
         * @}
         */

        /**
         * Context commands.
         * @{
         */
        RCTXMT_TERMINATE,
        RCTXMT_LOG
        /** @} */
    };
}
#endif

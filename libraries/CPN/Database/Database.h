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
 * \brief The Database abstract data type.
 * \author John Bridgman
 */
#ifndef CPNDB_DATABASE_H
#define CPNDB_DATABASE_H
#pragma once
#include "CPNCommon.h"
#include "KernelBase.h"
#include "Logger.h"
#include <string>

namespace CPN {

    class CPN_API Database : public LoggerOutput {
    public:

        static shared_ptr<Database> Local();

        virtual ~Database();

        virtual Key_t SetupHost(const std::string &name, const std::string &hostname,
                const std::string &servname, KernelBase *kmh) = 0;
        virtual Key_t GetHostKey(const std::string &host) = 0;
        virtual const std::string &GetHostName(Key_t hostkey) = 0;
        virtual void GetHostConnectionInfo(Key_t hostkey, std::string &hostname, std::string &servname) = 0;
        virtual void DestroyHostKey(Key_t hostkey) = 0;
        virtual Key_t WaitForHostStart(const std::string &host) = 0;
        virtual void SignalHostStart(Key_t hostkey) = 0;

        virtual void SendCreateWriter(Key_t hostkey, const SimpleQueueAttr &attr) = 0;
        virtual void SendCreateReader(Key_t hostkey, const SimpleQueueAttr &attr) = 0;
        virtual void SendCreateQueue(Key_t hostkey, const SimpleQueueAttr &attr) = 0;
        virtual void SendCreateNode(Key_t hostkey, const NodeAttr &attr) = 0;

        virtual Key_t CreateNodeKey(Key_t hostkey, const std::string &nodename) = 0;
        virtual Key_t GetNodeKey(const std::string &nodename) = 0;
        virtual const std::string &GetNodeName(Key_t nodekey) = 0;
        virtual Key_t GetNodeHost(Key_t nodekey) = 0;
        virtual void SignalNodeStart(Key_t nodekey) = 0;
        virtual void SignalNodeEnd(Key_t nodekey) = 0;

        /** Waits until the node starts and returns the key, if the node is
         * already started returns the key
         */
        virtual Key_t WaitForNodeStart(const std::string &nodename) = 0;
        virtual void WaitForNodeEnd(const std::string &nodename) = 0;
        virtual void WaitForAllNodeEnd() = 0;


        virtual Key_t GetCreateReaderKey(Key_t nodekey, const std::string &portname) = 0;
        virtual Key_t GetReaderNode(Key_t portkey) = 0;
        virtual Key_t GetReaderHost(Key_t portkey) = 0;
        virtual const std::string &GetReaderName(Key_t portkey) = 0;
        virtual void DestroyReaderKey(Key_t portkey) = 0;

        virtual Key_t GetCreateWriterKey(Key_t nodekey, const std::string &portname) = 0;
        virtual Key_t GetWriterNode(Key_t portkey) = 0;
        virtual Key_t GetWriterHost(Key_t portkey) = 0;
        virtual const std::string &GetWriterName(Key_t portkey) = 0;
        virtual void DestroyWriterKey(Key_t portkey) = 0;

        virtual void ConnectEndpoints(Key_t writerkey, Key_t readerkey) = 0;
        virtual Key_t GetReadersWriter(Key_t readerkey) = 0;
        virtual Key_t GetWritersReader(Key_t writerkey) = 0;
    };

}

#endif


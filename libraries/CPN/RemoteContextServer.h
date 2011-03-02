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
#ifndef CPN_REMOTECONTEXTSERVER_H
#define CPN_REMOTECONTEXTSERVER_H
#pragma once
#include "CPNCommon.h"
#include "Variant.h"

/*
    All requests have a field named 'type' which contains a number from RCTXMT_t.
    The basic form is:

    request = {
        "type"      : message type number,
        "msgid"     : (optional) an id that should be passed back with any reply,
        ... data members of the request ...
        dependent on type common ones are "key" and "name"
    }

    Replies will be of the form:

    reply = {
        "msgid"     : message id from request,
        "msgtype"   : "reply",
        "success"   : true or false,
        then the data under the name of the type of reply data
        "kernelinfo" or "nodeinfo" or "endpointinfo" etc.
    }

    There is another type of message called broadcast that is sent out by
    the server on certain events.

    broadcast = {
        "msgtype"   : "broadcast",
        then "kernelinfo" or "nodeinfo" etc.
        then some status variables like "numlivenodes"
    }

    The final type of message is the one that is for inter kernel communication
    these messages are simply routed to the kernel that should recieve it.

    kernel = {
        "msgtype"   : "kernel",
        "kernelkey" : the destination kernel key,
        "type"      : numberic message type,
        then "queueattr" or "nodeattr" or other data
    }

    the basic storage elements use these formats

    kernelinfo = {
        "key"       : key for the kernel,
        "name"      : name of the kernel,
        "hostname"  : name to use for other kernels to connect to this one,
        "servname"  : service name to use to connecto to this kernel,
        "live"      : true or false,
        "type"      : "endpointinfo",
        "client"    : the name of the RemoteContextClient that we are to send kernel message to
    }

    nodeinfo = {
        "key"       : the node key,
        "name"      : name of this node,
        "kernelkey" : the kernel key,
        "started"   : true or false,
        "dead"      : true or false,
        "endpoints" : {
            "endpointname" : key of endpoint,
            ...
        },
        "type"      : "endpointinfo"
    }

    endpointinfo = {
        "key"       : The key for this endpoint,
        "name"      : the name of this endpoint,
        "qname"     : the name of the queue if set and connected
        "nodekey"   : The key of the node this endpoint belongs to,
        "kernelkey" : the key of the kernel this node endpoint is on,
        "live"      : true or false
        "writerkey" : the key of the writer this endpoint is connected to if a reader,
        "readerkey" : the key of the reader this endpoint is connected to if a writer,
        "type"      : "endpointinfo"
    }

*/

namespace CPN {
    /** \brief the server for the remote context.
     *
     * Implementors only need to override SendMessage, BroadcastMessage, and LogMessage.
     * then call DispatchMessage for all messages from the clients.
     */
    class RemoteContextServer {
    public:
        RemoteContextServer();
        virtual ~RemoteContextServer();
        /**
         * Cause the context to go into the shutdown state.
         */
        virtual void Terminate();
        /**
         * \return whether or not the context is in the shutdown state.
         */
        bool IsTerminated() const { return shutdown; }
        int DebugLevel() const { return debuglevel; }
        int DebugLevel(int level) { return debuglevel = level; }
    protected:
        /**
         * \brief Process the given message.
         * \param sender a string id for the client
         * \param msg the message that the client sent
         */
        void DispatchMessage(const std::string &sender, const Variant &msg);
        /** \brief Send the given message to the given client.
         * \param recipient the id (previously used in DispatchMessage sender)
         * \param msg the message to be sent to the client
         */
        virtual void SendMessage(const std::string &recipient, const Variant &msg) = 0;
        /** \brief Broadcast a message to all clients
         * \param msg the message to send to all clients
         */
        virtual void BroadcastMessage(const Variant &msg) = 0;
        /** \brief Log a message
         * \param msg the message to be logged
         */
        virtual void LogMessage(const std::string &msg) = 0;

        void dbprintf(int level, const char *fmt, ...);
    private:

        void SetupKernel(const std::string &sender, const Variant &msg);
        void SignalKernelStart(const Variant &msg);
        void SignalKernelEnd(const Variant &msg);
        void GetKernelInfo(const std::string &sender, const Variant &msg);
        void CreateNodeKey(const std::string &sender, const Variant &msg);
        void SignalNodeStart(const Variant &msg);
        void SignalNodeEnd(const Variant &msg);
        void GetNodeInfo(const std::string &sender, const Variant &msg);
        void GetNumNodeLive(const std::string &sender, const Variant &msg);
        void GetCreateEndpointKey(const std::string &sender, const Variant &msg);
        void GetEndpointInfo(const std::string &sender, const Variant &msg);
        void ConnectEndpoints(const Variant &msg);

        Key_t NewKey();

        void RouteKernelMessage(const Variant &msg);
        Variant NewBroadcastMessage();

        typedef std::map<Key_t, Variant> DataMap;
        DataMap datamap;
        typedef std::map<std::string, Key_t> NameKeyMap;
        NameKeyMap kernelmap;
        NameKeyMap nodemap;

        int debuglevel;
        bool shutdown;
        unsigned numlivenodes;

        Key_t keycount;
    };
}
#endif

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
 * \brief The base definition of all nodes.
 * \author John Bridgman
 */

#ifndef CPN_NODEBASE_H
#define CPN_NODEBASE_H
#pragma once

#include "CPNCommon.h"
#include "NodeAttr.h"
#include "MessageQueue.h"
#include "NodeMessage.h"
#include "QueueBlocker.h"

#include "ReentrantLock.h"

#include "Pthread.h"

#include <map>

namespace CPN {

	/**
	 * \brief The definition common to all nodes in the process network.
	 *
	 * A node is a thread of execution which lasts the
	 * lifetime of the node object.
	 *
	 */
	class CPN_API NodeBase : private Pthread, private QueueBlocker, private NodeMsgDispatch {
        typedef std::map<Key_t, shared_ptr<QueueReader> > ReaderMap;
        typedef std::map<Key_t, shared_ptr<QueueWriter> > WriterMap;
        friend class Kernel;
	public:
		NodeBase(Kernel &ker, const NodeAttr &attr);

		virtual ~NodeBase();

		const std::string &GetName() const { return name; }

        const std::string &GetTypeName() const { return type; }

        Key_t GetKey() const { return nodekey; }

        shared_ptr<QueueReader> GetReader(const std::string &portname);
        shared_ptr<QueueWriter> GetWriter(const std::string &portname);

        Kernel *GetKernel() { return &kernel; }
	protected:

		virtual void Process() = 0;

		Kernel &kernel;
	private:
		void* EntryPoint();

        void Shutdown();

        shared_ptr<QueueReader> GetReader(Key_t ekey, bool create);
        shared_ptr<QueueWriter> GetWriter(Key_t ekey, bool create);
        void ProcessUntilMsgFrom(Key_t ekey);

        // From QueueBlocker
        void ReadNeedQueue(Key_t key);
        void WriteNeedQueue(Key_t key);
        void ReadBlock(shared_ptr<QueueReader> reader, unsigned thresh);
        void WriteBlock(shared_ptr<QueueWriter> writer, unsigned thresh);
        void ReleaseReader(Key_t ekey);
        void ReleaseWriter(Key_t ekey);
        shared_ptr<MsgPut<NodeMessagePtr> > GetMsgPut() { return msgqueue; }
        void CheckTerminate();

        void ProcessMessage(NodeSetReader *msg);
        void ProcessMessage(NodeSetWriter *msg);
        void ProcessMessage(NodeEnqueue *msg);
        void ProcessMessage(NodeDequeue *msg);
        void ProcessMessage(NodeReadBlock *msg);
        void ProcessMessage(NodeWriteBlock *msg);
        void ProcessMessage(NodeEndOfWriteQueue *msg);
        void ProcessMessage(NodeEndOfReadQueue *msg);

        // Private data
        Sync::ReentrantLock lock;
        const std::string name;
        const std::string type;
        const Key_t nodekey;
        shared_ptr<MsgQueue<NodeMessagePtr> > msgqueue;

        ReaderMap readermap;
        WriterMap writermap;

        shared_ptr<Database> database;

        bool terminated;
	};

}
#endif

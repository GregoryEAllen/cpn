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
#include "Message.h"

#include "ReentrantLock.h"

#include "Pthread.h"

#include <map>
#include <set>

namespace CPN {

	/**
	 * \brief The definition common to all nodes in the process network.
	 *
	 * A node is a thread of execution which lasts the
	 * lifetime of the node object.
	 *
	 */
	class CPN_API NodeBase
        : private Pthread,
        private NodeMessageHandler,
        private ReaderMessageHandler,
        private WriterMessageHandler
    {
        typedef std::map<Key_t, shared_ptr<QueueReader> > ReaderMap;
        typedef std::map<Key_t, shared_ptr<QueueWriter> > WriterMap;
        friend class Kernel;
	public:
		NodeBase(Kernel &ker, const NodeAttr &attr);

		virtual ~NodeBase();

        /** \return the name of this node
         */
		const std::string &GetName() const { return name; }

        /** \return the type name for this node
         */
        const std::string &GetTypeName() const { return type; }

        /** \return the unique key for this node
         */
        Key_t GetKey() const { return nodekey; }

        /** \breif used by the node to get an reader endpoint
         * \param portname the name of the reader endpoint
         * \return a shared_ptr to a QueueReader
         * \note This function blocks until the reader is set by the Kernel
         */
        shared_ptr<QueueReader> GetReader(const std::string &portname);
        /** \brief used by the node to get a writer endpoint
         * \param portname the name of the writer endpoint
         * \return a shared_ptr to a QueueWriter
         * \note this function blocks until the writer is set by the Kernel.
         */
        shared_ptr<QueueWriter> GetWriter(const std::string &portname);

        /** \brief Return a pointer to the kernel that his node is running under
         */
        Kernel *GetKernel() { return &kernel; }

        /// Internal do not use, will be removed later
        NodeMessageHandler *GetNodeMessageHandler() { return this; }
	protected:

		virtual void Process() = 0;

		Kernel &kernel;
	private:
		void* EntryPoint();


        shared_ptr<QueueReader> GetReader(Key_t ekey, bool block);
        shared_ptr<QueueWriter> GetWriter(Key_t ekey, bool block);
        void Block(Key_t ekey);
        void Unblock(Key_t ekey);

        void Shutdown();
        void CreateReader(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q);
        void CreateWriter(Key_t readerkey, Key_t writerkey, shared_ptr<QueueBase> q);
        void ReadBlock(Key_t readerkey, Key_t writerkey);
        void WriteBlock(Key_t writerkey, Key_t readerkey);
        void ReleaseReader(Key_t ekey);
        void ReleaseWriter(Key_t ekey);
        void CheckTerminate();

        void RMHEnqueue(Key_t writerkey, Key_t readerkey);
        void RMHEndOfWriteQueue(Key_t writerkey, Key_t readerkey);
        void RMHWriteBlock(Key_t writerkey, Key_t readerkey, unsigned requested);
        void RMHTagChange(Key_t writerkey, Key_t readerkey);

        void WMHDequeue(Key_t readerkey, Key_t writerkey);
        void WMHEndOfReadQueue(Key_t readerkey, Key_t writerkey);
        void WMHReadBlock(Key_t readerkey, Key_t writerkey, unsigned requested);
        void WMHTagChange(Key_t readerkey, Key_t writerkey);

        // Private data
        Sync::ReentrantLock lock;
        Sync::ReentrantCondition cond;
        const std::string name;
        const std::string type;
        const Key_t nodekey;

        ReaderMap readermap;
        WriterMap writermap;

        typedef std::set<Key_t> BlockSet;
        BlockSet blockset;

        shared_ptr<Database> database;

        bool terminated;

        Key_t blockkey;
	};

}
#endif

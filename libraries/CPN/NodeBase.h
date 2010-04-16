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
#include "QueueBase.h"
#include "NodeFactory.h"

#include "ReentrantLock.h"

#include "D4RNode.h"

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
	class CPN_API NodeBase : private Pthread, private QueueReleaser, private D4R::Node {
	public:
		NodeBase(Kernel &ker, const NodeAttr &attr);

		virtual ~NodeBase();

        /**
         * \return the unique name of this node
         */
		const std::string &GetName() const { return name; }

        /**
         * \return the type id of this node
         */
        const std::string &GetTypeName() const { return type; }

        /**
         * \return the process network wide unique id for this node
         */
        Key_t GetKey() const { return nodekey; }

        /**
         * \brief This method is for use by the user to aquire a reader endpoint.
         * This function blocks until the CPN::Kernel hands this node the queue
         * associated with the endpoint.
         * \param portname the port name of the reader to get.
         * \return a shared pointer to a reader for he given endpoint name
         */
        shared_ptr<QueueReader> GetReader(const std::string &portname);

        /**
         * \brief This method is for use by the user to aquire a writer endpoint.
         * This function blocks until the CPN::Kernel hands this node the queue
         * associated with the endpoint.
         * \param portname the port name fo the writer to get.
         * \return a shared pointer to a writer for the given endpoint name.
         */
        shared_ptr<QueueWriter> GetWriter(const std::string &portname);

        /** \brief Return a pointer to the kernel that his node is running under
         */
        Kernel *GetKernel() { return &kernel; }

        /**
         * \brief for use by the CPN::Kernel to create a new read endpoint.
         */
        void CreateReader(shared_ptr<QueueBase> q);

        /**
         * \brief for use by the CPN::Kernel to create a new writer endpoint.
         */
        void CreateWriter(shared_ptr<QueueBase> q);

        /**
         * \brief For use by the CPN::Kernel to start the node.
         */
        using Pthread::Start;

        /** \brief Called by the kernel when it is shutting down */
        void NotifyTerminate();

        /// For debugging ONLY!
        void LogState();
	protected:

        /** \brief Override this method to implement a node */
		virtual void Process() = 0;

		Kernel &kernel;
	private:
		void* EntryPoint();

        void SignalTagChanged();

        shared_ptr<QueueReader> GetReader(Key_t ekey);
        shared_ptr<QueueWriter> GetWriter(Key_t ekey);

        void ReleaseReader(Key_t ekey);
        void ReleaseWriter(Key_t ekey);

        // Private data
        Sync::ReentrantLock lock;
        Sync::ReentrantCondition cond;
        const std::string name;
        const std::string type;
        const Key_t nodekey;

        typedef std::map<Key_t, shared_ptr<QueueReader> > ReaderMap;
        typedef std::map<Key_t, shared_ptr<QueueWriter> > WriterMap;
        ReaderMap readermap;
        WriterMap writermap;

        shared_ptr<Database> database;
	};

}

#define CPN_DECLARE_NODE_FACTORY(type, klass) class type ## Factory : public CPN::NodeFactory {\
    public: type ## Factory() : CPN::NodeFactory(#type) {}\
    CPN::shared_ptr<CPN::NodeBase> Create(CPN::Kernel &ker, const CPN::NodeAttr &attr) { return CPN::shared_ptr<NodeBase>(new klass(ker, attr)); }};\
extern "C" CPN::shared_ptr<CPN::NodeFactory> cpninit ## type (void);\
CPN::shared_ptr<CPN::NodeFactory> cpninit ## type (void) { return CPN::shared_ptr<CPN::NodeFactory>(new type ## Factory); }

#endif

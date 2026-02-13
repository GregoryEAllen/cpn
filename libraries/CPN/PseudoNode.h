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
#pragma once

#include "CPNCommon.h"
#include "QueueBase.h"
#include "ReentrantLock.h"
#include <map>

namespace D4R {
    class Node;
}

namespace CPN {

    class CPN_API PseudoNode : private QueueReleaser {
    public:
        PseudoNode(const std::string &name_, Key_t k, shared_ptr<Context> ctx);
        virtual ~PseudoNode();

        /** \return the unique name of this node */
		const std::string &GetName() const { return name; }

        /** \return the process network wide unique id for this node */
        Key_t GetKey() const { return nodekey; }

        /**
         * \brief This method is for use by the user to aquire a reader endpoint.
         * This function blocks until the CPN::Kernel hands this node the queue
         * associated with the endpoint.
         * \param portname the port name of the reader to get.
         * \return a shared pointer to a reader for he given endpoint name
         */
        shared_ptr<QueueReader> GetIQueue(const std::string &portname);

        /**
         * \brief This method is for use by the user to aquire a writer endpoint.
         * This function blocks until the CPN::Kernel hands this node the queue
         * associated with the endpoint.
         * \param portname the port name fo the writer to get.
         * \return a shared pointer to a writer for the given endpoint name.
         */
        shared_ptr<QueueWriter> GetOQueue(const std::string &portname);

        /** \brief for use by the CPN::Kernel to create a new read endpoint. */
        void CreateReader(shared_ptr<QueueBase> q);
        /**\brief for use by the CPN::Kernel to create a new writer endpoint. */
        void CreateWriter(shared_ptr<QueueBase> q);
        /** \brief Called by the kernel when it is shutting down */
        void NotifyTerminate();
        /** \brief Perform actions (like joining a thread) before destruction */
        virtual void Shutdown();
        virtual bool IsPurePseudo();

        /// For debugging ONLY!
        virtual void LogState();
    protected:
        Logger logger;
    private:
        PseudoNode(const PseudoNode &);
        void ReleaseReader(Key_t ekey);
        void ReleaseWriter(Key_t ekey);
        shared_ptr<QueueReader> GetReader(Key_t ekey);
        shared_ptr<QueueWriter> GetWriter(Key_t ekey);


        Sync::ReentrantLock lock;
        Sync::ReentrantCondition cond;
        const std::string name;
        const Key_t nodekey;
        shared_ptr<D4R::Node> d4rnode;

        typedef std::map<Key_t, shared_ptr<QueueReader> > ReaderMap;
        typedef std::map<Key_t, shared_ptr<QueueWriter> > WriterMap;
        ReaderMap readermap;
        WriterMap writermap;

        shared_ptr<Context> context;
    };
}


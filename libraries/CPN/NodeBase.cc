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
 * \brief Implementation for the NodeBase class.
 * \author John Bridgman
 */

#include "NodeBase.h"
#include "Kernel.h"
#include "Exceptions.h"
#include "Database.h"
#include "D4RDeadlockException.h"
#include "ErrnoException.h"
#include "PthreadFunctional.h"

namespace CPN {

    NodeBase::NodeBase(Kernel &ker, const NodeAttr &attr)
    :   PseudoNode(attr.GetName(), attr.GetKey(), attr.GetDatabase()),
        kernel(ker),
        type(attr.GetTypeName())
    {
        thread.reset(CreatePthreadFunctional(this, &NodeBase::EntryPoint));
    }

    NodeBase::~NodeBase() {
    }

    void NodeBase::Start() {
        thread->Start();
    }

    void NodeBase::Shutdown() {
        PseudoNode::Shutdown();
        thread->Join();
    }

    void* NodeBase::EntryPoint() {
        try {
            kernel.GetDatabase()->SignalNodeStart(GetKey());
            Process();
        } catch (const CPN::ShutdownException &e) {
            // Forced shutdown
        } catch (const CPN::BrokenQueueException &e) {
            if (!kernel.GetDatabase()->SwallowBrokenQueueExceptions()) {
                throw;
            }
        } catch (const D4R::DeadlockException &e) {
            // A true deadlock was detected, die
            Logger logger(kernel.GetDatabase().get(), Logger::ERROR);
            logger.Name(GetName().c_str());
            logger.Info("DEADLOCK detected at %s\n", GetName().c_str());
        }
        kernel.NodeTerminated(GetKey());
        return 0;
    }


    bool NodeBase::IsPurePseudo() {
        return false;
    }

    void NodeBase::LogState() {
#if 0
        Logger logger(database.get(), Logger::ERROR);
        logger.Name(name.c_str());
        logger.Error("Logging (key: %llu), %u readers, %u writers, %s",
                nodekey, readermap.size(), writermap.size(), thread->Running() ? "Running" : "done");
        logger.Error("Thread id: %llu", (unsigned long long)((pthread_t)(*thread.get())));
        D4R::Tag tag = d4rnode->GetPrivateTag();
        logger.Error("Private key: (%llu, %llu, %llu, %llu)", tag.Count(), tag.Key(), tag.QueueSize(), tag.QueueKey());
        tag = d4rnode->GetPublicTag();
        logger.Error("Public key: (%llu, %llu, %llu, %llu)", tag.Count(), tag.Key(), tag.QueueSize(), tag.QueueKey());
        ReaderMap::iterator r = readermap.begin();
        while (r != readermap.end()) {
            r->second->GetQueue()->LogState();
            ++r;
        }
        WriterMap::iterator w = writermap.begin();
        while (w != writermap.end()) {
            w->second->GetQueue()->LogState();
            ++w;
        }
#endif
    }
}


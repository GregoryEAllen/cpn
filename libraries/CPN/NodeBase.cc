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
#include "Context.h"
#include "D4RDeadlockException.h"
#include "ErrnoException.h"
#include "PthreadFunctional.h"

namespace CPN {

    NodeBase::NodeBase(Kernel &ker, const NodeAttr &attr)
    :   PseudoNode(attr.GetName(), attr.GetKey(), ker.GetContext()),
        kernel(ker),
        type(attr.GetTypeName()),
        params(attr.GetParams())
    {
        thread.reset(CreatePthreadFunctional(this, &NodeBase::EntryPoint));
    }

    NodeBase::~NodeBase() {
    }

    std::string NodeBase::GetParam(const std::string &key) const {
        std::map<std::string, std::string>::const_iterator entry =
            params.find(key);
        if (entry == params.end()) {
            throw std::invalid_argument("Required parameter \"" + key + "\" missing"
                    " for node \"" + GetName() + "\"");
        }
        return entry->second;
    }

    bool NodeBase::HasParam(const std::string &key) const {
        std::map<std::string, std::string>::const_iterator entry =
            params.find(key);
        return entry != params.end();
    }

    void NodeBase::Start() {
        thread->Start();
    }

    void NodeBase::Shutdown() {
        thread->Join();
        PseudoNode::Shutdown();
    }

    void* NodeBase::EntryPoint() {
        try {
            kernel.GetContext()->SignalNodeStart(GetKey());
            Process();
        } catch (const CPN::ShutdownException &e) {
            // Forced shutdown
        } catch (const CPN::BrokenQueueException &e) {
            if (!kernel.SwallowBrokenQueueExceptions()) {
                throw;
            }
        } catch (const D4R::DeadlockException &e) {
            // A true deadlock was detected, die
            Logger logger(kernel.GetContext().get(), Logger::ERROR);
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
        PseudoNode::LogState();
        logger.Error("Thread id: %llu, %s", (unsigned long long)((pthread_t)(*thread.get())),
                (thread->Done() ? "done" : "running"));
    }
}


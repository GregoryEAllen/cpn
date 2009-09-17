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
 * \brief Definition of the queue attributes.
 * \author John Bridgman
 */

#ifndef CPN_QUEUEATTR_H
#define CPN_QUEUEATTR_H
#pragma once

#include "common.h"
#include "QueueDatatypes.h"
#include <string>

namespace CPN {
    
    /** \brief The attributes for a queue.
     *
     * The QueueAttr is used to encapsulate the information needed by the
     * Kernel to create a new connection between endpoints.
     */
    class CPN_API QueueAttr {
    public:
        QueueAttr(const unsigned queueLength_,
                const unsigned maxThreshold_)
            : queuehint(QUEUEHINT_DEFAULT), datatype(TypeName<void>()),
            queueLength(queueLength_), maxThreshold(maxThreshold_),
            numChannels(1),
            readerkey(0), writerkey(0), readernodekey(0), writernodekey(0)
            {}

        QueueAttr &SetReader(const std::string &nodename,
                const std::string &portname) {
            readernodename = nodename;
            readerportname = portname;
            return *this;
        }

        QueueAttr &SetReader(Key_t nodekey,
                const std::string &portname) {
            readernodekey = nodekey;
            readerportname = portname;
            return *this;
        }

        QueueAttr &SetWriter(const std::string &nodename,
                const std::string &portname) {
            writernodename = nodename;
            writerportname = portname;
            return *this;
        }

        QueueAttr &SetWriter(Key_t nodekey,
                const std::string &portname) {
            writernodekey = nodekey;
            writerportname = portname;
            return *this;
        }

        QueueAttr &SetHint(QueueHint_t hint) {
            queuehint = hint;
            return *this;
        }

        QueueAttr &SetDatatype(const std::string &type) {
            datatype = type;
            return *this;
        }

        QueueAttr &SetLength(unsigned length) {
            queueLength = length;
            return *this;
        }

        QueueAttr &SetMaxThreshold(unsigned maxthresh) {
            maxThreshold = maxthresh;
            return *this;
        }

        QueueAttr &SetReaderNodeKey(Key_t k) {
            readernodekey = k;
            return *this;
        }

        QueueAttr &SetWriterNodeKey(Key_t k) {
            writernodekey = k;
            return *this;
        }

        QueueAttr &SetReaderKey(Key_t k) {
            readerkey = k;
            return *this;
        }

        QueueAttr &SetWriterKey(Key_t k) {
            writerkey = k;
            return *this;
        }

        const std::string &GetWriterNode() const { return writernodename; }
        const std::string &GetWriterPort() const { return writerportname; }
        const std::string &GetReaderNode() const { return readernodename; }
        const std::string &GetReaderPort() const { return readerportname; }
        Key_t GetWriterNodeKey() const { return writernodekey; }
        Key_t GetReaderNodeKey() const { return readernodekey; }
        Key_t GetWriterKey() const { return writerkey; }
        Key_t GetReaderKey() const { return readerkey; }
        unsigned GetLength() const { return queueLength; }
        unsigned GetMaxThreshold() const { return maxThreshold; }
        unsigned GetNumChannels() const { return numChannels; }
        QueueHint_t GetHint() const { return queuehint; }
        const std::string &GetDatatype() const { return datatype; }

    private:
        QueueHint_t queuehint;
        std::string datatype;
        unsigned queueLength;
        unsigned maxThreshold;
        unsigned numChannels;
        std::string readernodename;
        std::string readerportname;
        std::string writernodename;
        std::string writerportname;
        Key_t readerkey;
        Key_t writerkey;
        Key_t readernodekey;
        Key_t writernodekey;
    };

    class SimpleQueueAttr {
    public:
        SimpleQueueAttr(const QueueAttr &attr)
            : queuehint(attr.GetHint()),
            datatype(attr.GetDatatype()),
            queueLength(attr.GetLength()),
            maxThreshold(attr.GetMaxThreshold()),
            numChannels(attr.GetNumChannels()),
            readerkey(attr.GetReaderKey()),
            writerkey(attr.GetWriterKey()),
            readernodekey(attr.GetReaderNodeKey()),
            writernodekey(attr.GetWriterNodeKey())
        {}

        SimpleQueueAttr &SetHint(QueueHint_t hint) {
            queuehint = hint;
            return *this;
        }

        SimpleQueueAttr &SetDatatype(const std::string &type) {
            datatype = type;
            return *this;
        }

        SimpleQueueAttr &SetLength(unsigned length) {
            queueLength = length;
            return *this;
        }

        SimpleQueueAttr &SetMaxThreshold(unsigned maxthresh) {
            maxThreshold = maxthresh;
            return *this;
        }

        SimpleQueueAttr &SetReaderNodeKey(Key_t k) {
            readernodekey = k;
            return *this;
        }

        SimpleQueueAttr &SetWriterNodeKey(Key_t k) {
            writernodekey = k;
            return *this;
        }

        SimpleQueueAttr &SetReaderKey(Key_t k) {
            readerkey = k;
            return *this;
        }

        SimpleQueueAttr &SetWriterKey(Key_t k) {
            writerkey = k;
            return *this;
        }


        Key_t GetWriterNodeKey() const { return writernodekey; }
        Key_t GetReaderNodeKey() const { return readernodekey; }
        Key_t GetWriterKey() const { return writerkey; }
        Key_t GetReaderKey() const { return readerkey; }
        unsigned GetLength() const { return queueLength; }
        unsigned GetMaxThreshold() const { return maxThreshold; }
        unsigned GetNumChannels() const { return numChannels; }
        QueueHint_t GetHint() const { return queuehint; }
        const std::string &GetDatatype() const { return datatype; }
    private:
        QueueHint_t queuehint;
        std::string datatype;
        unsigned queueLength;
        unsigned maxThreshold;
        unsigned numChannels;
        Key_t readerkey;
        Key_t writerkey;
        Key_t readernodekey;
        Key_t writernodekey;
    };
}
#endif

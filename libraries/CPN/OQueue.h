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
 * \brief Definition and implementation of an adaptor for
 * the CPN QueueWriter.
 * \author John Bridgman
 */

#ifndef CPN_QUEUE_QUEUEWRITERADAPTOR_H
#define CPN_QUEUE_QUEUEWRITERADAPTOR_H
#pragma once

#include "CPNCommon.h"
#include "QueueWriter.h"
#include "QueueDatatypes.h"
#include "Exceptions.h"

namespace CPN {
    /**
     * \brief A template class to do type conversion for the
     * writer end of the queue.
     */
    template<class T>
    class OQueue {
    public:
        OQueue() {}
        OQueue(shared_ptr<QueueWriter> q) : queue(q) {
            if (!TypeCompatable(TypeName<T>(), queue->GetDatatype())) {
                throw TypeMismatchException(TypeName<T>(), queue->GetDatatype());
            }
        }

        /**
         * Get an array to write into.
         * \param thresh the length of the array
         * \param chan the number of channel to get the array from.
         * \return the array
         */
        T* GetEnqueuePtr(unsigned thresh, unsigned chan=0) {
            return (T*) queue->GetRawEnqueuePtr(GetTypeSize<T>() * thresh, chan);
        }
        
        /**
         * Add count elements to the queue on all channels.
         * \param count the number of elements
         */
        void Enqueue(unsigned count) {
            queue->Enqueue(GetTypeSize<T>() * count);
        }

        /**
         * Enqueue the count elements in the array data into this queue.
         * \param data the array
         * \param count the number
         */
        void Enqueue(const T* data, unsigned count) {
            queue->RawEnqueue((void*)data, GetTypeSize<T>() * count);
        }

        /**
         * Enqueue the count elements in the 2D array into the channel.
         * \param data the array
         * \param count the number in each channel
         * \param numChans the number of channels
         * \param chanStride the number of elements between channels
         */
        void Enqueue(const T* data, unsigned count, unsigned numChans, unsigned chanStride) {
            queue->RawEnqueue((void*)data, GetTypeSize<T>() * count,
                    numChans, GetTypeSize<T>() * chanStride);
        }

        /// \return the number of channels
        unsigned NumChannels() const { return queue->NumChannels(); }
        /// \return the maximum threshold in data elements
        unsigned MaxThreshold() const { return queue->MaxThreshold()/GetTypeSize<T>(); }
        unsigned QueueLength() const { return queue->QueueLength()/GetTypeSize<T>(); }
        /// \return the amount of freespace in data elements
        unsigned Freespace() const { return queue->Freespace()/GetTypeSize<T>(); }
        /// \return the current channel stride, only call this after a successful call to GetEnqueuePtr.
        unsigned ChannelStride() const { return queue->ChannelStride()/GetTypeSize<T>(); }
        /// \return true if full
        bool Full() const { return queue->Full(); }
        /// \return the endpoint key
        Key_t GetKey() const { return queue->GetKey(); }
        /// \return the writer
        shared_ptr<QueueWriter> GetWriter() { return queue; }
        /// Release the writer, further operations are invalid
        void Release() { queue->Release(); queue.reset(); }

    private:
        shared_ptr<QueueWriter> queue;
    };
}
#endif

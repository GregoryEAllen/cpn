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
 * \brief Implementation and definition for QueueReader adapter.
 * \author John Bridgman
 */

#ifndef CPN_QUEUE_QUEUEREADERADAPTER_H
#define CPN_QUEUE_QUEUEREADERADAPTER_H
#pragma once

#include "common.h"
#include "QueueReader.h"
#include "QueueDatatypes.h"

namespace CPN {
    /** 
     * \brief Template class to do type conversion for reader end of the queue.
     */
    template<class T>
    class CPN_API QueueReaderAdapter {
    public:
        QueueReaderAdapter() {}
        QueueReaderAdapter(shared_ptr<QueueReader> q) : queue(q) {}

        /**
         * Get an array of the given type of the given length from the given channel
         * to read from.
         * \param thresh the size of the returned array.
         * \param chan the channel
         * \return the array
         */
        const T* GetDequeuePtr(unsigned thresh, unsigned chan=0) {
            return (T*) queue->GetRawDequeuePtr(GetTypeSize<T>() * thresh, chan);
        }

        /**
         * Dispose of count elements from the queue.
         * \param count the number to dispose of
         */
        void Dequeue(unsigned count) {
            queue->Dequeue(GetTypeSize<T>() * count);
        }

        /**
         * Read count element into the array data.
         * \param data the array
         * \param count the number of elements
         * \return true on success, false if the endpoint has shutdown
         */
        bool Dequeue(T* data, unsigned count) {
            return queue->RawDequeue((void*)data, GetTypeSize<T>() * count);
        }

        /**
         * Read count element into the 2D array data with the given number
         * of channels and channel stride.
         * \param data the array
         * \param count the number of elements in each channel
         * \param numChans the number of channels
         * \param chanStride the number of elements between each channel
         */
        bool Dequeue(T* data, unsigned count, unsigned numChans, unsigned chanStride) {
            return queue->RawDequeue((void*)data, GetTypeSize<T>() * count,
                    numChans, GetTypeSize<T>() * chanStride);
        }

        /// \return the number of channels
        unsigned NumChannels() const { return queue->NumChannels(); }
        /// \return the maximum threshold in bytes
        unsigned MaxThreshold() const { return queue->MaxThreshold(); }
        /// \return the number of bytes in the channel
        unsigned Count() const { return queue->Count(); }
        /// \return true if empty, false otherwise
        bool Empty() const { return queue->Empty(); }
        /// \return the endpoint key
        Key_t GetKey() const { return queue->GetKey(); }
        /// \return the underlying reader
        shared_ptr<QueueReader> GetReader() { return queue; }
        /// Release this reader all further actions are invalid
        void Release() { queue->Release(); queue.reset(); }
    private:
        shared_ptr<QueueReader> queue;
    };
}
#endif

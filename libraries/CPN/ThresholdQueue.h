/** \file
 * \brief Default threshold queue implementation.
 */

#ifndef CPN_QUEUE_THRESHOLDQUEUE_H
#define CPN_QUEUE_THRESHOLDQUEUE_H

#include "ThresholdQueueBase.h"
#include "QueueBase.h"
#include "PthreadMutex.h"
#include "PthreadCondition.h"
#include "common.h"

namespace CPN {

    /**
     * \brief A version of the ThresholdQueue that provides the
     * CPN Queue interface
     */
    class ThresholdQueue : public QueueBase {
    public:
        ThresholdQueue(const QueueAttr &attr);
        ~ThresholdQueue();

        // From QueueWriter
        void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);
        void Enqueue(ulong count);
        bool RawEnqueue(void* data, ulong count);
        bool RawEnqueue(void* data, ulong count, ulong numChans, ulong chanStride);
        ulong NumChannels(void) const;
        ulong Freespace(void) const;
        bool Full(void) const;

        // From QueueReader
        const void* GetRawDequeuePtr(ulong thresh, ulong chan=0);
        void Dequeue(ulong count);
        bool RawDequeue(void* data, ulong count);
        bool RawDequeue(void* data, ulong count, ulong numChans, ulong chanStride);
        //ulong NumChannels(void) const;
        ulong Count(void) const;
        bool Empty(void) const;

        // From QueueBase
        ulong ElementsEnqueued(void) const;
        ulong ElementsDequeued(void) const;

        ulong ChannelStride(void) const;

        static void RegisterQueueType(void);
    private:
        ThresholdQueueBase queue;
        mutable PthreadMutex qlock;
    };

}
#endif

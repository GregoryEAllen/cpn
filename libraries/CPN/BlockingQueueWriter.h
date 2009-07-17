/** \file
 * \brief BlockingQueueWriter
 */
#ifndef CPN_BLOCKINGQUEUEWRITER_H
#define CPN_BLOCKINGQUEUEWRITER_H

#include "common.h"
#include "NodeQueueWriter.h"
#include "QueueInfo.h"
#include "QueueStatus.h"

namespace CPN {

    class NodeInfo;

    /**
     * \brief Simple blocking implementation of NodeQueueWriter.
     *
     */
    class BlockingQueueWriter : public NodeQueueWriter {
    public:
        BlockingQueueWriter(NodeInfo* nodeinfo, const std::string &portname);

        ~BlockingQueueWriter();

        // From QueueWriter
        void* GetRawEnqueuePtr(ulong thresh, ulong chan=0);

        void Enqueue(ulong count);

        bool RawEnqueue(void* data, ulong count);

        bool RawEnqueue(void* data, ulong count, ulong numChans, ulong chanStride);

        ulong NumChannels(void) const;

        ulong Freespace(void) const;

        bool Full(void) const;

        const QueueDatatype* GetDatatype(void) const;
    };
}

#endif


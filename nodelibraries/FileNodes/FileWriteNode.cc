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
 * \author John Bridgman
 * A node which reads from a queue and writes to a file descriptor.
 */
#include "NodeBase.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "IQueue.h"
#include <string>
#include <unistd.h>

using namespace CPN;
using std::string;

class FileWriteNode : public NodeBase {
public:
    FileWriteNode(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr), fd(-1), input("input"), blocksize_set(false)
    {
        JSONToVariant p;
        p.Parse(attr.GetParam());
        ASSERT(p.Done(), "Error parsing param line %u column %u", p.GetLine(), p.GetColumn());
        Variant param = p.Get();
        ASSERT(!param["fd"].IsNull(), "FileReadNode must have fd parameter");
        fd = param["fd"].AsInt();
        if (!param["inport"].IsNull()) {
            input = param["inport"].AsString();
        }
        if (!param["blocksize"].IsNull()) {
            blocksize = param["blocksize"].AsUnsigned();
            blocksize_set = true;
        }
    }
private:
    void Process();
    int fd;
    string input;
    bool blocksize_set;
    unsigned blocksize;
};

CPN_DECLARE_NODE_FACTORY(FileWriteNode, FileWriteNode);

void FileWriteNode::Process() {
    IQueue<void> in = GetReader(input);
    try {
        while (true) {
            unsigned blksz;
            if (blocksize_set) {
                blksz = blocksize;
            } else {
                blksz = in.MaxThreshold();
            }
            const void *ptr = in.GetDequeuePtr(blksz);
            unsigned count = in.Count();
            if (count > 2*blksz || !ptr) {
                blksz = count;
                if (blksz == 0) break;
                in.Dequeue(0);
                ptr = in.GetDequeuePtr(blksz);
            }
            unsigned numwritten = write(fd, ptr, blksz);
            if (numwritten < 0) {
                break;
            }
            in.Dequeue(numwritten);
        }
    } catch (...) {
        close(fd);
        throw;
    }
    close(fd);
}



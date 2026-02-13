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
#include "IQueue.h"
#include <string>
#include <unistd.h>

using namespace CPN;
using std::string;

class FileWriteNode : public NodeBase {
public:
    FileWriteNode(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr)
    {
    }
private:
    void Process();
};

CPN_DECLARE_NODE_FACTORY(FileWriteNode, FileWriteNode);

void FileWriteNode::Process() {
    int fd = GetParam<int>("fd");
    bool blocksize_set = HasParam("blocksize");
    unsigned blocksize = GetParam<unsigned>("blocksize", 0);
    IQueue<void> in = GetIQueue("input");
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



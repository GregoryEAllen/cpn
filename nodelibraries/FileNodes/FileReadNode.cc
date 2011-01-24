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
 * A node which reads from a file descriptor and output the result to a queue.
 */
#include "NodeBase.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "OQueue.h"
#include <string>
#include <unistd.h>

using namespace CPN;
using std::string;

class FileReadNode : public NodeBase {
public:
    FileReadNode(Kernel &ker, const NodeAttr &attr)
        : NodeBase(ker, attr), fd(-1), output("output")
    {
        JSONToVariant p;
        p.Parse(attr.GetParam());
        ASSERT(p.Done(), "Error parsing param line %u column %u", p.GetLine(), p.GetColumn());
        Variant param = p.Get();
        ASSERT(!param["fd"].IsNull(), "FileReadNode must have fd parameter");
        fd = param["fd"].AsInt();
        if (!param["outport"].IsNull()) {
            output = param["outport"].AsString();
        }
    }
private:
    void Process();
    int fd;
    string output;
};

CPN_DECLARE_NODE_FACTORY(FileReadNode, FileReadNode);

void FileReadNode::Process() {
    OQueue<void> out = GetWriter(output);
    try {
        while (true) {
            unsigned blocksize = out.MaxThreshold();
            void *ptr = out.GetEnqueuePtr(blocksize);
            unsigned numread = read(fd, ptr, blocksize);
            if (numread < 1) {
                // Do something if error??
                break;
            }
            out.Enqueue(numread);
        }
    } catch (...) {
        close(fd);
    }
    close(fd);
}



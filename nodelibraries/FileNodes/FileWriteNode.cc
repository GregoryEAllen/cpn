
#include "NodeBase.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "QueueReaderAdapter.h"
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
    QueueReaderAdapter<void> in = GetReader(input);
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



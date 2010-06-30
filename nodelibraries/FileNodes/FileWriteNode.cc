
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
        : NodeBase(ker, attr), fd(-1), input("input")
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
    }
private:
    void Process();
    int fd;
    string input;
};

CPN_DECLARE_NODE_FACTORY(FileWriteNode, FileWriteNode);

void FileWriteNode::Process() {
    QueueReaderAdapter<void> in = GetReader(input);
    try {
        while (true) {
            unsigned blocksize = in.MaxThreshold();
            const void *ptr = in.GetDequeuePtr(blocksize);
            if (!ptr) {
                blocksize = in.Count();
                if (blocksize == 0) break;
                ptr = in.GetDequeuePtr(blocksize);
            }
            unsigned numwritten = write(fd, ptr, blocksize);
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



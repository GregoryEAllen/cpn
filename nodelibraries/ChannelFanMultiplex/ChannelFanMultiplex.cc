#include <NodeBase.h>
#include <IQueue.h>
#include <OQueue.h>
#include <string.h>
#include <vector>
#include <sstream>
#include <cassert>

using namespace CPN;

CPN_DECLARE_NODE_AND_FACTORY(ChannelFanMultiplex, ChannelFanMultiplex);

std::pair<int, int> ParsePair(std::istream &is) {
    int first, second;
    char c;
    while (is.get(c) && c != '(') {}
    is >> first;
    while (is.get(c) && c != ',') {}
    is >> second;
    while (is.get(c) && c != ')') {}
    is >> std::ws;
    return std::make_pair(first, second);
}

void ChannelFanMultiplex::Process() {
    int num_outputs = GetParam<int>("num outputs", 1);
    int num_inputs = GetParam<int>("num inputs", 1);
    unsigned blocksize = GetParam<unsigned>("blocksize", 1);
    unsigned overlap = GetParam<unsigned>("overlap", 0);
    std::vector<std::vector<std::pair<int, int> > > mapping(num_outputs);
    std::vector<IQueue<void> > inputs(num_inputs);
    std::vector<OQueue<void> > outputs(num_outputs);
    for (int i = 0; i < num_outputs; ++i) {
        std::ostringstream name_oss;
        name_oss << "out" << i;
        std::string name = name_oss.str();
        outputs[i] = GetOQueue(name);
        std::istringstream iss(GetParam(name));
        while (iss) {
            mapping[i].push_back(ParsePair(iss));
        }
        assert(inputs[i].NumChannels() >= mapping[i].size());
    }
    for (int i = 0; i < num_inputs; ++i) {
        std::ostringstream oss;
        oss << "in" << i;
        inputs[i] = GetIQueue(oss.str());
    }
    std::vector<const char *> iptrs(num_inputs);
    std::vector<unsigned> istrides(num_inputs);
    while (true) {
        for (int i = 0; i < num_inputs; ++i) {
            iptrs[i] = (const char*)inputs[i].GetDequeuePtr(blocksize);
            istrides[i] = inputs[i].ChannelStride();
        }
        for (int i = 0; i < num_outputs; ++i) {
            char *optr = (char *)outputs[i].GetEnqueuePtr(blocksize);
            unsigned ostride = outputs[i].ChannelStride();
            for (int j = 0, c = mapping[i].size(); j < c; ++j) {
                int idx = mapping[i][j].first;
                int chan = mapping[i][j].second;
                memcpy(optr+j*ostride, iptrs[idx] + chan*istrides[idx], blocksize);
            }
            outputs[i].Enqueue(blocksize);
        }
        for (int i = 0; i < num_inputs; ++i) {
            inputs[i].Dequeue(blocksize - overlap);
        }
    }
}


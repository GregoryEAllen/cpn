
#include "DNode.h"
#include "UUID.h"
#include <iostream>
#include <map>
#include <vector>
#include <algorithm>
#include <cassert>

typedef std::map<DHT::key_t, UUID> keyuuidmap;

int main(int argc, char **argv) {
    int numdata = 10000;
    int numnodes = 100;
    std::vector<DHT::DNode*> nodes;
    for (int i = 0; i < numnodes; ++i) {
        UUID nname = UUID::Generate();
        DHT::DNode* node = new DHT::DNode(nname.ToString());
        if (i != 0) {
            node->Connect(nodes.back());
        }
        node->Verify();
        nodes.push_back(node);
        //std::cout << "Created node id: " << node->GetID() << " Name: "  << node->GetName() << std::endl;
    }
    std::cout << "Created " << numnodes << " nodes." << std::endl;
    // Generate some random data.
    std::cout << "Adding random data." << std::endl;
    keyuuidmap data;
    for (int i = 0; i < numdata; ++i) {
        UUID id = UUID::Generate();
        DHT::key_t key = DHT::DNode::Hash(&id, sizeof(id));
        assert(data.find(key) == data.end());
        data.insert(std::make_pair(key, id));
        AutoBuffer buf(&id, sizeof(id));
        nodes[i%numnodes]->Put(key, buf);
        //std::cout << "Added id: " << key << " data: " << id.ToString() << std::endl; 
    }
    std::cout << "Added " << numdata << " elements." << std::endl;
    for (keyuuidmap::iterator itr = data.begin(); itr != data.end(); ++itr) {
        AutoBuffer buf = nodes[0]->Get(itr->first);
        UUID id;
        buf.Get(0, &id);
        //std::cout << "Get id: " << itr->first << " data: " << itr->second.ToString() << " Got: " << id.ToString() << std::endl;
        assert(id == itr->second);
    }
    std::cout << "All elements retreved." << std::endl;
	return 0;
}


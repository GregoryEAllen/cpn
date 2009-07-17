/** \file
 */

#include "KernelXMLLoader.h"
#include "Kernel.h"
#include <string>
#include <sstream>
#include <memory>
#include <cassert>
#include <libxml++/libxml++.h>
#include <iostream>

using namespace xmlpp;

namespace CPN {
	class KernelXMLLoader::pimpl_t {
	public:
		DomParser parser;
		Document* doc;
		Node* root;
	};
}

const char* const TAG_ID = "id";
const char* const TAG_NAME = "name";
const char* const TAG_NODES = "nodes";
const char* const TAG_NODE = "node";
const char* const TAG_QUEUES = "queues";
const char* const TAG_QUEUE = "queue";
const char* const TAG_SIZE = "size";
const char* const TAG_THRESHOLD = "maxThreshold";
const char* const TAG_CHANNELS = "channels";
const char* const TAG_WRITER = "writer";
const char* const TAG_READER = "reader";
const char* const TAG_PORT = "port";
const char* const TAG_TYPE = "type";
const char* const TAG_PARAM = "param";


std::string GetContent(Node* node) {
	std::string content;
	Node::NodeList c = node->get_children();
	assert(c.size() == 1);
	ContentNode* n = dynamic_cast<ContentNode*>(c.front());
	assert(n);
	content = n->get_content();
	return content;
}

void PortInfo(Node* node, std::string &name, std::string &port) {
	Node::NodeList list = node->get_children(TAG_NODE);
	assert(list.size() == 1);
	name = GetContent(list.front());
	list = node->get_children(TAG_PORT);
	assert(list.size() == 1);
	port = GetContent(list.front());
	//std::cout << "Port: " << name << "." << port << std::endl;
}

CPN::KernelXMLLoader::KernelXMLLoader(const std::string &filename) {
	std::auto_ptr<pimpl_t> p = std::auto_ptr<pimpl_t>(new pimpl_t);
	DomParser &parser = p->parser;
	parser.set_substitute_entities();
	parser.parse_file(filename);
	if (parser) {
		p->doc = parser.get_document();
		p->root = p->doc->get_root_node();
	}
	pimpl = p.release();
}

CPN::KernelXMLLoader::~KernelXMLLoader() {
	delete pimpl;
	pimpl = 0;
}

CPN::KernelAttr CPN::KernelXMLLoader::GetKernelAttr(void) {
	int state = 0;
	std::string name;
	unsigned long id = 0;
	Node::NodeList list = pimpl->root->get_children();
	for (Node::NodeList::iterator itr = list.begin();
			itr != list.end() && state < 2; itr++) {
		const std::string tagname = (*itr)->get_name();
		if (tagname == TAG_NAME) {
			name = GetContent(*itr);
			++state;
		} else if (tagname == TAG_ID) {
			std::istringstream iss(GetContent(*itr));
			iss >> id;
			++state;
		}
	}
	std::cout << "id: " << id << " name: " << name << std::endl;
	return CPN::KernelAttr(id, name);
}

void CPN::KernelXMLLoader::SetupNodes(Kernel &kernel) {
	Node::NodeList nodelists = pimpl->root->get_children(TAG_NODES);
	for (Node::NodeList::iterator itr = nodelists.begin();
			itr != nodelists.end(); itr++) {
		Node::NodeList nodes = (*itr)->get_children();
		for (Node::NodeList::iterator nitr = nodes.begin();
				nitr != nodes.end(); nitr++) {
			AddNode(kernel, *nitr);
		}
	}
}

void CPN::KernelXMLLoader::AddNode(Kernel &kernel, Node* node) {
	if (node->get_name() != TAG_NODE) return;
	std::string name;
	std::string type;
	std::string param;
	Node::NodeList list = node->get_children();
	for (Node::NodeList::iterator itr = list.begin();
			itr != list.end(); itr++) {
		std::string tagname = (*itr)->get_name();
		if (tagname == TAG_NAME) {
			name = GetContent(*itr);
		} else if (tagname == TAG_TYPE) {
			type = GetContent(*itr);
		} else if (tagname == TAG_PARAM) {
			param = GetContent(*itr);
		}
	}
	kernel.CreateNode(name, type, param);
	std::cout << "name: " << name << " type: " << type << " param: " << param << std::endl;
}

void CPN::KernelXMLLoader::SetupQueues(Kernel &kernel) {
	Node::NodeList qlists = pimpl->root->get_children(TAG_QUEUES);
	for (Node::NodeList::iterator itr = qlists.begin();
			itr != qlists.end(); itr++) {
		Node::NodeList queues = (*itr)->get_children();
		for (Node::NodeList::iterator qitr = queues.begin();
				qitr != queues.end(); qitr++) {
			AddQueue(kernel, *qitr);
		}
	}
}


void CPN::KernelXMLLoader::AddQueue(Kernel &kernel, Node* node) {
	if (node->get_name() != TAG_QUEUE) return;
	std::string name;
	std::string type;
	unsigned long size;
	unsigned long threshold;
	unsigned long channels;
	std::string writernode;
	std::string writerport;
	std::string readernode;
	std::string readerport;
	Node::NodeList list = node->get_children();
	for (Node::NodeList::iterator itr = list.begin();
			itr != list.end(); itr++) {
		std::string tagname = (*itr)->get_name();
		if (tagname == TAG_NAME) {
			name = GetContent(*itr);
		} else if (tagname == TAG_TYPE) {
			type = GetContent(*itr);
		} else if (tagname == TAG_SIZE) {
			std::istringstream iss(GetContent(*itr));
			iss >> size;
		} else if (tagname == TAG_THRESHOLD) {
			std::istringstream iss(GetContent(*itr));
			iss >> threshold;
		} else if (tagname == TAG_CHANNELS) {
			std::istringstream iss(GetContent(*itr));
			iss >> channels;
		} else if (tagname == TAG_WRITER) {
			PortInfo(*itr, writernode, writerport);
		} else if (tagname == TAG_READER) {
			PortInfo(*itr, readernode, readerport);
		}
	}
	kernel.CreateQueue(name, type, size, threshold, channels);
	kernel.ConnectWriteEndpoint(name, writernode, writerport);
	kernel.ConnectReadEndpoint(name, readernode, readerport);
	std::cout << "name: " << name << " type: " << type << " size: " << size
		<< " maxThreshold: " << threshold << " channels: " << channels
		<< " writernode: " << writernode << "." << writerport
		<< " readernode: " << readernode << "." << readerport << std::endl;
}



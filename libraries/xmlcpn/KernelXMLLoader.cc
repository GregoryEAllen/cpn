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
 */

#include "KernelXMLLoader.h"
#include "Kernel.h"
#include "Assert.h"
#include <string>
#include <sstream>
#include <memory>
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

const char TAG_NAME[] = "name";
const char TAG_NODES[] = "nodes";
const char TAG_NODE[] = "node";
const char TAG_QUEUES[] = "queues";
const char TAG_QUEUE[] = "queue";
const char TAG_SIZE[] = "size";
const char TAG_THRESHOLD[] = "maxThreshold";
const char TAG_CHANNELS[] = "channels";
const char TAG_WRITER[] = "writer";
const char TAG_READER[] = "reader";
const char TAG_PORT[] = "port";
const char TAG_TYPE[] = "type";
const char TAG_PARAM[] = "param";
const char TAG_DATATYPE[] = "datatype";


std::string GetContent(Node* node) {
	std::string content;
	Node::NodeList c = node->get_children();
	ASSERT(c.size() == 1);
	ContentNode* n = dynamic_cast<ContentNode*>(c.front());
	ASSERT(n);
	content = n->get_content();
	return content;
}

void PortInfo(Node* node, std::string &name, std::string &port) {
	Node::NodeList list = node->get_children(TAG_NODE);
	ASSERT(list.size() == 1);
	name = GetContent(list.front());
	list = node->get_children(TAG_PORT);
	ASSERT(list.size() == 1);
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
	std::string name;
	Node::NodeList list = pimpl->root->get_children();
	for (Node::NodeList::iterator itr = list.begin();
			itr != list.end(); itr++) {
		const std::string tagname = (*itr)->get_name();
		if (tagname == TAG_NAME) {
			name = GetContent(*itr);
            break;
		}
	}
	std::cout << " name: " << name << std::endl;
	return CPN::KernelAttr(name);
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
	kernel.CreateNode(CPN::NodeAttr(name, type).SetParam(param));
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
    CPN::QueueAttr attr;
	Node::NodeList list = node->get_children();
	for (Node::NodeList::iterator itr = list.begin();
			itr != list.end(); itr++) {
		std::string tagname = (*itr)->get_name();
		if (tagname == TAG_DATATYPE) {
            attr.SetDatatype(GetContent(*itr));
		} else if (tagname == TAG_SIZE) {
			std::istringstream iss(GetContent(*itr));
            unsigned size;
			iss >> size;
            attr.SetLength(size);

		} else if (tagname == TAG_THRESHOLD) {
			std::istringstream iss(GetContent(*itr));
            unsigned threshold;
			iss >> threshold;
            attr.SetMaxThreshold(threshold);
		} else if (tagname == TAG_CHANNELS) {
			std::istringstream iss(GetContent(*itr));
            unsigned channels;
			iss >> channels;
            attr.SetNumChannels(channels);
		} else if (tagname == TAG_WRITER) {
            Node::NodeList name = (*itr)->get_children(TAG_NODE);
            Node::NodeList port = (*itr)->get_children(TAG_PORT);
            ASSERT(name.size() == 1);
            ASSERT(port.size() == 1);
            attr.SetWriter(GetContent(name.front()), GetContent(port.front()));
		} else if (tagname == TAG_READER) {
            Node::NodeList name = (*itr)->get_children(TAG_NODE);
            Node::NodeList port = (*itr)->get_children(TAG_PORT);
            ASSERT(name.size() == 1);
            ASSERT(port.size() == 1);
            attr.SetReader(GetContent(name.front()), GetContent(port.front()));
		}
	}
	kernel.CreateQueue(attr);
}



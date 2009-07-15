/** \file
 */

#include "KernelXMLLoader.h"
#include "Kernel.h"
#include <string>
#include <libxml++/libxml++.h>


CPN::Kernel* CPN::CreateKernelFromXML(const std::string& filepath) {
	xmlpp::DomParser parser;
	parser.set_validate();
	parser.set_substitute_entities();
	parser.parse_file(filepath);
	if (parser) {
		xmlpp::Document* doc = parser.get_document();
		xmlpp::Node* root = doc->get_root_node();
	}
}


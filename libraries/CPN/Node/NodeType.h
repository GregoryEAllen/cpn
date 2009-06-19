/** \file
 * \brief A simple structure that defines all the information
 * needed to load a node dynamically.
 */
#ifndef CPN_NODETYPE_H
#define CPN_NODETYPE_H
#include <string>
namespace CPN {
	struct NodeType {
		/// Name of the nodetype to use.
		const ::std::string nodetypename;
		/// Name of the library to load.
		const char* libname;
		/// The symbol name of the factory to produce new nodes
		const char* factoryname;
		/// symbol name of the function to destroy old nodes
		const char* destructorname;
	};
}
#endif

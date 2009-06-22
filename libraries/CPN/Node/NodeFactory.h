/** \file
 * \brief Definition of the NodeFactory.
 */
#ifndef CPN_NODEFACTORY_H
#define CPN_NODEFACTORY_H

#include "PthreadMutex.h"
#include <string>
#include <map>

namespace CPN {

	class NodeBase;
	class NodeAttr;
	class Kernel;

	/**
	 * All derived classes must implement copy semantics.
	 */
	class NodeFactory {
	public:
		NodeFactory(const ::std::string &name_);

		virtual ~NodeFactory();

		virtual NodeBase* Create(Kernel &ker, const NodeAttr &attr, const void* const arg,
				const ulong argsize) = 0;

		virtual void Destroy(NodeBase* node) = 0;

		static NodeFactory* GetFactory(const ::std::string &name);

	private:
		const ::std::string name;
	};

}

#endif


/** \file
 * \brief Definition of the NodeFactory.
 */
#ifndef CPN_NODEFACTORY_H
#define CPN_NODEFACTORY_H

namespace CPN {

	class NodeBase;
	class NodeAttr;

	/**
	 * All derived classes must implement copy semantics.
	 */
	class NodeFactory {
	public:

		virtual NodeBase* Create(Kernel &ker, const NodeAttr &attr, const void* const arg,
				const ulong argsize);

		virtual void Destroy(NodeBase* node);
	};

}

#endif


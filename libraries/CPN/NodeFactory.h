/** \file
 * \brief Definition of the NodeFactory.
 */
#ifndef CPN_NODEFACTORY_H
#define CPN_NODEFACTORY_H

#include "common.h"
#include <string>

namespace CPN {

	class NodeBase;
	class NodeAttr;
	class Kernel;

	/**
	 * \brief The node factory provides a method for the kernel to
	 * create arbitrary user defined Nodes.
	 */
	class CPN_API NodeFactory {
	public:
		NodeFactory(const ::std::string &name_);

		virtual ~NodeFactory();

		/**
		 * Create a new node of the specific type.
		 *
		 * Upon the return of this function deletion of the
		 * memory arg points to should be valid.
		 *
		 * By default will call Create(ker, attr)
		 *
		 * \param ker the kernel
		 * \param attr attributes for the new node
		 * \param arg a pointer to a field of arguments (may be 0)
		 * \param argsize the size of the arguments
		 * \return a pointer to a concreate node object
		 */
		virtual NodeBase* Create(Kernel &ker, const NodeAttr &attr,
			       	const void* const arg,
				const ulong argsize);
		/**
		 * Create a new node for the specific type.
		 * \param ker the kernel
		 * \param attr attributes for the new node
		 * \return a pointer to a concreate node object
		 */
		virtual NodeBase* Create(Kernel &ker, const NodeAttr &attr) = 0;

		/**
		 * Create a new node for the specific type.
		 * \param ker the kernel
		 * \param attr attributes for the new node
		 * \param param a parameter string
		 * \return a pointer to a concreate node object
		 */
		virtual NodeBase* Create(Kernel &ker, const NodeAttr &attr,
			       	const std::string &param);

		/**
		 * Free the memory allocated by Create.
		 *
		 * \note This function is provided because it is possible that
		 * the node implementation library uses a different heap library
		 * than the rest of the application!
		 *
		 * \param node pointer to the node returned by Create.
		 */
		virtual void Destroy(NodeBase* node) = 0;

		/**
		 * Get the node type name for this factory.
		 * \return the type name
		 */
		const ::std::string &GetName(void) const { return name; }

	private:
		const ::std::string name;
	};

}

extern "C" {
	/**
	 * Return the a pointer to the factory for the given node type.
	 * \warning all factories must have static program lifetime.
	 * \param ntypename the name of the node type
	 * \return a pointer to an implementation of this class
	 */
	CPN::NodeFactory* CPN_API CPNGetNodeFactory(const ::std::string &ntypename);

	/**
	 * Add the given node to the NodeFactory Registry.
	 * If one adds different factory with the same name
	 * it will replace the current factory by that name.
	 * \warning all factories must have static program lifetime.
	 * \param fact pointer to the factory to add.
	 */
	void CPN_API CPNRegisterNodeFactory(CPN::NodeFactory* fact);

	/**
	 * Remove the factory with the given name from the registry.
	 * It is alright to remove the same node factory more than once.
	 * \warning all factories must have static program lifetime.
	 * \param ntypename the name of the factory
	 */
	void CPN_API CPNUnregisterNodeFactory(const ::std::string &ntypename);
}

#endif


/** \file
 * \brief Definition of the NodeFactory.
 */
#ifndef CPN_NODEFACTORY_H
#define CPN_NODEFACTORY_H

#include <string>

namespace CPN {

	class NodeBase;
	class NodeAttr;
	class Kernel;

	/**
	 * The node factory provides a method for the kernel to
	 * create arbitrary user defined Nodes.
	 *
	 * This class should be used by defining a class that inherits this
	 * class and implements the virtual functions. Then define a
	 * static global variable of this class in the implementation
	 * of the node this factory is for.
	 */
	class NodeFactory {
	public:
		NodeFactory(const ::std::string &name_);

		virtual ~NodeFactory();

		/**
		 * Create a new node of the specific type.
		 *
		 * Upon the return of this function deletion of the
		 * memory arg points to should be valid.
		 *
		 * \param ker the kernel
		 * \param attr attributes for the new node
		 * \param arg a pointer to a field of arguments (may be 0)
		 * \param argsize the size of the arguments (may be 0)
		 * \return a pointer to a concreate node object
		 */
		virtual NodeBase* Create(Kernel &ker, const NodeAttr &attr, const void* const arg,
				const ulong argsize) = 0;

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

		/**
		 * Return the a pointer to the factory for the given node type.
		 * \param name the name of the node type
		 * \return a pointer to an implementation of this class
		 */
		static NodeFactory* GetFactory(const ::std::string &ntypename);

	private:
		const ::std::string name;
	};

}

#endif


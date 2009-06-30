/** \file
 * The base definition of all nodes.
 */

#ifndef CPN_NODEBASE_H
#define CPN_NODEBASE_H

#include "Pthread.h"
#include "NodeAttr.h"


namespace CPN {

	// Forward declarations
	class Kernel;
	/**
	 * \brief The definition common to all nodes in the process network.
	 *
	 * A node is a thread of execution which lasts the
	 * lifetime of the node object.
	 *
	 */
	class NodeBase: public Pthread {
	public:
		NodeBase(Kernel &ker, const NodeAttr &attr) :
			kernel(ker), attr(attr) {
		}

		virtual ~NodeBase() {
		}

		const NodeAttr &GetAttr(void) const { return attr; }

		virtual void Process(void) = 0;
	protected:
		Kernel &kernel;
	private:
		void* EntryPoint(void);

		const NodeAttr attr;
	};

}
#endif

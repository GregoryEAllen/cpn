/** \file
 * The base definition of all nodes.
 */

#ifndef CPN_NODEBASE_H
#define CPN_NODEBASE_H

#include "Pthread.h"
#include "NodeAttr.h"
#include <string>


namespace ::CPN {
	// Forward declarations
	class QueueWriter;
	class QueueReader;
	class Kernel;
	/**
	 * \brief The definition common to all nodes in the process network.
	 *
	 * A node is a thread of execution which lasts the
	 * lifetime of the node object.
	 */
	class NodeBase: public Pthread {
	public:
		explicit NodeBase(Kernel &ker, NodeAttr &attr) : kernel(ker), attr(attr) {}
		/**
		 * \brief Assign the given writer queue end to the given port name.
		 */
		void ConnectQueue(::std::string portname, QueueWriter &qwriter);
		/**
		 * \brief Assign the given reader queue end to the given port name.
		 */
		void ConnectQueue(::std::string portname, QueueReader &qreader);

		virtual void Process(void);
	protected:
		/**
		 * \brief Get the writer queue for the given port name.
		 */
		QueueWriter &GetOutput(::std::string portname);
		/**
		 * \brief Get the writer queue for the given port name.
		 */
		QueueReader &GetInput(::std::string portname);
	private:
		void* EntryPoint(void);

		Kernel &kernel;
		NodeAttr attr;
	};
}
#endif

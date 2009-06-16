/** \file
 * The base definition of all nodes.
 */

#ifndef CPN_NODEBASE_H
#define CPN_NODEBASE_H

#include "Pthread.h"
#include "NodeAttr.h"
#include <string>


namespace CPN {

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
		void ConnectQueue(const ::std::string &portname, QueueWriter &qwriter);
		/**
		 * \brief Assign the given reader queue end to the given port name.
		 */
		void ConnectQueue(const ::std::string &portname, QueueReader &qreader);

		const NodeAttr &GetAttr(void) const { return attr; }

	protected:
		virtual void Process(void) = 0;

		/**
		 * \brief Get the writer queue for the given port name.
		 */
		QueueWriter &GetOutput(const ::std::string &portname) const;
		/**
		 * \brief Get the writer queue for the given port name.
		 */
		QueueReader &GetInput(const ::std::string &portname) const;
	private:
		void* EntryPoint(void);

		Kernel &kernel;

		const NodeAttr attr;
	};
}
#endif

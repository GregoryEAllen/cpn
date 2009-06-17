/** \file
 * The base definition of all nodes.
 */

#ifndef CPN_NODEBASE_H
#define CPN_NODEBASE_H

#include "Pthread.h"
#include "NodeAttr.h"
#include <string>
#include <map>


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
	 *
	 * \warning Initial implementation requires that the Connect*
	 * commands be called before the node is started and must not
	 * be called after the node is started.
	 */
	class NodeBase: public Pthread {
	public:
		NodeBase(Kernel &ker, NodeAttr &attr) : kernel(ker), attr(attr) {}
		/**
		 * \brief Assign the given writer queue end to the given port name.
		 */
		void ConnectWriter(const ::std::string &portname, QueueWriter *qwriter);
		/**
		 * \brief Assign the given reader queue end to the given port name.
		 */
		void ConnectReader(const ::std::string &portname, QueueReader *qreader);

		/**
		 * \brief Get the writer queue for the given port name.
		 */
		QueueWriter *GetWriter(const ::std::string &portname);
		/**
		 * \brief Get the writer queue for the given port name.
		 */
		QueueReader *GetReader(const ::std::string &portname);

		const NodeAttr &GetAttr(void) const { return attr; }

		virtual void Process(void) = 0;
	protected:
	private:
		virtual void* EntryPoint(void);

		Kernel &kernel;

		const NodeAttr attr;

		std::map<std::string, QueueWriter*> writerqlist;
		std::map<std::string, QueueReader*> readerqlist;
	};
}
#endif

//=============================================================================
//	$Id: CPNNodeDescriptor.h 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// CPNNodeDescriptor
//=============================================================================


#ifndef CPNNodeDescriptor_h
#define CPNNodeDescriptor_h


#include "CPNDefs.h"
#include <string>

class CPNNodeDescriptor{
	public:
		// Constructors & Destructor
		CPNNodeDescriptor(const std::string &theNodeName=EMPTY, const std::string &theTypeName=EMPTY, 
			const HostID &theHostID=NaN, const std::string &theInQ=EMPTY, const std::string &theOutQ=EMPTY);
		CPNNodeDescriptor(const CPNNodeDescriptor &toCopy);
		~CPNNodeDescriptor();

		// Update Methods
		int SetHostID(const HostID &theID){hostID = theID;}
		int SetInputQ(const std::string &qName) {inQ=qName;}
		int SetOutputQ(const std::string &qName) {outQ=qName;}
		
		// Accessor methods
		const std::string GetNodeName() const {return nodeName;}
		const std::string GetTypeName() const {return typeName;}
		const int GetHostID()		const {return hostID;}
		const std::string GetInQ()	const {return inQ;}
		const std::string GetOutQ()	const {return outQ;}
		
		// Debugging aides
		void Print(void);
		void Debug(const std::string &msg);

	private:
		// Data members
		std::string nodeName, typeName;
		HostID hostID;
		std::string inQ, outQ;
};


#endif

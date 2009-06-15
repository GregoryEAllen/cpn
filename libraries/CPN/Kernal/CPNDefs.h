//=============================================================================
//	$Id: CPNHDefs.h 1.0 2006/06/12 zucknick Exp $
//-----------------------------------------------------------------------------
// CPNDefs
//=============================================================================


#ifndef CPNDefs_h
#define CPNDefs_h


#include <stdio.h>
#include <stdlib.h>

#define DEBUG 0

//-----------------------------------------------------------------------------
//	flags and definitions for debugging printouts
//-----------------------------------------------------------------------------

#define EMPTY "NIL"
#define SUCCESS 1
#define FAILURE -1
#define NaN 999999999
#define NODEBLOCKLEN 20


typedef unsigned long HostID;
typedef unsigned long NodeID;


enum TypeID {BOOL, CHAR, INT, SHORT, LONG, FLOAT, DOUBLE};


#endif

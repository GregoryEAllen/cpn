//=============================================================================
//	PthreadFunctional (higher order function) class
//-----------------------------------------------------------------------------
//	POSIX Pthread class library
//	Copyright (C) 1997-1999  The University of Texas
//
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================

#ifndef PthreadFunctional_h
#define PthreadFunctional_h
#pragma once

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS

#include "PthreadLib.h"

//-----------------------------------------------------------------------------
//	The (virtual) base class
class PthreadFunctional : public Pthread {
  public:
	PthreadFunctional(void) : Pthread() { }
	PthreadFunctional(const PthreadAttr& attr) : Pthread(attr) { }
  private:
	virtual void* EntryPoint(void) = 0;
};
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//	templatized class (not intended to be used)
template <class T>
class PthreadFunctionalTemplate : public PthreadFunctional {
  public:
	PthreadFunctionalTemplate( T& obj, void* (T::*meth)(void) )
		: theObject(obj), theMethod(meth) { }
	PthreadFunctionalTemplate( T& obj, void* (T::*meth)(void), const PthreadAttr& attr)
		: PthreadFunctional(attr), theObject(obj), theMethod(meth) { }
  private:
	T& theObject;
	void* (T::*theMethod)(void);
	void* EntryPoint(void) { return (theObject.*theMethod)(); }
};
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
//	the proper way to create a PthreadFunctional (remember to delete!)
template <class T>
PthreadFunctional* CreatePthreadFunctional(T* obj, void* (T::*meth)(void))
{	return new PthreadFunctionalTemplate<T>(*obj, meth);	}
//-----------------------------------------------------------------------------
template <class T>
PthreadFunctional* CreatePthreadFunctional(T* obj, void* (T::*meth)(void), const PthreadAttr& attr)
{	return new PthreadFunctionalTemplate<T>(*obj, meth, attr);	}
//-----------------------------------------------------------------------------


#endif
#endif

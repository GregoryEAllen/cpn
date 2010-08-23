//=============================================================================
//	$Id: s.PthreadErrorHandler.h 1.8 03/11/07 17:23:42-06:00 gallen@ph.arlut.utexas.edu $
//-----------------------------------------------------------------------------
//	PthreadErrorHandler class
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

#ifndef PthreadErrorHandler_h
#define PthreadErrorHandler_h

#ifdef EXTERNAL_TEMPLATES
#     pragma interface
#endif

#include "PthreadDefs.h"
#ifdef _POSIX_THREADS
#include "ErrnoException.h"

class PthreadErrorHandler {
  public:
	PthreadErrorHandler(void)	{ error = 0; }
	int		Error(void)			{ return error; }
	void    Clear(void)         { error = 0; }
  protected:
	void	TrapError(int result);
	int		error;
};


//-----------------------------------------------------------------------------
inline void PthreadErrorHandler::TrapError(int err)
//	If err is nonzero, throw it as an exception.
//-----------------------------------------------------------------------------
{
	if (err) {
		error = err;
        throw ErrnoException(err);
	}
}


#endif
#endif

//=============================================================================
//	$Log: <Not implemented> $
//=============================================================================

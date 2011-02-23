//=============================================================================
//	$Id:$
//-----------------------------------------------------------------------------
//	Linear Feedback Shift Register (LFSR) class
//-----------------------------------------------------------------------------
//	Computational Process Networks class library
//	Copyright (C) 1997-2006  Gregory E. Allen and The University of Texas
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

#ifndef LFSR_h
#define LFSR_h

class LFSR {
  public:
	typedef unsigned long LFSR_t;
	LFSR(LFSR_t feed_, LFSR_t seed_) : feed(feed_), seed(seed_) { }
	
	LFSR_t Order(void) const;
	LFSR_t MaxVal(void) const;
	LFSR_t GetResult(void) { return seed = (seed & 1) ? (seed>>1 ^ feed) : (seed>>1); }

	LFSR_t Seed(void) const{ return seed; }
    LFSR_t Seed(LFSR_t s) { return seed = s; }
	LFSR_t Feed(void) const { return feed; }
    LFSR_t Feed(LFSR_t f) { return feed = f; }

  protected:
	LFSR_t feed;
	LFSR_t seed;
};

#endif

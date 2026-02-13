//=============================================================================
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
/** \file
 * \author John Bridgman
 */
#ifndef LOGICALCLOCK_H
#define LOGICALCLOCK_H
#pragma once

/** \brief A simple implementation of Lamport's logical clock.
 */
class LogicalClock {
public:
    typedef unsigned long long Clock_t;

    LogicalClock();

    /** \return the current value of the logical clock */
    Clock_t Get() const { return value; }

    /** \brief Increment the logical clock
     * \return the old value of the logical clock
     */
    Clock_t Tick() { return value++; }

    /** \brief Update the logical clock with the
     * given value obtained from another logical clock.
     * \return the old value
     */
    Clock_t Update(Clock_t v);

private:
    unsigned long long value;
};

#endif

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
#ifndef PATHUTILS_H
#define PATHUTILS_H
#pragma once
#include <string>

/**
 * Fully resolve path to its absolute path.
 * path must exist. Follows symlinks.
 */
std::string RealPath(const std::string &path);

/**
 * \return the directory name of path.
 * Trailing / are not considered part of the path.
 * If no / is contained inside path then . is returned.
 *
 * This was written to provide a reentrant version of
 * dirname. Should work the same as dirname in the
 * POSIX 1003.1-2008 specification.
 * Note that giving '..' returns '.' which is what
 * the POSIX standard says but man basename does '..'
 * gives '..' on most systems.
 */
std::string DirName(const std::string &path);

/**
 * Essentially the same as basename from libgen
 * but this one is reentrant.
 */
std::string BaseName(const std::string &path);

/**
 * Convinience function to concatinate a filename to a directory
 * in a platform dependent way.
 */
std::string PathConcat(const std::string &dir, const std::string &file);

/**
 * \returns true if path is an absolute path.
 */
bool IsAbsPath(const std::string &path);
#endif

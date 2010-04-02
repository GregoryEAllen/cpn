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
#ifndef DIRECTORY_H
#define DIRECTORY_H
#pragma once

#include <string>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

class Directory {
public:
    Directory(const std::string &dirname_);

    ~Directory();

    /**
     * Rewind to the beginning of the directory.
     */
    void Rewind();

    /**
     * \return true if we are at the end of the
     * directory.
     */
    bool End() const { return entry == 0; }

    /**
     * Get the name of the current file in the directory
     */
    std::string BaseName() const;
    std::string FullName() const { return DirName() + "/" + BaseName(); }
    std::string DirName() const { return dirname; }

    /**
     * Move to the next file in the directory.
     */
    void Next();

    bool IsDirectory() const;
    bool IsRegularFile() const;
    std::size_t Size() const;
private:
    void Stat() const;
    DIR *dir;
    std::string dirname;
    dirent *entry;
    mutable struct stat status;
    mutable bool stated;
};


#endif

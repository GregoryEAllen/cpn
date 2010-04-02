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

#include "Directory.h"
#include "ErrnoException.h"

Directory::Directory(const std::string &dirname_)
    : dir(0),dirname(dirname_), entry(0), stated(false)
{
    dir = opendir(dirname.c_str());
    if (!dir) {
        throw ErrnoException();
    }
    Next();
}

Directory::~Directory() {
    closedir(dir);
}

void Directory::Rewind() {
    rewinddir(dir);
}

void Directory::Next() {
    entry = readdir(dir);
    stated = false;
}

std::string Directory::BaseName() const {
    return std::string(entry->d_name);
}

bool Directory::IsDirectory() const {
    Stat();
    return S_ISDIR(status.st_mode);
}

bool Directory::IsRegularFile() const {
    Stat();
    return S_ISREG(status.st_mode);
}

std::size_t Directory::Size() const {
    Stat();
    return status.st_size;
}

void Directory::Stat() const {
    if (!stated) {
        std::string fpath = dirname + "/" + entry->d_name;
        if (stat(fpath.c_str(), &status) != 0) {
            throw ErrnoException();
        }
        stated = true;
    }
}


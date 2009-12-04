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
#ifndef SPREADCLIENT_H
#define SPREADCLIENT_H
#pragma once

#include "SpreadMessage.h"
#include <string>

class SpreadClient {
public:
    SpreadClient(
            const std::string &spread_name,
            const std::string &name
            );

    ~SpreadClient();

    const std::string &GetPrivateGroup() const { return private_group; }

    int Mailbox() const { return mbox; }

    int Join(const std::string &group_name);

    int Leave(const std::string &group_name);

    int Send(const SpreadMessage &msg);

    int Recv(SpreadMessage &msg);

    void Close();
private:
    int mbox;
    std::string private_group;
};
#endif

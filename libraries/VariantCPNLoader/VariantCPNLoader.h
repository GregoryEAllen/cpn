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
#ifndef VARIANTCPNLOADER_H
#define VARIANTCPNLOADER_H
#pragma once
#include "Kernel.h"
#include "Variant.h"
#include <utility>

/**
 * Load a CPN network from a specially defined Variant
 */
class VariantCPNLoader {
public:
    VariantCPNLoader();
    VariantCPNLoader(Variant conf);
    ~VariantCPNLoader();

    Variant &GetConfig() { return config; }
    void MergeConfig(Variant v);

    // Database functions
    void UseD4R(bool value);
    void GrowQueueMaxThreshold(bool value);
    void SwallowBrokenQueueExceptions(bool value);
    void LogLevel(int i);
    void AddLib(const std::string &filename);
    void AddLibList(const std::string &filename);
    void DatabaseHost(const std::string &host);
    void DatabasePort(const std::string &port);

    void AddNode(Variant v);
    void AddQueue(Variant v);
    void AddNodeMapping(const std::string &noden, const std::string &kernn);

    std::pair<bool, std::string> Validate() { return Validate(config); }

    CPN::KernelAttr GetKernelAttr() { return GetKernelAttr(config); }
    void Setup(CPN::Kernel *kernel) { Setup(kernel, config); }

    static CPN::shared_ptr<CPN::Database> LoadDatabase(Variant v);
    static CPN::KernelAttr GetKernelAttr(Variant args);

    static void Setup(CPN::Kernel *kernel, Variant args);

    static void LoadNodes(CPN::Kernel *kernel, Variant nodelist, Variant nodemap);
    static void LoadNode(CPN::Kernel *kernel, Variant attr, Variant nodemap);

    static void LoadQueues(CPN::Kernel *kernel, Variant queuelist);
    static void LoadQueue(CPN::Kernel *kernel, Variant attr);

    static std::pair<bool, std::string> Validate(Variant conf);
private:
    Variant config;
};
#endif

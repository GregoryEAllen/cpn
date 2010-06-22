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
#include "VariantCPNLoader.h"
#include "VariantToJSON.h"

VariantCPNLoader::VariantCPNLoader() { } 

CPN::KernelAttr VariantCPNLoader::GetKernelAttr(Variant args) {
    CPN::KernelAttr attr(args["name"].AsString());
    if (args["host"].IsString()) {
        attr.SetHostName(args["host"].AsString());
    }
    if (args["port"].IsString()) {
        attr.SetServName(args["port"].AsString());
    }
    return attr;
}

void VariantCPNLoader::Setup(CPN::Kernel *kernel, Variant args) {
    LoadNodes(kernel, args["nodes"], args["nodemap"]);
    LoadQueues(kernel, args["queues"]);
}

void VariantCPNLoader::LoadNodes(CPN::Kernel *kernel, Variant nodelist, Variant nodemap) {
    if (!nodelist.IsArray()) {
        return;
    }
    Variant::ConstListIterator itr, end;
    itr = nodelist.ListBegin();
    end = nodelist.ListEnd();
    while (itr != end) {
        if (itr->IsObject()) {
            LoadNode(kernel, *itr, nodemap);
        }
        ++itr;
    }
}

void VariantCPNLoader::LoadNode(CPN::Kernel *kernel, Variant attr, Variant nodemap) {
    CPN::NodeAttr nattr(attr["name"].AsString(), attr["type"].AsString());
    if (nodemap.IsObject() && nodemap.At(nattr.GetName()).IsString()) {
        nattr.SetHost(nodemap.At(nattr.GetName()).AsString());
    } else if (attr["host"].IsString()) {
        nattr.SetHost(attr["host"].AsString());
    }
    Variant param = attr["param"];
    if (!param.IsNull()) {
        if (param.IsString()) {
            nattr.SetParam(param.AsString());
        } else {
            nattr.SetParam(VariantToJSON(param));
        }
    }
    kernel->CreateNode(nattr);
}

void VariantCPNLoader::LoadQueues(CPN::Kernel *kernel, Variant queuelist) {
    if (!queuelist.IsArray()) {
        return;
    }
    Variant::ConstListIterator itr, end;
    itr = queuelist.ListBegin();
    end = queuelist.ListEnd();
    while (itr != end) {
        if (itr->IsObject()) {
            LoadQueue(kernel, *itr);
        }
        ++itr;
    }
}

void VariantCPNLoader::LoadQueue(CPN::Kernel *kernel, Variant attr) {
    CPN::QueueAttr qattr(attr["size"].AsUnsigned(), attr["threshold"].AsUnsigned());
    qattr.SetReader(attr["readernode"].AsString(), attr["readerport"].AsString());
    qattr.SetWriter(attr["writernode"].AsString(), attr["writerport"].AsString());
    if (!attr["type"].IsNull()) {
        if (attr["type"].IsNumber()) {
            qattr.SetHint(attr["type"].AsNumber<CPN::QueueHint_t>());
        } else if (attr["type"].AsString() == "threshold") {
            qattr.SetHint(CPN::QUEUEHINT_THRESHOLD);
        } else {
            qattr.SetHint(CPN::QUEUEHINT_DEFAULT);
        }
    }
    if (!attr["datatype"].IsNull()) {
        qattr.SetDatatype(attr["datatype"].AsString());
    }
    if (!attr["numchannels"].IsNull()) {
        qattr.SetNumChannels(attr["numchannels"].AsUnsigned());
    }
    if (!attr["alpha"].IsNull()) {
        qattr.SetAlpha(attr["alpha"].AsDouble());
    }
    kernel->CreateQueue(qattr);
}


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
#include "RemoteContext.h"
#include <set>
#include <map>

using CPN::shared_ptr;
using CPN::Context;
using std::string;
using std::set;
using std::pair;
using std::make_pair;
using std::map;

VariantCPNLoader::VariantCPNLoader() { } 
VariantCPNLoader::VariantCPNLoader(Variant conf) : config(conf) { } 
VariantCPNLoader::~VariantCPNLoader() { } 

void VariantCPNLoader::UseD4R(bool value) {
    config["d4r"] = value;
}

void VariantCPNLoader::GrowQueueMaxThreshold(bool value) {
    config["grow-queue-max-threshold"] = value;
}

void VariantCPNLoader::SwallowBrokenQueueExceptions(bool value) {
    config["swallow-broken-queue-exceptions"] = value;
}

void VariantCPNLoader::AddLib(const std::string &filename) {
    config["libs"].Append(filename);
}

void VariantCPNLoader::AddLibList(const std::string &filename) {
    config["liblist"].Append(filename);
}

void VariantCPNLoader::ContextHost(const std::string &host) {
    config["context"]["host"] = host;
}

void VariantCPNLoader::ContextPort(const std::string &port) {
    config["context"]["port"] = port;
}

void VariantCPNLoader::LogLevel(int i) {
    config["context"]["loglevel"] = i;
}

void VariantCPNLoader::MergeConfig(Variant v) {
    Variant::MapIterator topitr = v.MapBegin(), topend = v.MapEnd();
    for (;topitr != topend; ++topitr) {
        if (topitr->first == "context") {
            Variant::MapIterator mitr = v["context"].MapBegin(), mend = v["context"].MapEnd();
            for (;mitr != mend; ++mitr) {
                config["context"][mitr->first] = mitr->second;
            }
        } else if (topitr->first == "nodes") {
            Variant::ListIterator litr = v["nodes"].ListBegin(), lend = v["nodes"].ListEnd();
            for (;litr != lend; ++litr) {
                AddNode(*litr);
            }
        } else if (topitr->first == "queues") {
            Variant::ListIterator litr = v["queues"].ListBegin(), lend = v["queues"].ListEnd();
            for (;litr != lend; ++litr) {
                AddQueue(*litr);
            }
        } else {
            config[topitr->first] = topitr->second;
        }
    }
}


void VariantCPNLoader::AddNode(Variant v) {
    if (!config["nodes"].IsArray()) {
        config["nodes"] = Variant::ArrayType;
    }
    Variant nodes = config["nodes"];
    Variant::ListIterator itr = nodes.ListBegin(), end = nodes.ListEnd();
    for (; itr != end; ++itr) {
        if (v["name"] == itr->At("name")) {
            for (Variant::MapIterator mitr = v.MapBegin(),
                    mend = v.MapEnd(); mitr != mend; ++mitr) {
                itr->At(mitr->first) = mitr->second.Copy();
            }
            return;
        }
    }
    nodes.Append(v.Copy());
}

void VariantCPNLoader::AddQueue(Variant v) {
    if (!config["queues"].IsArray()) {
        config["queues"] = Variant::ArrayType;
    }
    Variant queues = config["queues"];
    Variant::ListIterator qitr = queues.ListBegin(), qend = queues.ListEnd();
    for (;qitr != qend; ++qitr) {
        if (
                (
                 v["readernode"] == qitr->At("readernode") &&
                 v["readerport"] == qitr->At("readerport")
                ) ||
                (
                 v["writernode"] == qitr->At("writernode") &&
                 v["writerport"] == qitr->At("writerport")
                )
           ) {
            for (Variant::MapIterator mitr = v.MapBegin(),
                    mend = v.MapEnd(); mitr != mend; ++mitr) {
                qitr->At(mitr->first) = mitr->second.Copy();
            }
            return;
        }
    }
    queues.Append(v.Copy());
}

void VariantCPNLoader::AddNodeMapping(const std::string &noden, const std::string &kernn) {
    if (!config["nodemap"].IsObject()) {
        config["nodemap"] = Variant::ObjectType;
    }
    Variant nodemap = config["nodemap"];
    nodemap[noden] = kernn;
}

shared_ptr<Context> VariantCPNLoader::LoadContext(Variant v) {
    shared_ptr<Context> context;
    if (v.IsNull()) {
        context = Context::Local();
    } else {
        if (v["host"].IsNull() && v["port"].IsNull()) {
            context = Context::Local();
        } else {
            SockAddrList addrs = SocketAddress::CreateIP(
                    v["host"].AsString(),
                    v["port"].AsString()
                    );
            shared_ptr<RemoteContext> ctx = shared_ptr<RemoteContext>(new RemoteContext(addrs));
            context = ctx;
        }

        if (!v["loglevel"].IsNull()) {
            context->LogLevel(v["loglevel"].AsInt());
        }
    }
    return context;
}

void VariantCPNLoader::KernelName(const std::string &name) {
    config["name"] = name;
}

void VariantCPNLoader::KernelHost(const std::string &host) {
    config["host"] = host;
}

void VariantCPNLoader::KernelPort(const std::string &port) {
    config["port"] = port;
}

CPN::KernelAttr VariantCPNLoader::GetKernelAttr(Variant args) {
    CPN::KernelAttr attr(args["name"].AsString());
    if (!args["context"].IsNull()) {
        attr.SetContext(LoadContext(args["context"]));
    }
    if (!args["host"].IsNull()) {
        attr.SetHostName(args["host"].AsString());
    }
    if (!args["port"].IsNull()) {
        attr.SetServName(args["port"].AsString());
    }

    if (!args["d4r"].IsNull()) {
        attr.UseD4R(args["d4r"].AsBool());
    }
    if (!args["swallow-broken-queue-exceptions"].IsNull()) {
        attr.SwallowBrokenQueueExceptions(args["swallow-broken-queue-exceptions"].AsBool());
    }
    if (!args["grow-queue-max-threshold"].IsNull()) {
        attr.GrowQueueMaxThreshold(args["grow-queue-max-threshold"].AsBool());
    }
    if (args["libs"].IsArray()) {
        for (Variant::ListIterator itr = args["libs"].ListBegin(); itr != args["libs"].ListEnd(); ++itr) {
            attr.AddSharedLib(itr->AsString());
        }
    }
    if (args["liblist"].IsArray()) {
        for (Variant::ListIterator itr = args["liblist"].ListBegin(); itr != args["liblist"].ListEnd(); ++itr) {
            attr.AddNodeList(itr->AsString());
        }
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
    if (param.IsObject()) {
        for (Variant::MapIterator i = param.MapBegin(), e = param.MapEnd();
                i != e; ++i)
        {
            nattr.SetParam(i->first, i->second.AsString());
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
    if (attr["name"].IsString()) {
        qattr.SetName(attr["name"].AsString());
    }
    kernel->CreateQueue(qattr);
}

std::pair<bool, std::string> VariantCPNLoader::Validate(Variant conf) {
    set<string> nodeset;
    if (!conf["name"].IsString()) {
        return make_pair(false, "Kernels must have a name.");
    }

    Variant nodes = conf["nodes"];
    Variant::ListIterator nitr = nodes.ListBegin(), nend = nodes.ListEnd();
    for (; nitr != nend; ++nitr) {
        if (!nitr->At("name").IsString()) {
            return make_pair(false, "Nodes must have a name");
        }
        if (!nitr->At("type").IsString()) {
            return make_pair(false, "Nodes must have a type");
        }
        if (!nodeset.insert(nitr->At("name").AsString()).second) {
            return make_pair(false, "Duplicate node");
        }
    }
    map<string, set<string> > readports, writeports;
    Variant queues = conf["queues"];
    Variant::ListIterator qitr = queues.ListBegin(), qend = queues.ListEnd();
    for (;qitr != qend; ++qitr) {
        if (qitr->At("size").IsNull()) {
            return make_pair(false, "Queues must have a size");
        }
        if (qitr->At("threshold").IsNull()) {
            return make_pair(false, "Queues must have a threshold");
        }
        if (!qitr->At("readernode").IsString()) {
            return make_pair(false, "Queues must have a readernode");
        }
        if (!qitr->At("readerport").IsString()) {
            return make_pair(false, "Queues must have a readerport");
        }
        if (!readports[qitr->At("readernode").AsString()].insert(qitr->At("readerport").AsString()).second) {
            return make_pair(false, "Two queues connected to the same reader port.");
        }
        if (!qitr->At("writernode").IsString()) {
            return make_pair(false, "Queues must have a writernode");
        }
        if (!qitr->At("writerport").IsString()) {
            return make_pair(false, "Queues must have a writerport");
        }
        if (!writeports[qitr->At("writernode").AsString()].insert(qitr->At("writerport").AsString()).second) {
            return make_pair(false, "Two queues connected to the same writer port.");
        }
    }
    return make_pair(true, "");
}


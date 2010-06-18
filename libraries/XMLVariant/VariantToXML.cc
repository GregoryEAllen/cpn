
#include "VariantToXML.h"
#include <sstream>
#include <libxml++/libxml++.h>


static void BuildDOM(xmlpp::Element *cur, Variant v, const std::string &name) {
    xmlpp::Element *child;
    switch (v.GetType()) {
    case Variant::TrueType:
    case Variant::FalseType:
    case Variant::NumberType:
    case Variant::StringType:
        child = cur->add_child(name);
        child->add_child_text(v.AsString());
        break;
    case Variant::ArrayType:
        for (Variant::ConstListIterator i = v.ListBegin(); i != v.ListEnd(); ++i) {
            if (i->IsArray()) {
                child = cur->add_child(name);
                BuildDOM(child, *i, name);
            } else {
                BuildDOM(cur, *i, name);
            }
        }
        break;
    case Variant::ObjectType:
        child = cur->add_child(name);
        for (Variant::ConstMapIterator i = v.MapBegin(); i != v.MapEnd(); ++i) {
            BuildDOM(child, i->second, i->first);
        }
        break;
    case Variant::NullType:
    default:
        break;
    }
}

std::string VariantToXML(Variant v, const std::string &rname, bool pretty) {
    std::ostringstream oss;
    VariantToXML(oss, v, rname, pretty);
    return oss.str();
}

std::ostream &VariantToXML(std::ostream &os, const Variant &v, const std::string &rname, bool pretty) {
    xmlpp::Document doc;
    xmlpp::Element *root = doc.create_root_node(rname);
    switch (v.GetType()) {
    case Variant::TrueType:
    case Variant::FalseType:
    case Variant::NumberType:
    case Variant::StringType:
        BuildDOM(root, v, rname);
        break;
    case Variant::ArrayType:
        for (Variant::ConstListIterator i = v.ListBegin(); i != v.ListEnd(); ++i) {
            BuildDOM(root, *i, rname);
        }
        break;
    case Variant::ObjectType:
        for (Variant::ConstMapIterator i = v.MapBegin(); i != v.MapEnd(); ++i) {
            BuildDOM(root, i->second, i->first);
        }
        break;
    case Variant::NullType:
    default:
        break;
    }

    if (pretty) {
        doc.write_to_stream_formatted(os, "UTF-8");
    } else {
        doc.write_to_stream(os, "UTF-8");
    }
    return os;
}


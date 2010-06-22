
#include "VariantToXML.h"
#include "Assert.h"
#include <sstream>
#include <libxml++/libxml++.h>

static std::string GetTypeName(Variant::Type_t t) {
    switch (t) {
    case Variant::TrueType:
        return "bool";
    case Variant::FalseType:
        return "bool";
    case Variant::NumberType:
        return "number";
    case Variant::StringType:
        return "string";
    case Variant::ArrayType:
        return "array";
    case Variant::ObjectType:
        return "object";
    case Variant::NullType:
        return "null";
    }
    ASSERT(false);
}

static void BuildDOM(xmlpp::Element *cur, Variant v, const std::string &name) {
    xmlpp::Element *child;
    switch (v.GetType()) {
    case Variant::TrueType:
    case Variant::FalseType:
    case Variant::NumberType:
    case Variant::StringType:
        child = cur->add_child(name);
        child->set_attribute("type", GetTypeName(v.GetType()));
        child->add_child_text(v.AsString());
        break;
    case Variant::ArrayType:
        for (Variant::ConstListIterator i = v.ListBegin(); i != v.ListEnd(); ++i) {
            if (i->IsArray()) {
                child = cur->add_child(name);
                child->set_attribute("type", GetTypeName(i->GetType()));
                BuildDOM(child, *i, name);
            } else {
                BuildDOM(cur, *i, name);
            }
        }
        break;
    case Variant::ObjectType:
        child = cur->add_child(name);
        child->set_attribute("type", GetTypeName(v.GetType()));
        for (Variant::ConstMapIterator i = v.MapBegin(); i != v.MapEnd(); ++i) {
            BuildDOM(child, i->second, i->first);
        }
        break;
    case Variant::NullType:
        child = cur->add_child(name);
        child->set_attribute("type", GetTypeName(v.GetType()));
        break;
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


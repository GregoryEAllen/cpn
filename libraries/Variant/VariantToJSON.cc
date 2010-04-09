
#include "VariantToJSON.h"
#include <sstream>


std::string VariantToJSON(const Variant &v, bool pretty) {
    std::ostringstream oss;
    VariantToJSON(oss, v, pretty);
    return oss.str();
}

void InsertString(std::ostream &os, const std::string str) {
    os << "\"";
	char buffer[6];
    const char *text = str.c_str();
    while (*text != '\0') {
		if (*text == '\\') {
            os << "\\\\";
		} else if (*text == '\"') {
            os << "\\\"";
		} else if (*text == '/') {
            os << "\\/";
		} else if (*text == '\b') {
            os << "\\b";
		} else if (*text == '\f') {
            os << "\\f";
		} else if (*text == '\n') {
            os << "\\n";
		} else if (*text == '\r') {
            os << "\\r";
		} else if (*text == '\t') {
            os << "\\t";
		} else if (*text < 0)	{
            os << *text;
		} else if (*text < 0x20) {
			sprintf(buffer, "\\u%4.4x", *text);
            os << buffer;
		} else {
            os << *text;
		}
        ++text;
	}
    os << "\"";
}

void InsertIndent(std::ostream &os, unsigned level) {
    os << "\n";
    for (unsigned j = 0; j < level; ++j) {
        os << "\t";
    }
}

std::ostream &VariantToJSON(std::ostream &os, const Variant &v, bool pretty, unsigned level) {
    switch (v.GetType()) {
    case Variant::NullType:
    case Variant::TrueType:
    case Variant::FalseType:
    case Variant::NumberType:
        os << v.AsString();
        break;
    case Variant::StringType:
        InsertString(os, v.AsString());
        break;
    case Variant::ArrayType:
        os << "[";
        for (Variant::ConstListIterator i = v.ListBegin(); i != v.ListEnd(); ++i) {
            if (i != v.ListBegin()) { os << ","; }
            if (pretty) { InsertIndent(os, level); }
            VariantToJSON(os, *i, pretty, level + 1);
        }
        if (pretty) { InsertIndent(os, level - 1); }
        os << "]";
        break;
    case Variant::ObjectType:
        os << "{";
        for (Variant::ConstMapIterator i = v.MapBegin(); i != v.MapEnd(); ++i) {
            if (i != v.MapBegin()) { os << ","; }
            if (pretty) { InsertIndent(os, level); }
            InsertString(os, i->first);
            if (pretty) { os << " : "; }
            else { os << ":"; }
            VariantToJSON(os, i->second, pretty, level + 1);
        }
        if (pretty) { InsertIndent(os, level - 1); }
        os << "}";
        break;
    default:
        break;
    }
    return os;
}

std::ostream &VariantToJSON(std::ostream &os, const Variant &v, bool pretty) {
    return VariantToJSON(os, v, pretty, 1);
}


#include "Variant.h"
#include "VariantToJSON.h"
#include "JSONToVariant.h"
#include "XMLToVariant.h"
#include "VariantToXML.h"
#include <iostream>
#include <sstream>
#include <string>
#include <fstream>
#include <unistd.h>

using std::string;
using std::cerr;
using std::cout;
using std::cin;
using std::istringstream;
using std::ifstream;
using std::ofstream;
using std::ostream;
using std::istream;
using std::endl;

int ConvertXMLToJSON(istream &is, ostream &os, bool pretty) {
    XMLToVariant xparser;
    xparser.ParseStream(is);
    if (!xparser.Done()) {
        cerr << "Error parsing XML\n" << xparser.GetMessage() << endl;
        return 1;
    }
    Variant val = xparser.Get();
    VariantToJSON(os, val, pretty);
    if (pretty) os << '\n';
    return 0;
}

int ConvertJSONToXML(istream &is, ostream &os, bool pretty, const string &rootname) {
    JSONToVariant jparser;
    is >> jparser;
    if (!jparser.Done()) {
        cerr << "Error parsing JSON on line " << jparser.GetLine()
            << " column " << jparser.GetColumn() << endl;
        return 1;
    }
    Variant val = jparser.Get();
    VariantToXML(os, val, rootname, pretty);
    if (pretty) os << '\n';
    return 0;
}

void PrintHelp(const string &progname) {
    cerr << "Usage: " << progname << " [options]\n";
    cerr << "\t-p\tUse pretty print\n";
    cerr << "\t-i file\tUse file as input (default: stdin)\n";
    cerr << "\t-o file\tUse file as output (default: stdout)\n";
    cerr << "\t-c chars\tUse chars as input\n";
    cerr << "\t-r name\tUse name as the root name of the XML document output (default: root)\n";
    cerr << "\t-x\tConvert XML to JSON\t";
    cerr << "\t-j\tConvert JSON to XML (default)\n";
    cerr << "\nNotice that the conversion is not 1-to-1 for all inputs.\n";
}

int main(int argc, char **argv) {
    string in_file;
    string out_file;
    string in_chars;
    bool pretty = false;
    string rootname = "root";
    enum {
        XML,
        JSON
    } in_type = JSON;
    while (true) {
        int c = getopt(argc, argv, "phi:o:c:r:xj");
        if (c == -1) break;
        switch (c) {
        case 'p':
            pretty = true;
            break;
        case 'i':
            in_file = optarg;
            break;
        case 'o':
            out_file = optarg;
            break;
        case 'c':
            in_chars = optarg;
            break;
        case 'r':
            rootname = optarg;
            break;
        case 'x':
            in_type = XML;
            break;
        case 'j':
            in_type = JSON;
            break;
        case 'h':
            PrintHelp(*argv);
            return 0;
        default:
            return 1;
        }
    }
    ifstream finput;
    ofstream foutput;
    istream *input = &cin;
    ostream *output = &cout;
    istringstream iss(in_chars);
    if (!in_chars.empty()) {
        input = &iss;
    } else if (!in_file.empty()) {
        finput.open(in_file.c_str());
        if (!finput) {
            cerr << "Error opening " << in_file << endl;
            return 1;
        }
        input = &finput;
    }
    if (!out_file.empty()) {
        foutput.open(out_file.c_str());
        if (!foutput) {
            cerr << "Error opening " << out_file << endl;
            return 1;
        }
        output = &foutput;
    }
    if (in_type == XML) {
        return ConvertXMLToJSON(*input, *output, pretty);
    }
    if (in_type == JSON) {
        return ConvertJSONToXML(*input, *output, pretty, rootname);
    }
    return 1;
}

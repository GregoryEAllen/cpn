
#include "Variant.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>

Variant ReadValues(FILE *f) {
    std::vector<char> buffer(1024);
    JSONToVariant parse;
    while (!feof(f) && parse.Ok()) {
        int ret = fread(&buffer[0], 1, buffer.size(), f);
        if (ret > 0) {
            parse.Parse(&buffer[0], ret);
        } else {
            break;
        }
    }
    return parse.Get();
}

std::string BuildLegendString(Variant run) {
    std::ostringstream oss;
    oss << "wheel: " << run["primewheel"].AsString() << " ";
    oss << "ppf: " << VariantToJSON(run["ppf"]) << " ";
    oss << "zc: " << run["zerocopy"].AsString() << " ";
    return oss.str();
}

void Shuffle(std::string infile, std::string datafile, std::string legendfile) {
    FILE *in = fopen(infile.c_str(), "r");
    if (!in) {
        fprintf(stderr, "Couldn't open %s\n", infile.c_str());
        exit(1);
    }
    FILE *data = fopen(datafile.c_str(), "w");
    if (!data) {
        fprintf(stderr, "Couldn't open %s\n", datafile.c_str());
        exit(1);
    }
    FILE *legend = fopen(legendfile.c_str(), "w");
    if (!legend) {
        fprintf(stderr, "Couldn't open %s\n", legendfile.c_str());
        exit(1);
    }

    Variant info = ReadValues(in);
    fclose(in);
    in = 0;

    typedef std::map<uint64_t, double> vmap_t;
    typedef vmap_t::iterator vitr_t;
    typedef std::map<std::string, vmap_t> dmap_t;
    typedef dmap_t::iterator ditr_t;
    std::vector<uint64_t> xdata;
    ditr_t ditr;
    vitr_t vitr;

    dmap_t datamap;

    Variant::ListIterator litr = info.ListBegin();
    while (litr != info.ListEnd()) {
        std::string legendstring = BuildLegendString(*litr);
        ditr = datamap.find(legendstring);
        if (ditr == datamap.end()) {
            ditr = datamap.insert(std::make_pair(legendstring, vmap_t())).first;
        }
        uint64_t xaxis = litr->At("maxprime").AsNumber<uint64_t>();
        double yaxis = litr->At("realtime").AsDouble();
        if (std::find(xdata.begin(), xdata.end(), xaxis) == xdata.end()) {
            xdata.push_back(xaxis);
        }
        vitr = ditr->second.find(xaxis);
        if (vitr == ditr->second.end()) {
            ditr->second.insert(std::make_pair(xaxis, yaxis));
        } else if (vitr->second > yaxis) {
            vitr->second = yaxis;
        }
        ++litr;
    }
    std::sort(xdata.begin(), xdata.end());
    fputs("x-axis\n", legend);
    ditr = datamap.begin();
    while (ditr != datamap.end()) {
        fprintf(legend, "%s\n", ditr->first.c_str());
        ++ditr;
    }
    fclose(legend);
    legend = 0;

    std::vector<uint64_t>::iterator xditr = xdata.begin();
    while (xditr != xdata.end()) {
        fprintf(data, "%llu", *xditr);
        ditr = datamap.begin();
        while (ditr != datamap.end()) {
            vitr = ditr->second.find(*xditr);
            if (vitr != ditr->second.end()) {
                fprintf(data, "\t%f", vitr->second);
            } else {
                fprintf(data, "\tNaN");
            }
            ++ditr;
        }
        fprintf(data, "\n");
        ++xditr;
    }
    fclose(data);
    data = 0;

}

const char* const VALID_OPTS = "sc";

int main(int argc, char **argv) {
    bool shuffle = false;
    bool concat = false;
    bool procOpts = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 's':
            shuffle = true;
            break;
        case 'c':
            concat = true;
            break;
        case -1:
            procOpts = false;
            break;
        default:
            printf("Invalid option\n");
            return 1;
        }
    }
    if (shuffle) {
        if (argc <= optind + 2) {
            fprintf(stderr, "Not enough parameters\n");
            exit(1);
        }
        Shuffle(argv[optind], argv[optind + 1], argv[optind + 2]);
    } else if (concat) {
        Variant output = Variant::ArrayType;
        for (int i = optind; i < argc; ++i) {
            FILE *f = fopen(argv[i], "r");
            if (!f) {
                fprintf(stderr, "Couldn't open %s\n", argv[i]);
                exit(1);
            } else {
                fprintf(stderr, "Processing: %s\n", argv[i]);
            }
            Variant val = ReadValues(f);
            fclose(f);
            for (Variant::ListIterator itr = val.ListBegin(); itr != val.ListEnd(); ++itr) {
                output.Append(*itr);
            }
        }
        puts(VariantToJSON(output).c_str());
    }
    return 0;
}


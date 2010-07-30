#include "Database.h"
#include "FlowMeasure.h"
#include "JSONToVariant.h"
#include "Kernel.h"
#include "LoadFromFile.h"
#include "NodeBase.h"
#include "ParseBool.h"
#include "PathUtils.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "RemoteDatabase.h"
#include "VariantCPNLoader.h"
#include "Variant.h"
#include "VariantToJSON.h"
#include "VBeamformer.h"
#include <algorithm>
#include <complex>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <vector>

using std::complex;
using CPN::shared_ptr;
using CPN::Database;

class CPNBFOutputNode : public CPN::NodeBase {
public:
    CPNBFOutputNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr)
    {
        JSONToVariant parser;
        parser.Parse(attr.GetParam().data(), attr.GetParam().size());
        ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
        Variant param = parser.Get();
        inports.resize(param["inports"].Size());
        std::transform(param["inports"].ListBegin(), param["inports"].ListEnd(),
                inports.begin(), std::mem_fun_ref(&Variant::AsString));
        blocksize = param["blocksize"].AsUnsigned();
        outfile = param["outfile"].AsString();
        nooutput = param["nooutput"].AsBool();
    }
    void Process() {
        std::vector< CPN::QueueReaderAdapter< complex<float> > > in(inports.size());
        std::vector< CPN::QueueReaderAdapter< complex<float> > >::iterator cur, begin, end;
        cur = begin = in.begin();
        end = in.end();
        for (std::vector<std::string>::iterator itr = inports.begin(); cur != end; ++cur, ++itr) {
            *cur = GetReader(*itr);
        }
        FILE *f = 0;
        if (!nooutput) {
            f = fopen(outfile.c_str(), "w");
            ASSERT(f, "Could not open %s", outfile.c_str());
        }
        cur = begin;
        FlowMeasure measure;
        measure.Start();
        while (true) {
            if (cur == end) {
                cur = begin;
                measure.Tick(blocksize);
            }
            unsigned amount = blocksize;
            const complex<float> *ptr = cur->GetDequeuePtr(amount);
            if (!ptr) {
                amount = cur->Count();
                if (amount > 0) {
                    ptr = cur->GetDequeuePtr(amount);
                } else {
                    break;
                }
            }
            if (!nooutput) {
                DataToFile(f, ptr, amount, cur->ChannelStride(), cur->NumChannels());
            }
            cur->Dequeue(amount);
            ++cur;
        }
        fprintf(stderr,
                "Output:\nAvg:\t%f Hz\nMax:\t%f Hz\nMin:\t%f Hz\n",
                measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());
        if (f) {
            fclose(f);
        }
    }
    std::string outfile;
    std::vector<std::string> inports;
    unsigned blocksize;
    bool nooutput;
};
CPN_DECLARE_NODE_FACTORY(CPNBFOutputNode, CPNBFOutputNode);

class CPNBFInputNode : public CPN::NodeBase {
public:
    CPNBFInputNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr)
    {
        JSONToVariant parser;
        parser.Parse(attr.GetParam().data(), attr.GetParam().size());
        ASSERT(parser.Done(), "Error parsing param line %u column %u", parser.GetLine(), parser.GetColumn());
        Variant param = parser.Get();
        outport = param["outport"].AsString();
        infile = param["infile"].AsString();
        repetitions = 1;
        if (param["repetitions"].IsNumber()) {
            repetitions = param["repetitions"].AsUnsigned();
        }
    }

    void Process() {
        std::ifstream in_file;
        std::istream *in = &std::cin;
        if (!infile.empty()) {
            in_file.open(infile.c_str());
            in = &in_file;
        }
        JSONToVariant parser;
        *in >> parser;
        if (!in->good() || !parser.Done()) {
            // either an error or we are at the end of file
            // Just ignore errors for now...
            return;
        }
        Variant header = parser.Get();
        while (in->get() != 0 && in->good());
        const unsigned length = header["length"].AsUnsigned();
        const unsigned numChans = header["numChans"].AsUnsigned();
        std::vector<char> data(length * numChans);
        for (unsigned i = 0; i < numChans; ++i) {
            unsigned numread = 0;
            while (in->good() && numread < length) {
                in->read(&data[i * length] + numread, length - numread);
                numread += in->gcount();
            }
        }
        CPN::QueueWriterAdapter<void> out = GetWriter(outport);
        FlowMeasure measure;
        unsigned qsize = out.QueueLength();
        unsigned rep = 0;
        unsigned written = 0;
        measure.Start();
        while (rep < repetitions && written < qsize) {
            unsigned len = std::min(qsize - written, length);
            out.Enqueue(&data[0], len, numChans, length);
            measure.Tick(len / sizeof(complex<short>));
            written += len;
            ++rep;
        }
        for (; rep < repetitions; ++rep) {
            out.GetEnqueuePtr(length);
            out.Enqueue(length);
            measure.Tick(length / sizeof(complex<short>));
        }
        fprintf(stderr,
                "Input:\nAvg:\t%f hz\nMax:\t%f hz\nMin:\t%f hz\n",
                measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());
    }

    std::string outport;
    std::string infile;
    unsigned repetitions;
};
CPN_DECLARE_NODE_FACTORY(CPNBFInputNode, CPNBFInputNode);

static const char* const VALID_OPTS = "hi:o:er:na:s:cf:H:q:";

static const char* const HELP_OPTS = "Usage: %s <vertical coefficient file> <horizontal coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-r num\t Run num times\n"
"\t-a n\t Use algorithm n for vertical\n"
"\t-n \t No output, just time\n"
"\t-s n\t Scale queue sizes by n\n"
"\t-c\t Print config and exit\n"
"\t-h\t Print this message and exit.\n"
"\t-H yes|no\t Split the horizontal beamformer (default: yes).\n"
"\t-f yes|no\t Use the 'fan' vertical beamformer (default: yes).\n"
"\t-q xxx\t Set xxx as the queue type (default: threshold).\n"
;


int cpnbf_main(int argc, char **argv) {
    bool procOpts = true;
    std::string input_file;
    std::string output_file;
    std::string vertical_config;
    std::string horizontal_config;
    std::string queue_type = "threshold";
    unsigned algo = 2;
    bool use_fan = true;
    bool estimate = false;
    unsigned repetitions = 1;
    bool nooutput = false;
    unsigned size_mult = 2;
    bool print_config = false;
    bool split_horizontal = true;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'c':
            print_config = true;
            break;
        case 'f':
            use_fan = ParseBool(optarg);
            break;
        case 'i':
            input_file = optarg;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'e':
            estimate = true;
            break;
        case 'a':
            algo = atoi(optarg);
            break;
        case 's':
            size_mult = atoi(optarg);
            break;
        case 'r':
            repetitions = atoi(optarg);
            break;
        case 'n':
            nooutput = true;
            break;
        case 'H':
            split_horizontal = ParseBool(optarg);
            break;
        case 'q':
            queue_type = optarg;
            break;

        case -1:
            procOpts = false;
            break;
        case 'h':
        default:
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 0;
        }
    }
    if (argc <= optind + 1) {
        fprintf(stderr, "Not enough parameters, need coefficient files\n");
        return 1;
    }
    if (input_file.empty()) {
        fprintf(stderr, "Must have an input file.\n");
        return 1;
    }
    if (output_file.empty() && !nooutput) {
        fprintf(stderr, "Either an output file or no output must be specified.\n");
        return 1;
    }
    vertical_config = argv[optind];
    horizontal_config = argv[optind + 1];

    Variant config;
    config["name"] = "kernel";
    std::string nodelist = RealPath("node.list");
    if (!nodelist.empty()) {
        config["database"]["liblist"].Append(nodelist);
    }
    VariantCPNLoader loader(config);
    Variant node;
    node["name"] = "vertical";
    if (use_fan) {
        node["type"] = "FanVBeamformerNode";
    } else {
        node["type"] = "VBeamformerNode";
    }
    node["param"]["inport"] = "input";
    node["param"]["outports"][0] = "out1";
    node["param"]["outports"][1] = "out2";
    node["param"]["outports"][2] = "out3";
    node["param"]["blocksize"] = 8192;
    node["param"]["file"] = vertical_config;
    node["param"]["algorithm"] = algo;
    loader.AddNode(node);
    node = Variant::NullType;
    node["type"] = "HBeamformerNode";
    node["param"]["inport"] = "input";
    node["param"]["outport"] = "output";
    node["param"]["estimate"] = estimate;
    node["param"]["file"] = horizontal_config;
    if (split_horizontal) {
        node["param"]["half"] = 1;
    }
    node["name"] = "hbf1";
    loader.AddNode(node);
    node["name"] = "hbf2";
    loader.AddNode(node);
    node["name"] = "hbf3";
    loader.AddNode(node);
    if (split_horizontal) {
        node["param"]["half"] = 2;
        node["name"] = "hbf1_2";
        loader.AddNode(node);
        node["name"] = "hbf2_2";
        loader.AddNode(node);
        node["name"] = "hbf3_2";
        loader.AddNode(node);
    }

    node = Variant::NullType;
    node["name"] = "input";
    node["type"] = "CPNBFInputNode";
    node["param"]["outport"] = "output";
    node["param"]["infile"] = input_file;
    node["param"]["repetitions"] = repetitions;
    loader.AddNode(node);
    node = Variant::NullType;

    node["name"] = "output";
    node["type"] = "CPNBFOutputNode";
    node["param"]["inports"][0] = "0";
    node["param"]["inports"][1] = "1";
    node["param"]["inports"][2] = "2";
    node["param"]["blocksize"] = 8192;
    node["param"]["outfile"] = output_file;
    node["param"]["nooutput"] = nooutput;
    loader.AddNode(node);
    node = Variant::NullType;

    Variant queue;
    queue["size"] = 8192*size_mult;
    queue["threshold"] = 8192;
    queue["type"] = queue_type;
    queue["datatype"] = "complex<float>";
    queue["numchannels"] = 256;
    queue["readerport"] = "input";
    queue["writerport"] = "out1";
    queue["readernode"] = "hbf1";
    queue["writernode"] = "vertical";
    loader.AddQueue(queue);
    queue["writerport"] = "out2";
    queue["readernode"] = "hbf2";
    loader.AddQueue(queue);
    queue["writerport"] = "out3";
    queue["readernode"] = "hbf3";
    loader.AddQueue(queue);
    queue["numchannels"] = 560;
    queue["readernode"] = "output";
    queue["readerport"] = "0";
    if (split_horizontal) { queue["writernode"] = "hbf1_2"; }
    else { queue["writernode"] = "hbf1"; }
    queue["writerport"] = "output";
    loader.AddQueue(queue);
    queue["readerport"] = "1";
    if (split_horizontal) { queue["writernode"] = "hbf2_2"; }
    else { queue["writernode"] = "hbf2"; }
    loader.AddQueue(queue);
    queue["readerport"] = "2";
    if (split_horizontal) { queue["writernode"] = "hbf3_2"; }
    else { queue["writernode"] = "hbf3"; }
    loader.AddQueue(queue);
    if (split_horizontal) {
        queue["size"] = 8192*256*size_mult;
        queue["threshold"] = 8192*256;
        queue["numchannels"] = 1;
        queue["readerport"] = "input";
        queue["writernode"] = "hbf1";
        queue["readernode"] = "hbf1_2";
        loader.AddQueue(queue);
        queue["writernode"] = "hbf2";
        queue["readernode"] = "hbf2_2";
        loader.AddQueue(queue);
        queue["writernode"] = "hbf3";
        queue["readernode"] = "hbf3_2";
        loader.AddQueue(queue);
    }
    queue["size"] = 8192*size_mult;
    queue["threshold"] = 8192;
    queue["numchannels"] = 256*12;
    queue["readernode"] = "vertical";
    queue["readerport"] = "input";
    queue["writernode"] = "input";
    queue["writerport"] = "output";
    queue["datatype"] = "complex<int16_t>";
    loader.AddQueue(queue);

    if (print_config) {
        printf("%s\n", VariantToJSON(config, true).c_str());
        return 0;
    }
    std::pair<bool, std::string> verifyret = loader.Validate();
    if (!verifyret.first) {
        fprintf(stderr, "Configuration error:\n%s\n", verifyret.second.c_str());
        return 1;
    }

    CPN::KernelAttr kattr = loader.GetKernelAttr();
    CPN::Kernel kernel(kattr);
    loader.Setup(&kernel);
    kernel.WaitForAllNodeEnd();
    return 0;
}


#include "Kernel.h"
#include "Variant.h"
#include "VariantCPNLoader.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"
#include "RemoteDatabase.h"
#include "NodeBase.h"
#include "QueueReaderAdapter.h"
#include "QueueWriterAdapter.h"
#include "FlowMeasure.h"
#include "VBeamformer.h"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <complex>
#include <algorithm>
#include <unistd.h>
#include <stdio.h>

using std::complex;

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
    }
    void Process() {
        std::vector< CPN::QueueReaderAdapter< complex<float> > > in(inports.size());
        std::vector< CPN::QueueReaderAdapter< complex<float> > >::iterator cur, begin, end;
        cur = begin = in.begin();
        end = in.end();
        for (std::vector<std::string>::iterator itr = inports.begin(); cur != end; ++cur, ++itr) {
            *cur = GetReader(*itr);
        }
        cur = begin;
        FlowMeasure measure;
        measure.Start();
        while (true) {
            if (cur == end) {
                cur = begin;
                measure.Tick(blocksize);
            }
            const complex<float> *ptr = cur->GetDequeuePtr(blocksize);
            if (!ptr) {
                break;
            }
            cur->Dequeue(blocksize);
            ++cur;
        }
        fprintf(stderr,
                "Output:\nAvg:\t%f Hz\nMax:\t%f Hz\nMin:\t%f Hz\n",
                measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());
    }
    std::vector<std::string> inports;
    unsigned blocksize;
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

static const char* const VALID_OPTS = "hi:o:er:na:s:c";

static const char* const HELP_OPTS = "Usage: %s <vertical coefficient file> <horizontal coefficient file>\n"
"\t-i filename\t Use input file (default stdin)\n"
"\t-o filename\t Use output file (default stdout)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-r num\t Run num times\n"
"\t-a n\t Use algorithm n for vertical\n"
"\t-n \t No output, just time\n"
"\t-s n\t Scale queue sizes by n\n"
"\t-c\t Print config\n"
;


int cpnbf_main(int argc, char **argv) {
    bool procOpts = true;
    std::string input_file;
    std::string output_file;
    VBeamformer::Algorithm_t algo = VBeamformer::SSE_VECTOR;
    bool estimate = false;
    unsigned repetitions = 1;
    bool nooutput = false;
    unsigned size_mult = 2;
    bool print_config = false;
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'c':
            print_config = true;
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
            algo = (VBeamformer::Algorithm_t)atoi(optarg);
            if (algo < VBeamformer::ALGO_BEGIN || algo >= VBeamformer::ALGO_END) {
                fprintf(stderr, "Unknown algorithm number %d\n", algo);
                return 1;
            }
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

        case -1:
            procOpts = false;
            break;
        default:
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 0;
        }
    }
    if (argc <= optind + 1) {
        fprintf(stderr, "Not enough parameters, need coefficient files\n");
        return 1;
    }
    Variant config;
    config["name"] = "kernel";
    Variant node;
    node["name"] = "vertical";
    node["type"] = "VBeamformerNode";
    node["param"]["inport"] = "input";
    node["param"]["outports"][0] = "out1";
    node["param"]["outports"][1] = "out2";
    node["param"]["outports"][2] = "out3";
    node["param"]["blocksize"] = 8192;
    node["param"]["file"] = argv[optind];
    node["param"]["algorithm"] = algo;
    config["nodes"].Append(node);
    node = Variant::NullType;
    node["type"] = "HBeamformerNode";
    node["param"]["inport"] = "input";
    node["param"]["outport"] = "output";
    node["param"]["estimate"] = estimate;
    node["param"]["file"] = argv[optind + 1];
    node["name"] = "hbf1";
    config["nodes"].Append(node.Copy());
    node["name"] = "hbf2";
    config["nodes"].Append(node.Copy());
    node["name"] = "hbf3";
    config["nodes"].Append(node.Copy());

    node = Variant::NullType;
    node["name"] = "input";
    node["type"] = "CPNBFInputNode";
    node["param"]["outport"] = "output";
    node["param"]["infile"] = input_file;
    node["param"]["repetitions"] = repetitions;
    config["nodes"].Append(node);
    node = Variant::NullType;

    node["name"] = "output";
    node["type"] = "CPNBFOutputNode";
    node["param"]["inports"][0] = "0";
    node["param"]["inports"][1] = "1";
    node["param"]["inports"][2] = "2";
    node["param"]["blocksize"] = 8192;
    node["param"]["outfile"] = output_file;
    config["nodes"].Append(node);
    node = Variant::NullType;

    Variant queue;
    queue["size"] = 8192*size_mult;
    queue["threshold"] = 8192;
    queue["type"] = "threshold";
    queue["datatype"] = "complex<float>";
    queue["numchannels"] = 256;
    queue["readerport"] = "input";
    queue["writerport"] = "out1";
    queue["readernode"] = "hbf1";
    queue["writernode"] = "vertical";
    config["queues"].Append(queue.Copy());
    queue["writerport"] = "out2";
    queue["readernode"] = "hbf2";
    config["queues"].Append(queue.Copy());
    queue["writerport"] = "out3";
    queue["readernode"] = "hbf3";
    config["queues"].Append(queue.Copy());
    queue["numchannels"] = 560;
    queue["readernode"] = "output";
    queue["readerport"] = "0";
    queue["writernode"] = "hbf1";
    queue["writerport"] = "output";
    config["queues"].Append(queue.Copy());
    queue["readerport"] = "1";
    queue["writernode"] = "hbf2";
    config["queues"].Append(queue.Copy());
    queue["readerport"] = "2";
    queue["writernode"] = "hbf3";
    config["queues"].Append(queue.Copy());
    queue["numchannels"] = 256*12;
    queue["readernode"] = "vertical";
    queue["readerport"] = "input";
    queue["writernode"] = "input";
    queue["writerport"] = "output";
    queue["datatype"] = "complex<int16_t>";
    config["queues"].Append(queue.Copy());

    if (print_config) {
        printf("%s\n", VariantToJSON(config, true).c_str());
        return 0;
    }
    CPN::KernelAttr kattr = VariantCPNLoader::GetKernelAttr(config);
    CPN::Kernel kernel(kattr);
    VariantCPNLoader::Setup(&kernel, config);
    kernel.WaitForAllNodeEnd();
    return 0;
}


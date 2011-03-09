#include "FlowMeasure.h"
#include "JSONToVariant.h"
#include "Kernel.h"
#include "LoadFromFile.h"
#include "NodeBase.h"
#include "NumProcs.h"
#include "ParseBool.h"
#include "PathUtils.h"
#include "IQueue.h"
#include "OQueue.h"
#include "ToString.h"
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
#include <string.h>
#include <unistd.h>
#include <vector>

#ifdef _OPENMP
#include <omp.h>
#endif

static const unsigned BLOCKSIZE = 8192;
static const unsigned OVERLAP = 2048;
using CPN::shared_ptr;
using std::complex;

class CPNBFOutputNode : public CPN::NodeBase {
public:
    CPNBFOutputNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr)
    {}

    void Process() {
        unsigned num_inports = GetParam<unsigned>("num_inports");
        unsigned blocksize = GetParam<unsigned>("blocksize");
        bool nooutput = GetParam<bool>("nooutput", !HasParam("outfile"));
        std::string outfile;
        if (!nooutput) {
            outfile = GetParam("outfile");
        }

        std::vector< CPN::IQueue< complex<float> > > in;
        std::vector< CPN::IQueue< complex<float> > >::iterator cur, begin, end;
        for (unsigned port = 0; port < num_inports; ++port) {
            std::ostringstream oss;
            oss << "in" << port;
            in.push_back(GetIQueue(oss.str()));
        }
        cur = begin = in.begin();
        end = in.end();

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
};
CPN_DECLARE_NODE_FACTORY(CPNBFOutputNode, CPNBFOutputNode);

class CPNBFInputNode : public CPN::NodeBase {
public:
    CPNBFInputNode(CPN::Kernel &ker, const CPN::NodeAttr &attr)
        : CPN::NodeBase(ker, attr)
    {}

    void Process() {
        unsigned blocksize = GetParam<unsigned>("blocksize");
        unsigned element_size = GetParam<unsigned>("element_size");
        unsigned repetitions = GetParam<unsigned>("repetitions", 1);
        bool forced_length = GetParam<bool>("forced_length", false);
        std::string infile = GetParam("infile");

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
        const unsigned data_length = header["length"].AsUnsigned();
        const unsigned numChans = header["numChans"].AsUnsigned();
        const unsigned length = blocksize * element_size;
        std::vector<char> data(length * numChans);
        for (unsigned i = 0; i < numChans; ++i) {
            unsigned numread = 0;
            while (in->good() && numread < data_length) {
                in->read(&data[i * data_length] + numread, data_length - numread);
                numread += in->gcount();
            }
        }
        if (forced_length) {
            unsigned d_len = data_length;
            while (d_len < length) {
                unsigned num_more = length - d_len;
                num_more = std::min(length, num_more);
                memcpy(&data[d_len], &data[0], num_more);
                d_len += num_more;
            }
        }
        CPN::OQueue<void> out = GetOQueue("out");
        FlowMeasure measure;
        unsigned qsize = out.QueueLength();
        unsigned rep = 0;
        unsigned written = 0;
        measure.Start();
        while (rep < repetitions && written < qsize) {
            unsigned len = std::min(qsize - written, length);
            out.Enqueue(&data[0], len, numChans, length);
            measure.Tick(len / element_size);
            written += len;
            ++rep;
        }
        for (; rep < repetitions; ++rep) {
            out.GetEnqueuePtr(length);
            out.Enqueue(length);
            measure.Tick(length / element_size);
        }
        fprintf(stderr,
                "Input:\nAvg:\t%f hz\nMax:\t%f hz\nMin:\t%f hz\n",
                measure.AverageRate(), measure.LargestRate(), measure.SmallestRate());
    }
};
CPN_DECLARE_NODE_FACTORY(CPNBFInputNode, CPNBFInputNode);

static const char* const VALID_OPTS = "h:i:o:er:R:na:s:c:f:F:S:q:p:PCv:V:j:J:l";

static const char* const HELP_OPTS = "Usage: %s [options]\n"
"\t-a n\t Use algorithm n for vertical\n"
"\t-C\t Print config and exit\n"
"\t-c y|n\t Load internal config. (default: yes)\n"
"\t-e\t Estimate FFT algorithm rather than measure.\n"
"\t-f y|n\t Use the 'fan' vertical beamformer (default: yes).\n"
"\t-F num\t Only do num fans.\n"
"\t-h file\t Use file for horizontal coefficients.\n"
"\t-i file\t Use input file\n"
"\t-j file\t Load file as JSON and merge with config.\n"
"\t-J JSON\t Load JSON and merge it with config. (allows overrides on the command line)\n"
"\t-l \t Force the input to be the full input length by repetition rather than zero fill.\n"
"\t-n \t No output, just time\n"
"\t-o file\t Use output file\n"
"\t-q xxx\t Set xxx as the queue type (default: threshold).\n"
"\t-p num\t Use num processors\n"
"\t-P\t If R is 1 don't put in Fork and Join nodes.\n"
"\t-r num\t Run num times\n"
"\t-R num\t Round robin the horizontal num.\n"
"\t-s n\t Scale queue sizes by n\n"
"\t-S y|n\t Split the horizontal beamformer (default: yes).\n"
"\t-v file\t Use file for vertial coefficients.\n"
"\t-V y|n\t Do vertical or not. If no, forces F to 1 and ignores -v.\n"
;


int cpnbf_main(int argc, char **argv) {
    if (argc == 1) {
        fprintf(stderr, HELP_OPTS, *argv);
        return 0;
    }
    bool procOpts = true;
    std::string input_file;
    std::string output_file;
    std::string vertical_config;
    std::string horizontal_config;
    std::string queue_type = "threshold";
    unsigned algo = 0;
    bool use_fan = true;
    unsigned num_fans = 3;
    unsigned num_rr = 1;
    bool estimate = false;
    unsigned repetitions = 1;
    bool nooutput = false;
    unsigned size_mult = 2;
    bool print_config = false;
    bool split_horizontal = true;
    bool load_internal_config = true;
    bool forced_length = false;
    bool do_vertical = true;
    bool rr_force = true;
    Variant config;
    config["name"] = "kernel";
    std::string nodelist = RealPath("node.list");
    if (!nodelist.empty()) {
        config["liblist"].Append(nodelist);
    }
    VariantCPNLoader loader(config);
    while (procOpts) {
        switch (getopt(argc, argv, VALID_OPTS)) {
        case 'a':
            algo = atoi(optarg);
            break;
        case 'C':
            print_config = true;
            break;
        case 'c':
            load_internal_config = ParseBool(optarg);
            break;
        case 'e':
            estimate = true;
            break;
        case 'f':
            use_fan = ParseBool(optarg);
            break;
        case 'F':
            num_fans = atoi(optarg);
            break;
        case 'h':
            horizontal_config = optarg;
            break;
        case 'i':
            input_file = optarg;
            break;
        case 'j':
            {
                JSONToVariant parser;
                FILE *f = fopen(optarg, "r");
                if (!f) {
                    fprintf(stderr, "Unable to open config file %s\n", optarg);
                    return 1;
                }
                parser.ParseFile(f);
                if (!parser.Done()) {
                    fprintf(stderr, "Error parsing config file %s on line %u column %u\n",
                            optarg, parser.GetLine(), parser.GetColumn());
                    return 1;
                }
                loader.MergeConfig(parser.Get());
            }
            break;
        case 'J':
            {
                JSONToVariant parser;
                parser.Parse(optarg, strlen(optarg));
                if (!parser.Done()) {
                    fprintf(stderr, "Error parsing command line JSON on line %u column %u\n",
                            parser.GetLine(), parser.GetColumn());
                    return 1;
                }
                loader.MergeConfig(parser.Get());
            }
            break;
        case 'l':
            forced_length = true;
            break;
        case 'n':
            nooutput = true;
            break;
        case 'o':
            output_file = optarg;
            break;
        case 'q':
            queue_type = optarg;
            break;
        case 'p':
            {
                int num_threads = atoi(optarg);
                SetNumProcs(num_threads);
#ifdef _OPENMP
                omp_set_num_threads(num_threads);
#endif
            }
            break;
        case 'P':
            rr_force = false;
            break;
        case 'r':
            repetitions = atoi(optarg);
            break;
        case 'R':
            num_rr = std::max(1,atoi(optarg));
            break;
        case 's':
            size_mult = atoi(optarg);
            break;
        case 'S':
            split_horizontal = ParseBool(optarg);
            break;
        case 'v':
            vertical_config = optarg;
            break;
        case 'V':
            do_vertical = ParseBool(optarg);
            break;
        case -1:
            procOpts = false;
            break;
        default:
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 0;
        }
    }
    if (load_internal_config) {
        if (!do_vertical) {
            num_fans = 1;
        }
        if (horizontal_config.empty()) {
            fprintf(stderr, "Must specify horitontal config.\n");
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 1;
        }
        if (vertical_config.empty() && do_vertical) {
            fprintf(stderr, "Must specify vertical config.\n");
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 1;
        }
        if (input_file.empty()) {
            fprintf(stderr, "Must have an input file.\n");
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 1;
        }
        if (output_file.empty() && !nooutput) {
            fprintf(stderr, "Either an output file or no output must be specified.\n");
            fprintf(stderr, HELP_OPTS, argv[0]);
            return 1;
        }

#define HBF_FMT "hbf_%u_%u"
#define HBF_FMT2 HBF_FMT "_2"
#define IN_FMT "in%u"
#define OUT_FMT "out%u"
#define FORK_FMT "fork_%u"
#define JOIN_FMT "join_%u"
#define INPUT "in"
#define OUTPUT "out"
        Variant node;

        if (do_vertical) {
            node["name"] = "vertical";
            if (use_fan) {
                node["type"] = "FanVBeamformerNode";
            } else {
                node["type"] = "VBeamformerNode";
            }
            node["param"]["num_outports"] = num_fans;
            node["param"]["blocksize"] = BLOCKSIZE;
            node["param"]["file"] = vertical_config;
            node["param"]["algorithm"] = algo;
            loader.AddNode(node);
        }
        node = Variant::NullType;
        node["type"] = "HBeamformerNode";
        node["param"]["estimate"] = estimate;
        node["param"]["file"] = horizontal_config;
        node["param"]["out overlap"] = OVERLAP;
        for (unsigned i = 0; i < num_fans; ++i) {
            for (unsigned j = 0; j < num_rr; ++j) {
                if (split_horizontal) {
                    node["param"]["half"] = 1;
                }
                node["name"] = ToString(HBF_FMT, i, j);
                loader.AddNode(node);
                if (split_horizontal) {
                    node["param"]["half"] = 2;
                    node["name"] = ToString(HBF_FMT2, i, j);
                    loader.AddNode(node);
                }
            }
        }

        node = Variant::NullType;
        node["name"] = "input";
        node["type"] = "CPNBFInputNode";
        node["param"]["infile"] = input_file;
        node["param"]["repetitions"] = repetitions;
        node["param"]["forced_length"] = forced_length;
        node["param"]["blocksize"] = BLOCKSIZE; //should be the same as the blocksize for the vertical beamformer.
        if (do_vertical) {
            node["param"]["element_size"] = sizeof(complex<short>);
        } else {
            node["param"]["element_size"] = sizeof(complex<float>);
        }
        loader.AddNode(node);
        node = Variant::NullType;

        node["name"] = "output";
        node["type"] = "CPNBFOutputNode";
        node["param"]["num_inports"] = num_fans;
        node["param"]["blocksize"] = BLOCKSIZE - OVERLAP;
        node["param"]["outfile"] = output_file;
        node["param"]["nooutput"] = nooutput;
        loader.AddNode(node);
        node = Variant::NullType;
        if (num_rr > 1 || rr_force) {
            node["type"] = "ForkJoinNode";
            node["param"]["size"] = BLOCKSIZE;
            node["param"]["overlap"] = OVERLAP;
            node["param"]["num_outports"] = num_rr;
            node["param"]["num_inports"] = 1;
            for (unsigned i = 0; i < num_fans; ++i) {
                node["name"] = ToString(FORK_FMT, i);
                loader.AddNode(node);
            }
            node["param"]["overlap"] = 0;
            node["param"]["num_outports"] = 1;
            node["param"]["num_inports"] = num_rr;
            node["param"]["size"] = BLOCKSIZE - OVERLAP;
            for (unsigned i = 0; i < num_fans; ++i) {
                node["name"] = ToString(JOIN_FMT, i);
                loader.AddNode(node);
            }
        }

        Variant queue;
        queue["size"] = BLOCKSIZE*size_mult*sizeof(complex<float>);
        queue["threshold"] = BLOCKSIZE*sizeof(complex<float>);
        queue["type"] = queue_type;
        queue["datatype"] = "complex<float>";
        queue["numchannels"] = 256;
        if (do_vertical) {
            queue["writernode"] = "vertical";
        } else {
            queue["writernode"] = "input";
            queue["writerport"] = OUTPUT;
        }
        for (unsigned i = 0; i < num_fans; ++i) {
            if (do_vertical) {
                queue["writerport"] = ToString(OUT_FMT, i);
            }
            if (num_rr > 1 || rr_force) {
                queue["readernode"] = ToString(FORK_FMT, i);
                queue["readerport"] = ToString(IN_FMT, 0);
            } else {
                queue["readernode"] = ToString(HBF_FMT, i, 0);
                queue["readerport"] = INPUT;
            }
            loader.AddQueue(queue);
        }
        if (num_rr > 1 || rr_force) {
            for (unsigned i = 0; i < num_fans; ++i) {
                queue["writernode"] = ToString(FORK_FMT, i);
                queue["readerport"] = INPUT;
                for (unsigned j = 0; j < num_rr; ++j) {
                    queue["writerport"] = ToString(OUT_FMT, j);
                    queue["readernode"] = ToString(HBF_FMT, i, j);
                    loader.AddQueue(queue);
                }
            }
        }

        queue["numchannels"] = 560;
        queue["readernode"] = "output";
        queue["writerport"] = OUTPUT;

        const char *wn_fmt = HBF_FMT;
        if (split_horizontal) { wn_fmt = HBF_FMT2; }
        for (unsigned i = 0; i < num_fans; ++i) {
            queue["readerport"] = ToString(IN_FMT, i);
            if (num_rr > 1 || rr_force) {
                queue["readernode"] = "output";
                queue["writernode"] = ToString(JOIN_FMT, i);
                queue["writerport"] = ToString(OUT_FMT, 0);
                loader.AddQueue(queue);
                queue["writerport"] = OUTPUT;
                queue["readernode"] = ToString(JOIN_FMT, i);
                for (unsigned j = 0; j < num_rr; ++j) {
                    queue["readerport"] = ToString(IN_FMT, j);
                    queue["writernode"] = ToString(wn_fmt, i, j);
                    loader.AddQueue(queue);
                }
            } else {
                queue["writernode"] = ToString(wn_fmt, i, 0);
                loader.AddQueue(queue);
            }
        }

        if (split_horizontal) {
            queue["size"] = BLOCKSIZE*560*size_mult*sizeof(complex<float>);
            queue["threshold"] = BLOCKSIZE*560*sizeof(complex<float>);
            queue["numchannels"] = 1;
            queue["readerport"] = INPUT;
            for (unsigned i = 0; i < num_fans; ++i) {
                for (unsigned j = 0; j < num_rr; ++j) {
                    queue["writernode"] = ToString(HBF_FMT, i, j);
                    queue["readernode"] = ToString(HBF_FMT2, i, j);
                    loader.AddQueue(queue);
                }
            }
        }
        if (do_vertical) {
            queue["size"] = BLOCKSIZE*size_mult*sizeof(complex<int16_t>);
            queue["threshold"] = BLOCKSIZE*sizeof(complex<int16_t>);
            queue["numchannels"] = 256*12;
            queue["readernode"] = "vertical";
            queue["readerport"] = INPUT;
            queue["writernode"] = "input";
            queue["writerport"] = OUTPUT;
            queue["datatype"] = "complex<int16_t>";
            loader.AddQueue(queue);
        }
    }

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
    kernel.WaitForNode("output");
    return 0;
}


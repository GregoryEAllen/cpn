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

#include "Kernel.h"
#include "QueueReaderAdapter.h"
#include "ThresholdSieveOptions.h"
#include "ErrnoException.h"
#include "VariantCPNLoader.h"
#include "ThresholdSieveDefaults.h"
#include "ThresholdSieveOptions.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"
#include "ParseBool.h"
#include <sys/times.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <fstream>

typedef ThresholdSieveOptions::NumberT NumberT;

static double getTime() {
    timeval tv;
    if (gettimeofday(&tv, 0) != 0) {
        throw ErrnoException();
    }
    return static_cast<double>(tv.tv_sec) + 1e-6 * static_cast<double>(tv.tv_usec);
}

static Variant GetDefaults() {
    JSONToVariant p;
    for (unsigned char *i = ThresholdSieveDefaults_json,
            *e = ThresholdSieveDefaults_json + ThresholdSieveDefaults_json_len;
            i != e; ++i)
    {
        p.Parse(*i);
    }
    if (!p.Done()) {
        std::cerr << "Error parsing defaults, line " << p.GetLine() << " column " << p.GetColumn() << std::endl;
        return Variant::ObjectType;
    }
    return p.Get();
}

static void PrintHelp(const std::string &progname) {
    std::cerr << "Usage: " << progname << " -hv -m maxprime -q queuesize -t threshold -f filename -p primes per filter -i iterations\n"
        "\t-h\tPrint out this message\n"
        "\t-v\tBe verbose, print out the primes found\n"
        "\t-V\tPritty print\n"
        "\t-m\tSpecify the maximum number to consider for primes (default 100)\n"
        "\t-q\tSpecify the queue size to use (default 100)\n"
        "\t-t\tSpecify the threshold to use (default 2)\n"
        "\t-f\tSpecify a file to use instead of stdout (clobbers)\n"
        "\t-pa,b,c,...\tSpecify the number of primes per filter as a polynomial (default 1)\n"
        "\t-w\tSpecify the number of primes in the producer prime wheel (default 0)\n"
        "\t-i\tRerun the given number of times\n"
        "\t-r\tReport per filter statistics\n"
        "\t-z num\tZero copy\n"
        "\t-j file\tLoad file as JSON and merge with loader config\n"
        "\t-J JSON\tLoad JSON and merge with loader config\n"
        "\t-c y|n\tUse internal config.\n"
        "\t-C\tPrint config\n"
        "\t-P file\tLoad file as JSON and merge with params to controller\n"
        "\t-d num\tDivisor for calculation of which kernel to choose when creating filters.\n"
        "\n"
        "Note that when the number of primes in the prime wheel is not zero the maximum\n"
        "number to consider for primes is not exact.\n"
        "Also, If the queue size is the same as the threshold size the sieve may deadlock.\n"
        ;
}

using CPN::QueueReaderAdapter;

static Variant SieveTest(VariantCPNLoader &loader, bool verbose) {
    CPN::Kernel kernel(loader.GetKernelAttr());
    CPN::Key_t pseudokey = 0;
    Variant result = Variant::ObjectType;
    tms tmsStart;
    tms tmsStop;
    times(&tmsStart);
    double start = getTime();
    if (verbose) {
        pseudokey = kernel.CreatePseudoNode(VERBOSE_NAME);
    }
    loader.Setup(&kernel);
    if (verbose) {
        result["result"] = Variant::ArrayType;
        QueueReaderAdapter<NumberT> out = kernel.GetPseudoReader(pseudokey, IN_PORT);
        NumberT value = 0;
        while (out.Dequeue(&value, 1)) {
            result["result"].Append(value);
        }
        out.Release();
        kernel.DestroyPseudoNode(pseudokey);
        result["numprimes"] = result["result"].Size();
    }
    kernel.WaitNodeTerminate(CONTROL_NAME);
    double stop = getTime();
    times(&tmsStop);
    result["realtime"] = stop - start;
    result["usertime"] = (double)(tmsStop.tms_utime - tmsStart.tms_utime)/(double)sysconf(_SC_CLK_TCK);
    result["systime"] = (double)(tmsStop.tms_stime - tmsStart.tms_stime)/(double)sysconf(_SC_CLK_TCK);
    return result;
}



int main(int argc, char **argv) {
    VariantCPNLoader loader;
    loader.KernelName("ThresholdSieve");
    Variant param = GetDefaults();

    int numIterations = 1;
    bool verbose = false;
    bool prettyprint = false;
    bool print_config = false;
    bool internal_config = true;;
    std::string outfile = "";
    while (true) {
        int c = getopt(argc, argv, "m:q:t:hf:i:p:vVw:rz:j:J:c:CP:d:");
        if (c == -1) break;
        switch (c) {
        case 'd':
            param["divisor"] = strtod(optarg,0);
            break;
        case 'c':
            internal_config = ParseBool(optarg);
            break;
        case 'C':
            print_config = true;
            break;
        case 'P':
            {
                JSONToVariant parser;
                std::ifstream f(optarg);
                if (!f) {
                    std::cerr << "Unable to open config file " << optarg << std::endl;
                    return 1;
                }
                parser.ParseStream(f);
                if (!parser.Done()) {
                    std::cerr << "Error parsing config file " <<
                        optarg << " on line " << parser.GetLine() <<
                        " column " << parser.GetColumn() << std::endl;
                    return 1;
                }
                Variant p = parser.Get();
                for (Variant::MapIterator i = p.MapBegin(), e = p.MapEnd();
                        i != e; ++i)
                {
                    param[i->first] = i->second;
                }
            }
            break;
        case 'j':
            {
                JSONToVariant parser;
                std::ifstream f(optarg);
                if (!f) {
                    std::cerr << "Unable to open config file " << optarg << std::endl;
                    return 1;
                }
                parser.ParseStream(f);
                if (!parser.Done()) {
                    std::cerr << "Error parsing config file " <<
                        optarg << " on line " << parser.GetLine() <<
                        " column " << parser.GetColumn() << std::endl;
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
                    std::cerr << "Error parsing command line JSON on line "
                        << parser.GetLine() << " column " << parser.GetColumn() << std::endl;
                    return 1;
                }
                loader.MergeConfig(parser.Get());
            }
            break;
        case 'w':
            {
                int nps = atoi(optarg);
                if (nps < 0) { nps = 0; }
                if (nps >= 8) { nps = 8; }
                param["primewheel"] = nps;
            }
            break;
        case 'm':
            {
                int maxprime = atoi(optarg);
                if (maxprime < 2) maxprime = 2;
                param["maxprime"] = maxprime;
            }
            break;
        case 'q':
            {
                int queuesize = atoi(optarg);
                if (queuesize < 1) queuesize = 1;
                param["queuesize"] = queuesize;
            }
            break;
        case 't':
            {
                int threshold = atoi(optarg);
                if (threshold < 2) threshold = 2;
                param["threshold"] = threshold;
            }
            break;
        case 'r':
            param["report"] = true;
            break;
        case 'z':
            param["zerocopy"] = atoi(optarg);
            break;
        case 'p':
            {
                Variant ppf = Variant::ArrayType;
                char *num = strtok(optarg, ", ");
                while (num != 0) {
                    ppf.Append(atof(num));
                    num = strtok(0, ", ");
                }
                param["ppf"] = ppf;
            }
            break;
        case 'v':
            verbose = true;
            break;
        case 'V':
            prettyprint = true;
            break;
        case 'f':
            outfile = optarg;
            break;
        case 'i':
            numIterations = atoi(optarg);
            if (numIterations < 0) numIterations = 1;
            break;
        case 'h':
            PrintHelp(argv[0]);
            return 0;
        default:
            std::cerr << "Invalid Option: " << c << std::endl;
            PrintHelp(argv[0]);
            return 0;
        }
    }

    if (internal_config) {
        if (verbose) {
            param["outputport"] = OUT_PORT;

            Variant queue = Variant::ObjectType;
            queue["size"] = param["queuesize"];
            queue["threshold"] = param["threshold"];
            queue["writernode"] = CONTROL_NAME;
            queue["writerport"] = OUT_PORT;
            queue["readerport"] = IN_PORT;
            queue["readernode"] = VERBOSE_NAME;
            loader.AddQueue(queue);
        }
        Variant node = Variant::ObjectType;
        node["name"] = CONTROL_NAME;
        node["type"] = "ThresholdSieveController";
        node["param"] = param;
        loader.AddNode(node);
    }
    if (print_config) {
        std::cout << PrettyJSON(loader.GetConfig(), true) << std::endl;
        return 0;
    }

    if (internal_config) {
        Variant output = Variant::ArrayType;
        for (int i = 0; i < numIterations; ++i) {
            Variant result = SieveTest(loader, verbose);
            result["maxprime"] = param["maxprime"];
            result["queuesize"] = param["queuesize"];
            result["threshold"] = param["threshold"];
            result["primewheel"] = param["primewheel"];
            result["ppf"] = param["ppf"];
            result["zerocopy"] = param["zerocopy"];
            output.Append(result);
        }
        if (!outfile.empty()) {
            std::ofstream out(outfile.c_str());
            out << PrettyJSON(output, prettyprint) << std::endl;
        } else {
            std::cout << PrettyJSON(output, prettyprint) << std::endl;
        }
    } else {
        CPN::Kernel kernel(loader.GetKernelAttr());
        kernel.Wait();
    }
    return 0;
}




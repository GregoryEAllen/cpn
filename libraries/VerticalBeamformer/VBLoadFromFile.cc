
#include "FanVBeamformer.h"
#include "VBeamformer.h"
#include "Assert.h"
#include "Variant.h"
#include "JSONToVariant.h"

#include <fstream>
#include <vector>

using std::complex;

std::auto_ptr<VBeamformer> VBLoadFromFile(const std::string &filename) {
    std::fstream f;
    f.open(filename.c_str(), std::fstream::in | std::fstream::binary);
    JSONToVariant parser;
    f >> parser;
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    while (f.get() != 0 && f.good());
    ASSERT(f.good());
    unsigned numFans = header["numFans"].AsUnsigned();
    unsigned numStaveTypes = header["numStaveTypes"].AsUnsigned();
    unsigned numElemsPerStave = header["numElemsPerStave"].AsUnsigned();
    unsigned filterLen = header["filterLen"].AsUnsigned();
    std::vector< float > filter(filterLen * numStaveTypes * numElemsPerStave * numFans);
    std::vector< complex<float> > bbcor(numFans * numStaveTypes * numElemsPerStave);
    unsigned numread = 0;
    unsigned numtoread = sizeof(float) * filter.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&filter[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    numread = 0;
    numtoread = sizeof(complex<float>) * bbcor.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&bbcor[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread, "%u != %u", numread, numtoread);
    return std::auto_ptr<VBeamformer>(new VBeamformer(numFans, numStaveTypes, numElemsPerStave, filterLen,
                &filter[0], &bbcor[0]));
}

std::auto_ptr<FanVBeamformer> FanVBLoadFromFile(const std::string &filename) {
    std::fstream f;
    f.open(filename.c_str(), std::fstream::in | std::fstream::binary);
    JSONToVariant parser;
    f >> parser;
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    while (f.get() != 0 && f.good());
    ASSERT(f.good());
    unsigned numFans = header["numFans"].AsUnsigned();
    unsigned numStaveTypes = header["numStaveTypes"].AsUnsigned();
    unsigned numElemsPerStave = header["numElemsPerStave"].AsUnsigned();
    unsigned filterLen = header["filterLen"].AsUnsigned();
    std::vector< float > filter(filterLen * numStaveTypes * numElemsPerStave * numFans);
    std::vector< complex<float> > bbcor(numFans * numStaveTypes * numElemsPerStave);
    unsigned numread = 0;
    unsigned numtoread = sizeof(float) * filter.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&filter[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    numread = 0;
    numtoread = sizeof(complex<float>) * bbcor.size();
    while (f.good() && numread < numtoread) {
        f.read(((char*)&bbcor[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    return std::auto_ptr<FanVBeamformer>(new FanVBeamformer(numFans, numStaveTypes, numElemsPerStave, filterLen,
                &filter[0], &bbcor[0]));
}



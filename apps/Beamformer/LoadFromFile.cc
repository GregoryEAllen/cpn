
#include "LoadFromFile.h"
#include "HBeamformer.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"
#include "Assert.h"

#include <complex>
#include <iterator>
#include <fstream>
#include <algorithm>

using std::complex;

template<typename T>
class VarListIter : public std::iterator<std::input_iterator_tag, T> {
public:
    VarListIter(Variant::ConstListIterator itr_) : itr(itr_) {}
    T operator*() { return itr->AsNumber<T>(); }
    bool operator==(const VarListIter &o) { return o.itr == itr; }
    bool operator!=(const VarListIter &o) { return o.itr != itr; }
    VarListIter operator++() { return VarListIter(++itr); }
    VarListIter operator++(int) { return VarListIter(itr++); }
private:
    Variant::ConstListIterator itr;
};


std::auto_ptr<HBeamformer> HBLoadFromFile(const std::string &filename) {
    std::fstream f;
    f.open(filename.c_str(), std::fstream::in | std::fstream::binary);

    JSONToVariant parser;
    f >> parser;
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    while (f.get() != 0 && f.good());
    ASSERT(f.good());

    unsigned numStaves = header["numStaves"].AsUnsigned();
    unsigned numBeams = header["numBeams"].AsUnsigned();
    unsigned length = header["numSamples"].AsUnsigned();
    std::vector<unsigned> staveIndex(VarListIter<unsigned>(header["staveIndex"].ListBegin()),
            VarListIter<unsigned>(header["staveIndex"].ListEnd()));
    std::vector< complex<float> > coeffs(length * numBeams);
    std::vector< complex<float> > replica(length * numBeams);

    unsigned numread = 0;
    unsigned numtoread = sizeof(complex<float>) * length * numBeams;
    while (f.good() && numread < numtoread) {
        f.read(((char*)&coeffs[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    numread = 0;
    numtoread = sizeof(complex<float>) * length * numBeams;
    while (f.good() && numread < numtoread) {
        f.read(((char*)&replica[0]) + numread, (numtoread - numread));
        numread += f.gcount();
    }
    ASSERT(numread == numtoread);
    return std::auto_ptr<HBeamformer>(new HBeamformer(length, numStaves, numBeams, &coeffs[0], &replica[0], &staveIndex[0], true));
}


unsigned DataFromFile(FILE *f, void *ptr, unsigned maxlength,
        unsigned chanstride, unsigned numchans) {
    JSONToVariant parser;
    parser.ParseFile(f);
    ASSERT(parser.Done(), "Parsing error: line %u, column %u\n",
                parser.GetLine(), parser.GetColumn());
    Variant header = parser.Get();
    unsigned length = header["length"].AsUnsigned();
    unsigned numChans = header["numChans"].AsUnsigned();
    numChans = std::min(numChans, numchans);
    while (true) {
        int c = fgetc(f);
        ASSERT(c != EOF, "Execpected EOF");
        if (c == '\0') break;
    }
    for (unsigned chan = 0; chan < numChans; ++chan) {
        unsigned toread = std::min(maxlength, length);
        unsigned numread = 0;
        char *base = ((char*)ptr) + (chanstride * chan);
        while (numread < toread && !feof(f)) {
            numread += fread(base + numread, 1, toread - numread, f);
        }
        ASSERT(!feof(f), "Unexpected EOF");
        if (numread < length) {
            int ret = fseek(f, length - numread, SEEK_CUR);
            ASSERT(ret == 0, "Seek failed");
        }
    }
    return std::min(maxlength, length);
}

unsigned DataToFile(FILE *f, const void *ptr, unsigned length,
        unsigned chanstride, unsigned numchans) {
    Variant header(Variant::ObjectType);
    header["length"] = length;
    header["numChans"] = numchans;
    std::string h = VariantToJSON(header);
    unsigned numwritten = 0;
    unsigned numtowrite = h.size();
    while (numwritten < numtowrite && !feof(f)) {
        numwritten += fwrite(h.data() + numwritten, 1, numtowrite - numwritten, f);
    }
    if (feof(f)) return 0;
    fputc('\0', f);
    for (unsigned chan = 0; chan < numchans; ++chan) {
        numwritten = 0;
        numtowrite = length;
        const char *base = ((char*)ptr) + (chanstride * chan);
        while (numwritten < numtowrite && !feof(f)) {
            numwritten += fwrite(base + numwritten, 1, numtowrite - numwritten, f);
        }
        if (feof(f)) return 0;
    }
    return length;
}


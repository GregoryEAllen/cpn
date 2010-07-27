
#include "LoadFromFile.h"
#include "Variant.h"
#include "JSONToVariant.h"
#include "VariantToJSON.h"
#include "Assert.h"

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


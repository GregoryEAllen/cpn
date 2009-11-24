
#include "Base64.h"
#include "ToString.h"
#include <unistd.h>
#include <stdio.h>
#include <math.h>

const char VALID_OPS[] = "hedi:o:sgf";

const char HELP_TEXT[] = "%s <-e|-d> [-i filename] [-o filename]\n"
"\t-e\t Encode (default)\n"
"\t-d\t Decode\n"
"\t-i file\t use file as input (default stdin)\n"
"\t-o file\t use file as output (default stdout)\n"
"\t-s\t Perform a self test";

const char DEC_TEST_FILE_FORMAT[] = "testfile%d.dec";
const char ENC_TEST_FILE_FORMAT[] = "testfile%d.enc";
const int NUM_TESTS = 10;


void Encode();
void Decode();
void SelfTest();
void GenTestFiles();
void TestFiles();

int main(int argc, char **argv) {
    bool procOpts = true;
    char *inputfile = 0;
    char *outputfile = 0;
    bool encoding = true;
    while (procOpts) {
        int opt = getopt(argc, argv, VALID_OPS);
        switch (opt) {
        case 'e':
            encoding = true;
            break;
        case 'd':
            encoding = false;
            break;
        case 'i':
            inputfile = optarg;
            break;
        case 'o':
            outputfile = optarg;
            break;
        case 's':
            SelfTest();
            return 0;
        case 'g':
            GenTestFiles();
            return 0;
        case 'f':
            TestFiles();
            return 0;
        case 'h':
            printf(HELP_TEXT, argv[0]);
            return 0;
        case -1:
            procOpts = false;
            break;
        default:
            printf("Unknown param\n");
            return 0;
        }
    }

    if (inputfile != 0) {
        if (freopen(inputfile, "r", stdin) == 0) {
            perror("Could not open file");
            return 1;
        }
    }
    if (outputfile != 0) {
        if (freopen(outputfile, "w", stdout) == 0) {
            perror("Could not open file");
            return 1;
        }
    }

    if (encoding) {
        Encode();
    } else {
        Decode();
    }
}

void Decode() {
    Base64Decoder decoder;
    char buffer[4096];
    std::vector<char> out;
    while (!feof(stdin)) {
        int numread = fread(buffer, 1, sizeof(buffer), stdin);
        if (numread > 0) {
            decoder.DecodeBlock(buffer, numread);
            out = decoder.GetPartial();
            if (fwrite(&out[0], 1, out.size(), stdout) != out.size()) {
                perror("Error writing file");
                return;
            }
        } else if (ferror(stdin)) {
            perror("Error reading file");
            return;
        }
    }
    out = decoder.BlockEnd();
    if (fwrite(&out[0], 1, out.size(), stdout) != out.size()) {
        perror("Error writing file");
        return;
    }
}

void Encode() {
    Base64Encoder encoder;
    char buffer[4096];
    std::string out;
    while (!feof(stdin)) {
        int numread = fread(buffer, 1, sizeof(buffer), stdin);
        if (numread > 0) {
            encoder.EncodeBlock(buffer, numread);
            out = encoder.GetPartial();
            if (fwrite(out.data(), 1, out.length(), stdout) != out.length()) {
                perror("Error writing file");
                return;
            }
        } else if (ferror(stdin)) {
            perror("Error reading file");
            return;
        }
    }
    out = encoder.BlockEnd();
    if (fwrite(out.data(), 1, out.length(), stdout) != out.length()) {
        perror("Error writing file");
        return;
    }
}

void SelfTest() {
    srand(1);
    for (int count = 0; count < NUM_TESTS; ++count) {
        std::vector<int> buffer;
        int num = rand();
        if (num <= 0) { num = -num + 1; }
        if (num > (1<<16)) { num %= 1<<16; }
        printf("Test %d with %d bytes\n", count, num*sizeof(int));
        for (int i = 0; i < num; ++i) {
            buffer.push_back(rand());
        }
        Base64Encoder encoder;
        encoder.EncodeBlock(&buffer[0], buffer.size()*sizeof(int));
        std::string val = encoder.BlockEnd();
        Base64Decoder decoder;
        decoder.DecodeBlock(val.data(), val.length());
        std::vector<char> result = decoder.BlockEnd();
        bool equal = (result.size() == (buffer.size() *sizeof(int)));
        if (equal) {
            if (memcmp(&result[0], &buffer[0], result.size()) != 0) {
                equal = false;
            }
        }
        if (equal) {
            printf("Test pass\n");
        } else {
            printf("Test fail\n");
        }
    }
}

void GenTestFiles() {
    srand(1);
    for (int i = 0; i < NUM_TESTS; ++i) {
        int num = rand();
        if (num <= 0) { num = -num + 1; }
        if (num > (1<<16)) { num %= 1<<16; }
        std::string filename = ToString(DEC_TEST_FILE_FORMAT, i);
        printf("Generating %s with %d bytes\n", filename.c_str(), num*sizeof(int));
        FILE *file = fopen(filename.c_str(), "w");
        if (file == 0) {
            perror("Error opening file");
            return;
        }
        for (int j = 0; j < num; ++j) {
            int val = rand();
            if (fwrite(&val, sizeof(val), 1, file) != 1) {
                perror("Error writing to file");
                fclose(file);
                return;
            }
        }
        fclose(file);
    }
}

std::vector<char> ReadFile(const std::string &filename) {
    std::vector<char> ret(256, 0);
    int num = 0;
    FILE *file = fopen(filename.c_str(), "r");
    if (file == 0) {
        perror("Failed to open file");
        exit(1);
    }
    while (!feof(file)) {
        int numread = fread(&ret[num], 1, ret.size() - num, file);
        if (numread > 0) {
            num += numread;
            if (ret.size() == num) {
                ret.resize(2*num);
            }
        }
    }
    fclose(file);
    ret.resize(num);
    return ret;
}

void TestFiles() {
    for (int count = 0; count < NUM_TESTS; ++count) {
        std::vector<char> dec = ReadFile(ToString(DEC_TEST_FILE_FORMAT, count));
        std::vector<char> enc = ReadFile(ToString(ENC_TEST_FILE_FORMAT, count));
        printf("Test %d encoding... ", count);
        Base64Encoder encoder;
        encoder.EncodeBlock(&dec[0], dec.size());
        std::string val = encoder.BlockEnd();
        bool equal = (val.size() == enc.size());
        if (equal) {
            if (memcmp(val.data(), &enc[0], enc.size()) != 0) {
                equal = false;
            }
        }
        if (equal) { printf("Pass\n"); }
        else { printf("Fail\n"); }

        printf("Test %d decoding... ", count);
        Base64Decoder decoder;
        decoder.DecodeBlock(&enc[0], enc.size());
        std::vector<char> result = decoder.BlockEnd();
        equal = (result.size() == dec.size());
        if (equal) {
            if (memcmp(&result[0], &dec[0], dec.size()) != 0) {
                equal = false;
            }
        }
        if (equal) { printf("Pass\n"); }
        else { printf("Fail\n"); }
    }
}


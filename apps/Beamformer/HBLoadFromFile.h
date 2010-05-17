
#pragma once

#include <memory>
#include <string>

class HBeamformer;

std::auto_ptr<HBeamformer> HBLoadFromFile(const std::string &filename);

unsigned HBDataFromFile(FILE *f, void *ptr, unsigned maxlength,
        unsigned chanstride, unsigned numchans);

unsigned HBDataToFile(FILE *f, const void *ptr, unsigned length,
        unsigned chanstride, unsigned numchans);


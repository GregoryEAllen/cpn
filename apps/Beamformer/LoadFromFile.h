
#pragma once

#include <memory>
#include <string>

class HBeamformer;

std::auto_ptr<HBeamformer> HBLoadFromFile(const std::string &filename);

unsigned DataFromFile(FILE *f, void *ptr, unsigned maxlength,
        unsigned chanstride, unsigned numchans);

unsigned DataToFile(FILE *f, const void *ptr, unsigned length,
        unsigned chanstride, unsigned numchans);


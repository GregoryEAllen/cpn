
#pragma once

#include <memory>
#include <string>

class HBeamformer;
class VBeamformer;
class FanVBeamformer;

std::auto_ptr<HBeamformer> HBLoadFromFile(const std::string &filename, bool estimate);

std::auto_ptr<VBeamformer> VBLoadFromFile(const std::string &filename);

std::auto_ptr<FanVBeamformer> FanVBLoadFromFile(const std::string &filename);

unsigned DataFromFile(FILE *f, void *ptr, unsigned maxlength,
        unsigned chanstride, unsigned numchans);

unsigned DataToFile(FILE *f, const void *ptr, unsigned length,
        unsigned chanstride, unsigned numchans);


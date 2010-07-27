
#pragma once

#include <stdio.h>

unsigned DataFromFile(FILE *f, void *ptr, unsigned maxlength,
        unsigned chanstride, unsigned numchans);

unsigned DataToFile(FILE *f, const void *ptr, unsigned length,
        unsigned chanstride, unsigned numchans);


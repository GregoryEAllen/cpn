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

#ifndef THRESHOLDSIEVEOPTIOINS_H
#define THRESHOLDSIEVEOPTIOINS_H
#pragma once
#include "CPNCommon.h"
#include <vector>
#include <string>

struct ThresholdSieveOptions {
    typedef unsigned long long NumberT;
    NumberT maxprime;
    NumberT filtercount;
    unsigned long queuesize;
    unsigned long threshold;
    std::vector<double> primesPerFilter;
    unsigned long numPrimesSource;
    CPN::QueueHint_t queuehint;
    std::string outputport;
    bool printprimes;
    CPN::Key_t consumerkey;
    bool report;
    int zerocopy;
    std::vector<std::string> kernels;
    int divisor;

    std::string Serialize();
    static ThresholdSieveOptions Deserialize(const std::string &str);
};

const int WRITE_COPY = 1;
const int READ_COPY = 2;
const char* const CONTROL_NAME = "Controller";
const char* const PRODUCER_NAME = "Producer";
const char* const FILTER_FORMAT = "Filter: %lu";
const char* const QUEUE_FORMAT = "Queue: %lu";
const char* const CONTROL_PORT = "Control Port";
const char* const IN_PORT = "x";
const char* const OUT_PORT = "y";

void CreateNewFilter(CPN::Kernel &kernel, ThresholdSieveOptions &opts, CPN::Key_t ourkey);

#endif


/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "clinfo.h"
#include "cudainfo.h"
#include "glinfo.h"
#include <iostream>
#include "ng/log.h"

int main(int argc, char *argv[])
{
    ng::LogSinkPrintf p;
#ifdef ANDROID
    ng::log::Logger::theGlobal()->addSink(&p);
#endif
    tfw::CLInfoCollector clinfo;
    clinfo.collect();
    std::cout << clinfo.serialize() << std::endl;
    tfw::GLInfoCollector glinfo;
    glinfo.collect();
    std::cout << glinfo.serializeEGL() << std::endl;
    std::cout << glinfo.serializeGLES() << std::endl;
    std::cout << glinfo.serializeGL() << std::endl;
    tfw::CudaInfoCollector cudaInfo;
    cudaInfo.collect();
    std::cout << cudaInfo.serialize() << std::endl;
    return 0;
}

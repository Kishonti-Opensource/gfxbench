/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/log.h"
#include "deviceinfo.h"
#include "clinfo.h"
#include "glinfo.h"
#include "cudainfo.h"
#include "metalinfo.h"
#include "keyvaluevisitor.h"
#include "osxdeviceinfocollector.h"

#include <fstream>
#include <sstream>
#include <iostream>



int main(int argc, char *argv[])
{
    sysinf::OsxDeviceInfoCollector collector;
    sysinf::Properties properties;
    properties.collect(collector);
    std::ofstream f("/Users/gergo.doczi/workspace/props.txt");
    f << properties.toJsonString(false);
    std::cout << "JDJDJD" << std::endl;
    return 0;
}

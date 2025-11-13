/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "resourcemanager.h"

#include <cstdio>
#include "checksum.h"


int main(int, char**)
{
    tfw::ResourceManager resourceManager;
    resourceManager.setHashingEnabled(true);

    std::istreambuf_iterator<char> eos;

    auto stream1 = resourceManager.open("test1.txt");
    stream1->seekg(0, std::ios::end);
    stream1->seekg(0, std::ios::beg);
    std::string(std::istreambuf_iterator<char>(*stream1), eos);
    auto stream2 = resourceManager.open("test2.txt");
    std::string(std::istreambuf_iterator<char>(*stream2), eos);

    std::vector<unsigned char> sha1 = resourceManager.getSha1();
    printf("SHA1: ");
    for (size_t i = 0; i < sha1.size(); ++i) {
        printf("%02X", sha1[i]);
    }
    printf("\n");

    return 0;
}

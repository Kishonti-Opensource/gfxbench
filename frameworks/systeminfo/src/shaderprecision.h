/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef SHADERPRECISION_H_
#define SHADERPRECISION_H_

#include <stdint.h>
#include <string>

namespace sysinf
{

class ShaderPrecision
{
public:
    ShaderPrecision();
    void calculate(int32_t w, int32_t h);
    std::string fragmentPrecisionBase64Png() const;

private:
    void setup(int32_t w, int32_t h);
    void teardown();
    void render();
    void compileShader(uint32_t shader);
    std::string getInfoLog () const;

    int32_t width_;
    int32_t height_;
    uint32_t prog_;

    std::string fragmentPrecisionBase64Png_;
};

}


#endif  // SHADERPRECISION_H_

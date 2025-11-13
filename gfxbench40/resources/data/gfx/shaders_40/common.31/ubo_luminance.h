/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ubo_bindings.h"

layout(std140, binding = LUMINANCE_BINDING_SLOT) uniform LuminanceBufferLayout
{
    //.x - adaptive luminance
    //.y - current frame average luminance
    //.zw - padding
    vec4 adaptive_avg_pad2;
};
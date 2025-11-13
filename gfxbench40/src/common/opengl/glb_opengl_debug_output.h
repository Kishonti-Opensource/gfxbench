/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLB_OPENGL_DEBUG_OUTPUT_H
#define GLB_OPENGL_DEBUG_OUTPUT_H

#include "glb_discard_functions.h"
#include <kcl_math3d.h>
#include <kcl_base.h>
#include <sstream>
#include <vector>

namespace GLB
{
    class OpenGLDebugOutput
    {
    public:
        OpenGLDebugOutput();
        void Enable();
        void Disable();

    private:
		static void GFXB_APIENTRY DebugCallback(
            KCL::enum_t source,
            KCL::enum_t type,
            KCL::uint32 id,
            KCL::enum_t severity,
            int length,
            const char *message,
            const void *userParam);
    };
};

#endif
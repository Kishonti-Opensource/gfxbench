/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMMON_SHADER_H
#define COMMON_SHADER_H


#include "platform.h"

namespace GLB
{

	KCL::uint32 initProgram(const char *vert, const char *frag, bool lowlevel, bool forceHighp);

	void deleteProgram(KCL::uint32 program);

	KCL::uint32 initShader(GLenum shaderType, const char *shaderSource);

	int glsl_log(GLuint obj, GLenum check_compile, const char *op);

}


#endif

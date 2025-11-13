/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "shader_reflection.h"

//0 - Vertex, 1 - TessControl, 2 - TessEvaluation, 3 - Geometry, 4 - Fragment, 5 - Compute
void CompileGLSL(NGL_shader_source_descriptor ssds[6], _shader_reflection &reflection);

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined USE_ANY_GL
#include "opengl/offscrman.h"
#elif defined HAVE_DX
#include "d3d11/offscrman.h"
#elif defined USE_METAL
#include "metal/offscrman.h"
#endif

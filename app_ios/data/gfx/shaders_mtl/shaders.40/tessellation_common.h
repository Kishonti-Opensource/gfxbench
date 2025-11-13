/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifndef _TESSELLATION_COMMON_H
#define _TESSELLATION_COMMON_H
using tessLevelType = mfloat;

#ifdef EMIT_QUADS
#define HAS_ABS_DISPLACEMENT 1
#elif defined(EMIT_TRIANGLES)
#define NO_ABS_DISPLACEMENT 1
#endif

#ifdef HAS_ABS_DISPLACEMENT
#define HAS_ABS_DISPLACEMENT 1
#else
#define NO_ABS_DISPLACEMENT 1
#endif


#ifdef HAS_ABS_DISPLACEMENT
#define InnerLevels(x) _InnerLevels[x]
#define OuterLevels(x) _OuterLevels[x]

struct alignas(mfloat) TessFactor
{
    mfloat _OuterLevels[4];
    mfloat _InnerLevels[2];
};
#else
#define InnerLevels(x) _levels[x + 3]
#define OuterLevels(x) _levels[x + 0]

struct alignas(mfloat4) TessFactor
{
    mfloat4 _levels;
};
#endif

struct UserPerPatchData
{
	hfloat4x4 Px;
	hfloat4x4 Py;
	hfloat4x4 Pz;
};

struct ControlData
{
	unsigned patchCount;
    unsigned instanceCount;
};

using uint = unsigned;

#endif // _TESSELLATION_COMMON_H
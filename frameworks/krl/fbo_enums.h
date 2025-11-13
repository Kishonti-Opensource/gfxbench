/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FBO_ENUMS_H
#define FBO_ENUMS_H

namespace GLB
{

	enum FBO_COLORMODE
	{
		RGB565_Linear = 1,  // setMinMagFilter(GL_LINEAR, GL_LINEAR)
		RGB888_Linear = 2,  // setMinMagFilter(GL_LINEAR, GL_LINEAR)
		RGB888_MipMap = 3,  // setMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR)
		RGB565_Nearest = 4,  // setMinMagFilter(GL_NEAREST, GL_NEAREST)
		RGBA8888_Linear = 5,  // setMinMagFilter(GL_LINEAR, GL_LINEAR)
		RGBA5551_Linear = 6,  // setMinMagFilter(GL_LINEAR, GL_LINEAR)
		RGBA8888_Nearest = 7,
        RGB888_Nearest = 8,
	};


	enum FBO_DEPTHMODE
	{
		DEPTH_None = 0,
		DEPTH_16_RB = 1,
		DEPTH_24_RB = 2,
		DEPTH_16_T = 3
	};
}

#endif

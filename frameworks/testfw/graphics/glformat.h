/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GLFORMAT_H
#define GLFORMAT_H

#include <stdint.h>
namespace tfw
{

struct GLFormat
{
	GLFormat()
		: red(5)
		, green(6)
		, blue(5)
		, alpha(-1)
		, depth(16)
		, stencil(-1)
		, fsaa(-1)
		, isExact(false)
#if defined(__QNX__)
		, visual_id(0)
#endif
	{}
	GLFormat(int32_t redBits, int32_t greenBits, int32_t blueBits, int32_t alphaBits, int32_t depthBits, int32_t stencilBits, int32_t fsaaSamples, bool exact)
		: red(redBits)
		, green(greenBits)
		, blue(blueBits)
		, alpha(alphaBits)
		, depth(depthBits)
		, stencil(stencilBits)
		, fsaa(fsaaSamples)
		, isExact(exact)
#if defined(__QNX__)
		, visual_id(0)
#endif
	{}
	int32_t red;
	int32_t green;
	int32_t blue;
	int32_t alpha;
	int32_t depth;
	int32_t stencil;
	int32_t fsaa;
	bool isExact;
#if defined(__QNX__)
	int32_t visual_id;
#endif
};

}//tfw
#endif

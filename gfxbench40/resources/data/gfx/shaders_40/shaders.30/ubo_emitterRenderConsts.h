/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
layout(std140) uniform emitterRenderConsts
{
	uniform highp		mat4 mvp;
	uniform highp		mat4 mv;
	uniform mediump		vec4 emitter_maxlifeX_sizeYZ_pad;
	uniform mediump		vec4 color_pad;
}; 
/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
struct _light
{
	//xyz - position, w - type
    vec4 position_type;
	//x - radius, y - atten, z - spotx, w - spoty
    vec4 atten_parameters;
	//xyz - color, w - unused
    vec4 color;
	//xyz - dir, w - unused
    vec4 dir;
};

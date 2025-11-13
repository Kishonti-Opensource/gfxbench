/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4 color0;
uniform float perimeter_threshold;

in float4 position;
in float2 tex_coord;
out float4 out_color { color(0) };
void main()
{
	const float border_min = perimeter_threshold;
	const float border_max = 1.0 - border_min;

	bool to_discard = tex_coord.x > border_min && tex_coord.x < border_max &&
		tex_coord.y > border_min &&  tex_coord.y < border_max;

	if( perimeter_threshold > 0.0 && to_discard )
	{
		discard;
	}
	out_color = color0;
}

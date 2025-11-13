/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform float4 color0;
uniform float perimeter_threshold;

in float3 normal;
in float3 view_dir;

out float4 out_color { color(0) };
void main()
{
	float3 n_view_dir = normalize(view_dir);

	// Front face
	float d = -dot( normalize(normal), n_view_dir);
	if(perimeter_threshold > 0.0 && d > perimeter_threshold)
	{
		discard;
	}

	// Back face
	d = -dot( normalize(-normal), n_view_dir);
	if(perimeter_threshold > 0.0 && d > perimeter_threshold)
	{
		discard;
	}

	out_color = color0;
}

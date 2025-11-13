/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform subpassInput<float> indirect_weight_texture { color(JOB_ATTACHMENT6) };

out float4 out_res { color(LIGHTING_RT_INDEX) };

void main()
{
	float weight = 1.0/(subpassLoad(indirect_weight_texture).x + 0.01);
	
	out_res = float4(weight);
}

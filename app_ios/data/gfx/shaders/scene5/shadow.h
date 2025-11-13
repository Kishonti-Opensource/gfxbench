/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined(TYPE_fragment) || defined(TYPE_compute)

half get_linear_shadow(sampler2DShadow<float> shadow_map, float3 shadow_coord)
{
#if LINEAR_SHADOW_FILTER == 1
	return half(texture(shadow_map, shadow_coord.xyz));
#else
	const float2 size = float2(SHADOWMAP_SIZE, SHADOWMAP_SIZE);
	float2 uv = shadow_coord.xy + (0.5 / size);

	float2 fraction = fract(size * uv.xy);
	uv.xy -= fraction / size;

	float ref = clamp(shadow_coord.z, 0.0, 1.0);
	
	half4 samples = textureGather(shadow_map, uv, ref);

	half sampleX0 = mix(samples.w, samples.z, fraction.x);
	half sampleX1 = mix(samples.x, samples.y, fraction.x);
	half sampleXY = mix(sampleX0, sampleX1, fraction.y);

	return sampleXY;
#endif
}

#endif
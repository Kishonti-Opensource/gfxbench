/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if EXPOSURE_ADAPTIVE
uniform float4 hdr_efw_tau;
uniform float delta_time;
#endif
uniform float4 hdr_auto_exposure_values;
uniform float2 texture_samples_inv;

#if EXPOSURE_ADAPTIVE
uniform float2 hdr_predefined_luminance;
#endif

buffer float values[] { ssbo, readonly };
buffer float4 hdr_exposure { ssbo };

shared float samples[WORKGROUP_SIZE_X];

float adaptive_exposure(float luminance, float exposure_bias)
{
	// Middle gray value [Krawczyk]
	float log10_value = log(luminance + 1.0) / log(10.0);
	float middle_gray = 1.03 - (2.0 / (2.0 + log10_value));

	// Tune the exposure with user provided value
	middle_gray = middle_gray * exp2(exposure_bias);

	float linear_exposure = middle_gray / luminance;
	linear_exposure = max(linear_exposure, hdr_auto_exposure_values.y);
	linear_exposure = min(linear_exposure, hdr_auto_exposure_values.z);
	return linear_exposure;
}

numthreads(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z);
void main()
{
	// Load the samples to the fast shared memory
	samples[gl_LocalInvocationIndex] = values[gl_LocalInvocationIndex];
	workgroupMemoryBarrierShared();

	// Parallel reduction
	for(uint s = uint(WORKGROUP_SIZE_X) / uint(2); s > uint(0); s = s >> uint(1))
	{
		if(gl_LocalInvocationIndex < s)
		{
			samples[gl_LocalInvocationIndex] += samples[gl_LocalInvocationIndex + s];
		}
		workgroupMemoryBarrierShared();
	}

	// The first thread stores the results
	if (gl_LocalInvocationIndex == uint(0))
	{
		// The avg logatimic(natural) luminance of the current frame
		float avg_lum = max(exp((samples[0] * texture_samples_inv.x) * texture_samples_inv.y), 0.001);

#if EXPOSURE_AUTO
		// Auto exposure with user bias
		float frame_lum = avg_lum;
		float linear_exposure = adaptive_exposure(frame_lum, hdr_auto_exposure_values.x);
		float exposure = log2(linear_exposure);
#elif EXPOSURE_ADAPTIVE
		// Adaptive luminance mode
		// Pattanaik's exponential decay function
		float prev_lum = hdr_exposure.z;
		prev_lum = mix(prev_lum, hdr_predefined_luminance.x, hdr_predefined_luminance.y);
		float frame_lum = prev_lum + (avg_lum - prev_lum) * (1.0 - exp(-delta_time * hdr_efw_tau.w));

		// Auto exposure with user bias
		float linear_exposure = adaptive_exposure(frame_lum, hdr_auto_exposure_values.x);
		float exposure = log2(linear_exposure);
#elif
		error - exposure mode not set
#endif
		hdr_exposure.x = exposure;
		hdr_exposure.y = linear_exposure;
		hdr_exposure.z = frame_lum;
		hdr_exposure.w = avg_lum;
	}
}

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D<float> gbuffer_depth_texture;
uniform sampler2D<half> dof_input_texture; // Original texture without blur
#ifndef DOF_MERGED_APPLY
uniform sampler2D<half> input_texture; // Blurred texture
#endif

#if !DOF_HALF_RES
uniform float4 depth_parameters;
#endif
uniform float4 dof_parameters; //.x - focus distance(dof_camera_focus + dof_focus_range), .z range, .w func param

#define dof_focus_distance dof_parameters.x
#define dof_range dof_parameters.z
#define dof_func_param dof_parameters.w

in float2 texcoord;
out half4 out_res { color(0) };
void main()
{
	float depth = textureLod(gbuffer_depth_texture, texcoord, 0.0).x;

#if !DOF_HALF_RES
	// Calculate linear depth
	depth = get_linear_depth(depth,depth_parameters);
#endif

	// Calculate depth parameters
	float f = (depth - dof_focus_distance) / dof_range;
	f = pow(clamp (f, 0.0, 1.0), dof_func_param);

	// Mix the original image with the blurry one
	half3 final = texture(dof_input_texture, texcoord).xyz;
#ifndef DOF_MERGED_APPLY
	final = mix(final, texture(input_texture, texcoord).xyz, half(f)) ; //NOTE: Use 0.0 instead of f to turn off DoF
#else
	final = mix(final, blur(texcoord).xyz, half(f)) ; //NOTE: Use 0.0 instead of f to turn off DoF
#endif

	out_res = half4(final, 1.0h);
}

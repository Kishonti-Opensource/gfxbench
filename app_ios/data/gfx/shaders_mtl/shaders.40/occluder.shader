/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#include "attribute_locations.h"

#ifndef COMPATIBLE_OCCLUDERS
#error COMPATIBLE_OCCLUDERS not defined
#endif

struct VertexInput
{
	hfloat3 in_position		[[attribute(0)]];
	hfloat3 in_normal		[[attribute(1)]];
	hfloat3 in_tangent		[[attribute(2)]];
	hfloat2 in_texcoord0	[[attribute(3)]];
	hfloat2 in_texcoord1	[[attribute(4)]];
};

struct VertexOutput
{
	hfloat4 out_position [[position]];
};

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]],
								constant hfloat4x4 * mvp [[buffer(1)]])
{
	VertexOutput output;

	output.out_position = *mvp * hfloat4(input.in_position, 1.0);
	
#if COMPATIBLE_OCCLUDERS
	output.out_position.z = (output.out_position.z + output.out_position.w)/2.0;
#endif
	
	return output;
}

#endif

#ifdef TYPE_fragment

fragment _float4 shader_main()
{			
	return _float4(1.0);
}

#endif

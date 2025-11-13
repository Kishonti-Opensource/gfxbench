/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

vertex PlanarReflectionInOut shader_main(VertexInput    vi [[ stage_in ]],
										 constant VertexUniforms *vu [[ buffer(VERTEX_UNIFORM_DATA_INDEX) ]])
{
    PlanarReflectionInOut r ;
    
    _float2 in_texcoord0 = vi.in_texcoord0.xy ;
  
    _float4 tmp;
	_float3 position;
	_float3 normal = vi.in_normal.xyz;
	_float3 tangent = vi.in_tangent.xyz;
	
	decodeFromByteVec3(normal);
	decodeFromByteVec3(tangent);

	position = vi.in_position.xyz;

	r.out_position = vu->mvp * _float4( position, 1);

	r.out_texcoord0 = v_float2(in_texcoord0);

#if defined TRANSLATE_UV
	r.out_texcoord0 += v_float2( vu->translate_uv.xy );
#endif


#if defined FOG
	_float4 fog_position = mv * vec4( position, 1.0);
	r.fog_distance = v_float( clamp( -fog_position.z * vu->fog_density, 0.0, 1.0) );
#endif
    
    return r;
}


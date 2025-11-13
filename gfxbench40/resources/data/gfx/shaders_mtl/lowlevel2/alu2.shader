/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

struct Alu2InOut
{
    hfloat4 m_Position [[position]];
	v_float2 uv;
#if !DO_EMISSIVE_AND_AMBIENT
	hfloat3 corners;
#endif
};

#ifdef TYPE_fragment

struct Alu2Uniforms
{
	// Camera
	hfloat3 view_pos;
	hfloat4 depth_parameters;
#if !DO_EMISSIVE_AND_AMBIENT
	// Lights
	hfloat4 light_pos_atten_array[NUM_LIGHTS];
	hfloat3 light_color_array[NUM_LIGHTS];
#endif
};


#if !DO_EMISSIVE_AND_AMBIENT

#define DO_LOOP_ITERATION(i) \
            { \
                _float3 light_dir = _float3(fu.light_pos_atten_array[(i)].xyz - origin_vs);                \
                _float sq_distance = dot( light_dir, light_dir);             \
                \
                _float atten_param = -fract(fu.light_pos_atten_array[(i)].w);               \
                _float atten = atten_param * sq_distance + _float(1.0);  \
                if (atten > _float(0.0))    \
                {   \
                    atten = clamp( atten, _float(0.0), _float(1.0));        \
                    \
                    light_dir = light_dir / sqrt( sq_distance); \
                            \
                    _float diffuse = dot(normal, light_dir); \
                    diffuse = clamp(diffuse, _float(0.0), _float(1.0)); \
                    \
                    _float light_intensity = fu.light_pos_atten_array[(i)].w + atten_param; \
                    result_color.xyz +=  albedo_emission.xyz * diffuse * _float(3.0) * _float3(fu.light_color_array[(i)]) * light_intensity * atten;    \
                } \
            }

#define DO_2_LOOP_ITERATIONS(i) \
    DO_LOOP_ITERATION(i) \
    DO_LOOP_ITERATION(i+1)

#define DO_4_LOOP_ITERATIONS(i) \
    DO_2_LOOP_ITERATIONS(i) \
    DO_2_LOOP_ITERATIONS(i+2)

#define DO_6_LOOP_ITERATIONS(i) \
    DO_4_LOOP_ITERATIONS(i) \
    DO_2_LOOP_ITERATIONS(i+4)

#define DO_8_LOOP_ITERATIONS(i) \
    DO_4_LOOP_ITERATIONS(i) \
    DO_4_LOOP_ITERATIONS(i+4)

#define DO_16_LOOP_ITERATIONS(i) \
    DO_8_LOOP_ITERATIONS(i) \
    DO_8_LOOP_ITERATIONS(i+8)

#endif


fragment _float4 shader_main(         Alu2InOut         in            [[ stage_in   ]],
							 constant Alu2Uniforms&     fu            [[ buffer(1)  ]],
							          texture2d<_float> texture_unit0 [[ texture(0) ]],
							          texture2d<_float> texture_unit1 [[ texture(1) ]],
							          texture2d<hfloat> texture_unit3 [[ texture(2) ]],
							          sampler           sampler0      [[ sampler(0) ]])
{
#if DO_EMISSIVE_AND_AMBIENT
    _float4 albedo_emission = texture_unit0.sample(sampler0, hfloat2(in.uv));
    _float4 result_color = _float4(albedo_emission.xyz, 1.0);
    if (albedo_emission.w < _float(1.0))
    {
        result_color.xyz *= _float3(0.0/255.0, 30.0/255.0, 50.0/255.0);
    }
    result_color.xyz = mix(result_color.xyz, albedo_emission.xyz, albedo_emission.w);
	return result_color;
#else
    _float4 albedo_emission = texture_unit0.sample(sampler0, hfloat2(in.uv));
    hfloat depth  = texture_unit3.sample( sampler0, hfloat2(in.uv)).x;
    _float4 result_color = _float4(0.0, 0.0, 0.0, 1.0);
    if (depth < 1.0)
    {
        if (albedo_emission.w < _float(1.0))
        {
            hfloat d = fu.depth_parameters.y / (depth - fu.depth_parameters.x);
            hfloat3 origin_vs = d * in.corners + fu.view_pos.xyz;

            _float3 view_dir = _float3(normalize( origin_vs - fu.view_pos));
            _float3 normal = texture_unit1.sample(sampler0, hfloat2(in.uv)).xyz * _float(2.0) - _float(1.0);
			
            DO_16_LOOP_ITERATIONS(0)
            result_color.xyz *= _float(1.0) - albedo_emission.w;
        }
    }
    return result_color;
#endif
}
#endif

#ifdef TYPE_vertex


struct VertexInput
{
    hfloat4 pos_uv [[attribute(0)]];
#if !DO_EMISSIVE_AND_AMBIENT
	hfloat4 corner [[attribute(1)]];
#endif
};

vertex Alu2InOut shader_main(VertexInput in [[ stage_in ]])
{
	Alu2InOut res;
	
    res.uv = v_float2(in.pos_uv.zw);
#if !DO_EMISSIVE_AND_AMBIENT
    res.corners = in.corner.xyz;
#endif
    res.m_Position = _float4(in.pos_uv.xy, 0.0, 1.0);
    
    return res;
}

#endif


/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_fragment

uniform sampler2D texture_unit0; //color map
uniform sampler2D texture_unit1; //normal-emission map
uniform highp sampler2D texture_unit3; //depth map

// Camera
uniform vec3 view_pos;
uniform vec4 depth_parameters;

#if !DO_EMISSIVE_AND_AMBIENT
// Lights
uniform highp vec4 light_pos_atten_array[NUM_LIGHTS];
uniform vec3 light_color_array[NUM_LIGHTS];

#define DO_LOOP_ITERATION(i) \
            { \
                vec3 light_dir = light_pos_atten_array[(i)].xyz - origin_vs;                \
                float sq_distance = dot( light_dir, light_dir);             \
                \
                float atten_param = -fract(light_pos_atten_array[(i)].w);               \
                float atten = atten_param * sq_distance + 1.0;  \
                if (atten > 0.0)    \
                {   \
                    atten = clamp( atten, 0.0, 1.0);        \
                    \
                    light_dir = light_dir / sqrt( sq_distance); \
                            \
                    float diffuse = dot(normal, light_dir); \
                    diffuse = clamp(diffuse, 0.0, 1.0); \
                    \
                    float light_intensity = light_pos_atten_array[(i)].w + atten_param; \
                    result_color.xyz +=  albedo_emission.xyz * diffuse * 3.0 * light_color_array[(i)] * light_intensity * atten;    \
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

in vec2 uv;
#if !DO_EMISSIVE_AND_AMBIENT
in vec3 corners;
#endif
out vec4 frag_color;

void main()
{
#if DO_EMISSIVE_AND_AMBIENT
    vec4 albedo_emission = texture(texture_unit0, uv);
    vec4 result_color = vec4(albedo_emission.xyz, 1.0);
    if (albedo_emission.w < 1.0)
    {
        result_color.xyz *= vec3(0.0/255.0, 30.0/255.0, 50.0/255.0);
    }
    result_color.xyz = mix(result_color.xyz, albedo_emission.xyz, albedo_emission.w);
	frag_color = result_color;
#else
    vec4 albedo_emission = texture(texture_unit0, uv);
    highp float depth  = texture( texture_unit3, uv).x;
    vec4 result_color = vec4(0.0, 0.0, 0.0, 1.0);
    if (depth < 1.0)
    {
        if (albedo_emission.w < 1.0)
        {
            highp float d = depth_parameters.y / (depth - depth_parameters.x);
            highp vec3 origin_vs = d * corners + view_pos.xyz;

            vec3 view_dir = normalize( origin_vs - view_pos);
            vec3 normal = texture(texture_unit1, uv).xyz * 2.0 - 1.0;
			
            DO_16_LOOP_ITERATIONS(0)
            result_color.xyz *= 1.0 - albedo_emission.w;
        }
    }
    frag_color = result_color;
#endif
}
#endif

#ifdef TYPE_vertex
in vec4 in_pos_uv;
out vec2 uv;

#if !DO_EMISSIVE_AND_AMBIENT
in vec4 in_corner;
out vec3 corners;
#endif

void main()
{
    uv = in_pos_uv.zw;
#if !DO_EMISSIVE_AND_AMBIENT
    corners = in_corner.xyz;
#endif
    gl_Position = vec4(in_pos_uv.xy, 0.0, 1.0);
}
#endif


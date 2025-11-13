/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef RG16F_SUPPORTED
#define TEX_TYPE rg16f
#else
#define TEX_TYPE rgba8
#endif

//layout (TEX_TYPE, binding = IN_TEX_BIND) uniform readonly mediump image2D in_tex;

layout(std430, binding = MAX_BUFFER_BIND) buffer MaxBuffer
{
    vec2 v[];
} max_buffer;

uniform mediump sampler2D velocity_texture;

shared vec3 samples[THREAD_COUNT];

vec2 tile_corner_uv;

vec3 fetchVelocity(int index)
{
    ivec2 coords;
    coords.y = index / K;
    coords.x = index - coords.y * K;
    if (coords.x >= K || coords.y >= K)
    {
        return vec3(0.0);
    }
    
    vec2 uv = vec2(coords) * STEP_UV + tile_corner_uv;
    if (uv.x > 1.0 || uv.y > 1.0)
    {
        return vec3(0.0);
    }
    
    //vec2 velocity = unpackVec2FFromVec4(texture(velocity_texture, uv));
    vec2 velocity = unpack_velocity(velocity_texture, uv);
    return vec3(velocity, dot(velocity, velocity));
}

layout (local_size_x = THREAD_COUNT, local_size_y = 1, local_size_z = 1) in;
void main()
{
    vec3 max_velocity = vec3(0.0, 0.0, -1.0);

    tile_corner_uv = vec2(gl_WorkGroupID.xy) / vec2(gl_NumWorkGroups.xy);
    
    for (int i = 0; i < PASS_COUNT; i++)
    {
        samples[gl_LocalInvocationIndex].xyz = fetchVelocity(int(gl_LocalInvocationIndex) + i * THREAD_COUNT);
        memoryBarrierShared();
        barrier();

        // Parallel reduction
        for(uint s = uint(THREAD_COUNT) / 2u; s > 0u; s >>= 1u)
        {
            if(gl_LocalInvocationIndex < s)
            {
                vec3 velocity = samples[gl_LocalInvocationIndex + s];
                if (velocity.z > samples[gl_LocalInvocationIndex].z)
                {
                    samples[gl_LocalInvocationIndex] = velocity;
                }
            }
            memoryBarrierShared();
            barrier();
        }

        if (gl_LocalInvocationIndex == 0u)
        {
            if (samples[0].z > max_velocity.z)
            {
                max_velocity = samples[0];
            }
        }
        barrier();
    }

    if (gl_LocalInvocationIndex == 0u)
    {
        max_buffer.v[gl_WorkGroupID.y * gl_NumWorkGroups.x + gl_WorkGroupID.x] = max_velocity.xy;
    }
}

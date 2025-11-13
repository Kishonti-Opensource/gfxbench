/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

#define MAX_PARTICLES 128
#define WORK_GROUP_WIDTH 16
#define WORK_GROUP_HEIGHT 8	
#define M_PI 3.141592654
#define WFT_SINE 0

#define m_age particles[particle_idx].Age_Speed_Accel.x
#define m_speed particles[particle_idx].Age_Speed_Accel.y
#define m_acceleration particles[particle_idx].Age_Speed_Accel.z

#define m_amplitude particles[particle_idx].Amplitude.xyz
#define m_phase particles[particle_idx].Phase.xyz
#define m_frequency particles[particle_idx].Frequency.xyz
#define m_t particles[particle_idx].T.xyz
#define m_b particles[particle_idx].B.xyz
#define m_n particles[particle_idx].N.xyz
#define m_velocity particles[particle_idx].Velocity.xyz
#define m_pos particles[particle_idx].Pos.xyz

struct _particle
{
	vec4 Pos; // particle instance's position
	vec4 Velocity;
	vec4 T;
	vec4 B;
	vec4 N;
	vec4 Phase;
	vec4 Frequency;
	vec4 Amplitude;
	vec4 Age_Speed_Accel;
};




layout (local_size_x = WORK_GROUP_WIDTH, local_size_y = WORK_GROUP_HEIGHT) in;

layout (std430, binding = 0) buffer BufferObject
{
    _particle particles[MAX_PARTICLES];
};

layout( binding = 0, rgba8) writeonly uniform mediump image3D outTexture;

uniform samplerCube envmap0;
uniform sampler2D shadow_map;
uniform mat4 shadow_matrix;

#ifdef TYPE_compute
void main() 
{
	const int vol_size = 32;
	for( int z=0; z<vol_size; z++)
	{
		vec4 color = vec4( 0.0);
		ivec3 P = ivec3(gl_GlobalInvocationID.xy, z);
		vec3 position_in_volume = vec3( P) / vec3( float(vol_size) - 1.0);
		
		for( int particle_idx=0; particle_idx<MAX_PARTICLES; particle_idx++)
		{
			float radius = 0.1 + m_age * 0.15;

					
			vec3 diff = position_in_volume - m_pos;
			float d = dot( diff, diff);

			float tt = radius * radius - d;
			
			if( tt > 0.0)
			{
				const float displacement_scale = 0.5;

				float displacement = texture( envmap0, diff).x;

				if( displacement * displacement_scale * radius * radius < tt)
				{
					vec3 n = normalize( diff);
					
#if 0
					vec4 j = shadow_matrix * vec4( position_in_volume * 2.0, 1.0);
					j.xyz /= j.w;

					float s = texture( shadow_map, j.xy).x;
					
					if( s != 1.0)
					{
						s = 0.0;
					}
#endif
					const vec3 light_color = vec3( 1.0, 0.8, 0.4) *0.5;
					const vec3 light_dir = vec3( -1.0, 0.0, 0.0);
					
					float f = dot( n, light_dir);
					f = clamp( f, 0.0, 1.0);
					// f *= s;
					// f *= 1.0 - displacement;
					f += 0.1;
					vec3 color0 = f * light_color;
					
					color.xyz += color0;
					color.a += 0.02;
				}
			}
		}
		
		imageStore( outTexture, P, vec4( color)); 
	}
}
#endif

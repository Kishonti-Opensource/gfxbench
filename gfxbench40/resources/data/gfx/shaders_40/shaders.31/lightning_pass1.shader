/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "tbds_header.shader"
#include "lightning_header.shader"

#define WORK_GROUP_SIZE 16

#define MIN_SEGMENT_LENGTH				1.0
#define MIN_AGE							500.0
#define MAX_AGE							900.0

#define BONE_INTERPOLATOR_SPEED 		0.5

layout(std430, binding = 0) buffer lightning_buffer_layout
{
	float vertex_counts[LIGHTNING_COUNT];
	vec4 positions[];
} lightning_buffer;

layout(std430, binding = 1) buffer endpoints_layout
{
	vec4 positions[];
} endpoints;

struct LightInstancingStruct
{
	highp vec4 model0;
	highp vec4 model1;
	highp vec4 model2;
	highp vec4 model3;
	
	highp vec4 light_color;
	highp vec4 light_pos;
	highp vec4 attenuation_parameter;
	highp vec4 light_x;
	highp vec4 spot_cos; 
};

layout (std430, binding = 2) buffer lights_layout
{
    LightInstancingStruct lights[];
} lights_buffer;

#ifdef GL_ES
layout(rgba8, binding = 0) readonly uniform mediump image2D noise_map;
#else
layout(rgba8, binding = 0) readonly uniform image2D noise_map;
#endif

layout(location = 0) uniform float time;

layout(location = 1) uniform vec3 actor_pos;
layout(location = 2) uniform vec3 actor_right;

layout(location = 3) uniform uint bone_segment_count;
layout(location = 4) uniform vec4 bone_segments[28];

shared int iterations;
shared float life_time;
shared int generation;

vec3 begin_point;
vec3 end_point;

void store_vertex(int lightning_index, int index, vec4 value)
{
	lightning_buffer.positions[BUFFER_SIZE * lightning_index + index] = value;
}

vec4 load_vertex(int lightning_index, int index)
{
	return lightning_buffer.positions[BUFFER_SIZE * lightning_index + index];
}

vec3 get_endpoint(int index)
{
	return endpoints.positions[index].xyz;
}

void store_light(int index, vec3 pos, float f, float r)
{
	float atten = -1.0 / (r * r);

	mat4 model_matrix = mat4(r * 1.25);	
	lights_buffer.lights[index].model0  = vec4(r * 1.25, 0.0, 0.0, 0.0);
	lights_buffer.lights[index].model1  = vec4(0.0, r * 1.25,  0.0, 0.0);
	lights_buffer.lights[index].model2 = vec4(0.0, 0.0, r * 1.25,  0.0);
	lights_buffer.lights[index].model3 = vec4(pos.x, pos.y, pos.z, 1.0);
	
	lights_buffer.lights[index].light_color = vec4(f, f, f, 0.0);
	lights_buffer.lights[index].light_pos = vec4(pos, 0.0);
	lights_buffer.lights[index].attenuation_parameter = vec4(atten, 0.0, 0.0, 0.0);
	
	// Only for spot lights:
	//lights_buffer.lights[index].light_x = vec4(0.0, 0.0, 0.0, 0.0);
	//lights_buffer.lights[index].spot_cos = vec4(0.0, 0.0, 0.0, 0.0);	
}

vec3 rand_unsigned(int idx)
{
	float factor = float(idx % 17)  + 1.0;
	int time_factor = (int(time * 0.04 * factor) + idx) % 1024;
	return imageLoad(noise_map, ivec2(time_factor, 1)).xyz;
}

vec3 rand_signed(int idx)
{
	return rand_unsigned(idx) * vec3(2.0) - vec3(1.0);
}

vec3 rand2_unsigned(int idx, int idx2)
{
	return imageLoad(noise_map, ivec2(idx2 % 1000, (idx % 1000) + 4)).xyz;
}

vec3 rand2_signed(int idx, int idx2)
{
	return rand2_unsigned(idx, idx2) * vec3(2.0) - vec3(1.0);
}

vec3 step_rand_unsigned(int idx)
{
	float factor = float(idx % 17)  + 1.0;
	int time_factor = (int(time * 0.09 * factor) + idx) % 1024;
	return imageLoad(noise_map, ivec2(time_factor, 2)).xyz;
}

vec3 step_rand_signed(int idx)
{
	return step_rand_unsigned(idx) * vec3(2.0) - vec3(1.0);
}

int get_bone_id(int lightning_index, int generation)
{		
	int slot_id = lightning_index * 102 + (generation % 34) * 3 + 0;	
	float rnd_value = (imageLoad(noise_map, ivec2(slot_id, 0)).x);	
	return int(round(rnd_value * float(bone_segment_count)));
}

float get_intensity(int lightning_index, int generation, float life_time)
{
	int slot_id = int(float((lightning_index + generation) % 10) * 102.0 + 102.0 * life_time);
	return imageLoad(noise_map, ivec2(slot_id, 3)).x + 0.3;
}

vec3 find_endpoint(int generation, vec3 begin_point, float interpolator)
{	
	// The begin point can be on the left or the right side of the actor
    // Small offset is added to avoid precision issues    
    float begin_point_sign = sign(dot(actor_right, begin_point - actor_pos) - 0.001);
    
	// If the begin point is higher then the actor.y * 1.5
	bool high_point = begin_point.y > actor_pos.y * 1.5;
	
	int start_index;
	int end_index = 0;
	
	if (high_point && generation % 3 == 1)
	{
		start_index = SKY_ENDPOINT_OFFSET;
		end_index = ENDPOINT_COUNT;		
	}
	else
	{
		start_index = GROUND_ENDPOINT_OFFSET;
		end_index = SKY_ENDPOINT_OFFSET;				
	}	
    
    // Add some lightning to the alley
    if (time > 16500.0 && time < 23000.0 && generation % 5 == 1)
    {
        start_index = int(rand2_unsigned(generation, 4).x * 80.0);
        end_index = 81;
    }
			
	int end_point_index = start_index;
	float min_dist = 999.0;	
	for(int i = start_index; i < end_index - 1; i++)
	{
		vec3 test_point = get_endpoint(i);
		float end_point_sign = sign(dot(actor_right, test_point - actor_pos));		
		if (begin_point_sign != 0.0 && begin_point_sign != end_point_sign)
		{
			// The end point is on the other side of the actor
			continue;
		}
        
        // Randomly skip some endpoints
        if (rand2_unsigned(i, generation).x > 0.5)
        {
            continue;
        }
        
		// Choose the closest point
		float test_dist = distance(begin_point, test_point);
		if(min_dist > test_dist)
		{
			end_point_index = i;
			min_dist = test_dist;
		} 
	}		
	
	// Interpolate between the two endpoints         
	vec3 end_point_1 = get_endpoint(end_point_index + 0);
	vec3 end_point_2 = get_endpoint(end_point_index + 1);        
	return mix(end_point_1, end_point_2, interpolator);
}


layout (local_size_x = WORK_GROUP_SIZE, local_size_y = 1, local_size_z = 1) in;
void main()
{	
	int lightning_index = int(gl_WorkGroupID.x);
	int local_id = int(gl_LocalInvocationID.x);
	int global_id = int(gl_GlobalInvocationID.x);
    
    // Only used by the first thread in the group
    float intensity = 0.0;
		
	// First determinate the begin and end points of the lightning
	if (local_id == 0)
	{		
		float max_age = mix(MIN_AGE, MAX_AGE, float(lightning_index) / float(LIGHTNING_COUNT));
		generation = int(floor(time / max_age));
		float age = time - float(generation) * max_age;
				
		// life_time variable goes from 1.0 to 0.0		
		life_time = (max_age - age) / max_age;
		
		// Get a bone for the lightning and calculate the begin point
		int bone_index = get_bone_id(lightning_index, generation);
		float bone_interpolator = life_time * BONE_INTERPOLATOR_SPEED;
		
        // Add lighting to the gun of the robot         
		if (lightning_index == 2 && time > 55000.0)		
		{
			bone_index = 10;
            bone_interpolator = life_time;
            if (generation % 2 == 1)
            {
                bone_interpolator = 1.0 - bone_interpolator;
            }           
		}		
		
		vec3 bone_pos0 = bone_segments[bone_index * 2 + 0].xyz;
		vec3 bone_pos1 = bone_segments[bone_index * 2 + 1].xyz;			
		
		bone_interpolator = clamp(bone_interpolator, 0.0, 1.0);
		begin_point = mix(bone_pos0, bone_pos1, bone_interpolator);
		
		// Get the end point for the lightning
		float end_point_interpolator = 0.0;
		if (life_time < 0.2)
		{
			end_point_interpolator = 0.0;
		}
		
		end_point = find_endpoint(generation, begin_point, end_point_interpolator);
		//end_point = find_endpoint(generation, bone_pos0, end_point_interpolator);
        
		// Get the intensity 
		intensity = get_intensity(lightning_index, generation, 1.0 - life_time);				
		// Add the lightning to the pole 				
		if (lightning_index == 0 && time < 10000.0)		
		{
			float interpolator = 1.0 - float(int(time) % 1200) / 1000.0;    
			interpolator = clamp(interpolator, 0.0, 1.0);	
			begin_point =  get_endpoint(3);
			end_point = mix(get_endpoint(0), get_endpoint(1), interpolator);
		}
	
		store_vertex(lightning_index, 0, vec4(begin_point, pack_w(intensity, 1.0)));
		store_vertex(lightning_index, 1, vec4(end_point, pack_w(intensity, 0.0)));	
        
        // Adaptive division: lightning length is between about 8 and 77
        // Iterations should be between 4 and 9
        float lightning_length = distance(begin_point, end_point);
        iterations = (int(lightning_length) - 10) / 4 + 4;   
        iterations = clamp(iterations, 4, 9);
        //iterations = 0;
	}		
	
	// Wait for the first thread (Order shared and buffer memory transactions)
	memoryBarrier();
	barrier();	

    // Build up the lightning with midpoint displacement algorithm
    // First we only have 1 segment. Every iteration duplicates the number of segments. 
    int num_segments = 1;    
    for (int i = 0; i < iterations; i++)
    {    
        // Distribute the segments between the threads
        int pass_count = (num_segments + WORK_GROUP_SIZE - 1) / WORK_GROUP_SIZE;                  
        for (int passIt = 0; passIt < pass_count; passIt++)
        {        
            // The index of the segment that we work with
            int idx = passIt * WORK_GROUP_SIZE + local_id;
            if (idx >= num_segments)
            {
                continue;
            }
            
            // The index of the new segment
            int new_pos = num_segments + idx;
            
            // Get the two vertices
            vec4 a = load_vertex(lightning_index, idx * 2 + 0);
            vec4 b = load_vertex(lightning_index, idx * 2 + 1);
            
            float dist = length(a.xyz - b.xyz);                                        
            int rnd_seed = num_segments * 2 + idx+  generation;
            vec3 random = rand2_signed(rnd_seed, lightning_index);
            
            // Create a new branche          
            //if (passIt > 0 && random.x > -1.5 && dist > MIN_SEGMENT_LENGTH)
            if (i < iterations - 3 && idx > 0 && random.x > 0.3 && dist > MIN_SEGMENT_LENGTH )
            {            
                // Unpack the intensity and the distance from the endpoint			
                vec2 packed_a = unpack_w(a.w);
                vec2 packed_b = unpack_w(b.w);
                float intensity_a = packed_a.x;
                float intensity_b = packed_b.x;
                float end_point_distance_a = packed_a.y;									
                float end_point_distance_b = packed_b.y;
            
                // The length factor of the branche
                float branche_delta = dist * 0.204;
            
                // Make the branche less intensive
                float mid_intensity = intensity_a * 0.7;         
             
                // Get the endpoint of the branche by translate the 'a' point to the direction of the midpoint 
                vec3 midpoint_dir = b.xyz - a.xyz;
                
                int rnd_idx1 = num_segments + lightning_index + generation;
                vec3 offset = rand2_signed(int(float(rnd_idx1 + 37) * time * 0.00006), lightning_index) * branche_delta;		
                                  
                // Average the segment distance
                float new_endpoint_distance = (end_point_distance_a + end_point_distance_b) * 0.5;               
                
                offset = rand2_signed(0, lightning_index) * branche_delta;
                vec3 new_endpoint = a.xyz + midpoint_dir * 0.9 + offset;
                
                // Store the new branche                
                store_vertex(lightning_index, new_pos * 2 + 0, vec4(a.xyz, pack_w(mid_intensity, new_endpoint_distance)));
                store_vertex(lightning_index, new_pos * 2 + 1, vec4(new_endpoint, pack_w(mid_intensity, new_endpoint_distance)));	
            }
            // Split the segment
            else if (dist > MIN_SEGMENT_LENGTH)
            {		
                 // The center of the segment
                vec3 mid = (a.xyz + b.xyz) * 0.5;
            
                // Unpack the intensity and the distance from the endpoint			
                vec2 packed_a = unpack_w(a.w);
                vec2 packed_b = unpack_w(b.w);
                float intensity_a = packed_a.x;
                float intensity_b = packed_b.x;
                float end_point_distance_a = packed_a.y;									
                float end_point_distance_b = packed_b.y;
            
                // The direction of the segment	
                vec3 dir = normalize(b.xyz - a.xyz);
                
                // The length factor of the midpoint
                float mid_delta = dist * 0.17; //0.17
                
                float interpolator = life_time * 2.0;
                interpolator = clamp(life_time, 0.0, 0.6);

                int seed = num_segments + local_id + generation;                
                vec3 random1 = rand2_signed(seed, lightning_index);
                vec3 random2 = rand2_signed(seed + 10, lightning_index);                
                vec3 rnd_vector = mix(random1, random2, interpolator) * mid_delta;
                float d = dot(dir, rnd_vector);
                rnd_vector = rnd_vector - dir * d;

                // Modify the midpoint
                mid += rnd_vector;			
                    
                // Average the segment distance
                float new_endpoint_distance = (end_point_distance_a + end_point_distance_b) / 2.0;
                                
                // Replace and add the new segments                                
                //store_vertex(lightning_index, idx * 2 + 0, a);
                store_vertex(lightning_index, idx * 2 + 1, vec4(mid, pack_w(intensity_a, new_endpoint_distance)));	
                    
                store_vertex(lightning_index, new_pos * 2 + 0, vec4(mid, pack_w(intensity_b, new_endpoint_distance)));
                store_vertex(lightning_index, new_pos * 2 + 1, b);	      
            }      
            // Just copy the segment    
            else
            {
                store_vertex(lightning_index, new_pos * 2 + 0, a);
                store_vertex(lightning_index, new_pos * 2 + 1, b);
            }
        }

        // After the barrier all the threads have created one more segment.        
        num_segments = 2 * num_segments;
        memoryBarrierBuffer();
        barrier();	
    }
    
	if (local_id == 0)
	{
		// Write out the number of vertices
		lightning_buffer.vertex_counts[lightning_index] = float(num_segments * 2);
		
		// Add lights to the first and the last segments
		vec3 first_segment = load_vertex(lightning_index, 1).xyz;
		vec3 light_dir = normalize(first_segment - begin_point);			
		
		store_light(lightning_index * 2, begin_point + light_dir * 2.0, intensity, 10.0);		
		store_light(lightning_index * 2 + 1, end_point, intensity, 10.0);
	}
}

#endif

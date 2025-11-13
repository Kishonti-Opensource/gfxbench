/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef TYPE_compute

#include "lightning_pass1.h"
#include "lightning_header.shader"

#define WORK_GROUP_SIZE LP1_WORK_GROUP_SIZE

#define MIN_SEGMENT_LENGTH				1.0
#define MIN_AGE							500.0
#define MAX_AGE							900.0

#define BONE_INTERPOLATOR_SPEED 		0.5

struct lightning_buffer_layout
{
	_float  vertex_counts[LIGHTNING_COUNT];
	_float4 positions[MAX_LIGHTNING_BUFFER];
} ; //lightning_buffer;

struct LightInstancingStruct
{
	hfloat4 model0;
	hfloat4 model1;
	hfloat4 model2;
	hfloat4 model3;
	
	hfloat4 light_color;
	hfloat4 light_pos;
	hfloat4 attenuation_parameter;
	hfloat4 light_x;
	hfloat4 spot_cos; 
};


void store_vertex(int lightning_index, int index, _float4 value, device lightning_buffer_layout* lightning_buffer)
{
	lightning_buffer->positions[BUFFER_SIZE * lightning_index + index] = value;
}

_float4 load_vertex(int lightning_index, int index, device lightning_buffer_layout* lightning_buffer)
{
	return lightning_buffer->positions[BUFFER_SIZE * lightning_index + index];
}

_float3 get_endpoint(int index, device _float4* endpoints)
{
	return endpoints[index].xyz;
}

void store_light(int index, _float3 pos, _float f, _float r, device LightInstancingStruct* lights_buffer)
{
	_float atten = -1.0 / (r * r);

	_float4x4 model_matrix = _float4x4(r * _float(1.25) );	
	lights_buffer[index].model0  = _float4(r * 1.25, 0.0, 0.0, 0.0);
	lights_buffer[index].model1  = _float4(0.0, r * 1.25,  0.0, 0.0);
	lights_buffer[index].model2 = _float4(0.0, 0.0, r * 1.25,  0.0);
	lights_buffer[index].model3 = _float4(pos.x, pos.y, pos.z, 1.0);
	
	lights_buffer[index].light_color = _float4(f, f, f, 0.0);
	lights_buffer[index].light_pos = _float4(pos, 0.0);
	lights_buffer[index].attenuation_parameter = _float4(atten, 0.0, 0.0, 0.0);
	
	// Only for spot lights:
	//lights_buffer.lights[index].light_x = _float4(0.0, 0.0, 0.0, 0.0);
	//lights_buffer.lights[index].spot_cos = _float4(0.0, 0.0, 0.0, 0.0);	
}

_float3 rand_unsigned(int idx, constant lightning_pass1_uniforms* lu, texture2d<_float> noise_map)
{
	_float factor = _float(idx % 17)  + 1.0;
	int time_factor = (int(lu->time * 0.04 * factor) + idx) % 1024;
	return noise_map.read(uint2(time_factor, 1)).xyz ;
}

_float3 rand_signed(int idx, constant lightning_pass1_uniforms* lu, texture2d<_float> noise_map)
{
	return rand_unsigned(idx, lu, noise_map) * _float3(2.0) - _float3(1.0);
}

_float3 rand2_unsigned(int idx, int idx2, texture2d<_float> noise_map)
{
	return noise_map.read(uint2(idx2 % 1000, (idx % 1000) + 4)).xyz ;
}

_float3 rand2_signed(int idx, int idx2, texture2d<_float> noise_map)
{
	return rand2_unsigned(idx, idx2, noise_map) * _float3(2.0) - _float3(1.0);
}

_float3 step_rand_unsigned(int idx, constant lightning_pass1_uniforms* lu, texture2d<_float> noise_map)
{
	_float factor = _float(idx % 17)  + 1.0;
	int time_factor = (int(lu->time * 0.09 * factor) + idx) % 1024;
	return noise_map.read(uint2(time_factor, 2)).xyz ;
}

_float3 step_rand_signed(int idx, constant lightning_pass1_uniforms* lu, texture2d<_float> noise_map)
{
	return step_rand_unsigned(idx,lu, noise_map) * _float3(2.0) - _float3(1.0);
}

int get_bone_id(int lightning_index, int generation, constant lightning_pass1_uniforms* lu, texture2d<_float> noise_map)
{		
	int slot_id = lightning_index * 102 + (generation % 34) * 3 + 0;	
	_float rnd_value = (noise_map.read(uint2(slot_id, 0)).x);	
	return int(round(rnd_value * _float(lu->bone_segment_count)));
}

_float get_intensity(int lightning_index, int generation, _float life_time, texture2d<_float> noise_map)
{
	int slot_id = int(_float((lightning_index + generation) % 10) * 102.0 + 102.0 * life_time);
	return noise_map.read(uint2(slot_id, 3)).x + 0.3;
}

_float3 find_endpoint(int generation, _float3 begin_point, _float interpolator, constant lightning_pass1_uniforms* lu,
					  texture2d<_float> noise_map, device _float4* endpoints)
{	
	// The begin point can be on the left or the right side of the actor
    // Small offset is added to avoid precision issues    
    _float begin_point_sign = sign(dot(lu->actor_right, begin_point - lu->actor_pos) - 0.001);
    
	// If the begin point is higher then the actor.y * 1.5
	bool high_point = begin_point.y > lu->actor_pos.y * 1.5;
	
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
    if (lu->time > 16500.0 && lu->time < 23000.0 && generation % 5 == 1)
    {
        start_index = int(rand2_unsigned(generation, 4, noise_map).x * 80.0);
        end_index = 81;
    }
			
	int end_point_index = start_index;
	_float min_dist = 999.0;	
	for(int i = start_index; i < end_index - 1; i++)
	{
		_float3 test_point = get_endpoint(i, endpoints);
		_float end_point_sign = sign(dot(lu->actor_right, test_point - lu->actor_pos));		
		if (begin_point_sign != 0.0 && begin_point_sign != end_point_sign)
		{
			// The end point is on the other side of the actor
			continue;
		}
        
        // Randomly skip some endpoints
        if (rand2_unsigned(i, generation, noise_map).x > 0.5)
        {
            continue;
        }
        
		// Choose the closest point
		_float test_dist = distance(begin_point, test_point);
		if(min_dist > test_dist)
		{
			end_point_index = i;
			min_dist = test_dist;
		} 
	}		
	
	// Interpolate between the two endpoints         
	_float3 end_point_1 = get_endpoint(end_point_index + 0, endpoints);
	_float3 end_point_2 = get_endpoint(end_point_index + 1, endpoints);        
	return mix(end_point_1, end_point_2, interpolator);
}


kernel void shader_main(constant lightning_pass1_uniforms* lu               [[ buffer(LP1_UNIFORMS_BFR_SLOT)   ]],
				                 texture2d<_float>         noise_map        [[ texture(LP1_NOISE_MAP_TEX_SLOT) ]],
				        device   _float4*                  endpoints        [[ buffer(LP1_ENDPOINTS_BFR_SLOT)  ]],
				        device   lightning_buffer_layout*  lightning_buffer [[ buffer(LP1_LIGHTNING_BFR_SLOT)  ]],
				        device   LightInstancingStruct*    lights_buffer    [[ buffer(LP1_LIGHTS_BFR_SLOT)     ]],
				        		 uint                      lightning_index  [[ threadgroup_position_in_grid    ]],
								 uint                      local_id         [[ thread_position_in_threadgroup  ]],
								 uint                      global_id        [[ thread_position_in_grid         ]])
{	
	threadgroup int generation;
	threadgroup _float life_time;
	threadgroup int iterations;
	
	_float3 begin_point;
	_float3 end_point;
    
    // Only used by the first thread in the group
    _float intensity = 0.0;
		
	// First determinate the begin and end points of the lightning
	if (local_id == 0)
	{		
		_float max_age = mix(MIN_AGE, MAX_AGE, _float(lightning_index) / _float(LIGHTNING_COUNT));
		generation = int(floor(lu->time / max_age));
		_float age = lu->time - _float(generation) * max_age;
				
		// life_time variable goes from 1.0 to 0.0		
		life_time = (max_age - age) / max_age;
		
		// Get a bone for the lightning and calculate the begin point
		int bone_index = get_bone_id(lightning_index, generation, lu, noise_map);
		_float bone_interpolator = life_time * BONE_INTERPOLATOR_SPEED;
		
        // Add lighting to the gun of the robot         
		if (lightning_index == 2 && lu->time > 55000.0)		
		{
			bone_index = 10;
            bone_interpolator = life_time;
            if (generation % 2 == 1)
            {
                bone_interpolator = 1.0 - bone_interpolator;
            }           
		}		
		
		_float3 bone_pos0 = lu->bone_segments[bone_index * 2 + 0].xyz;
		_float3 bone_pos1 = lu->bone_segments[bone_index * 2 + 1].xyz;			
		
		bone_interpolator = clamp(bone_interpolator, 0.0, 1.0);
		begin_point = mix(bone_pos0, bone_pos1, bone_interpolator);
		
		// Get the end point for the lightning
		_float end_point_interpolator = 0.0;
		if (life_time < 0.2)
		{
			end_point_interpolator = 0.0;
		}
		
		end_point = find_endpoint(generation, begin_point, end_point_interpolator, lu, noise_map, endpoints);
		//end_point = find_endpoint(generation, bone_pos0, end_point_interpolator, lu, noise_map, endpoints);
        
		// Get the intensity 
		intensity = get_intensity(lightning_index, generation, 1.0 - life_time, noise_map);				
		// Add the lightning to the pole 				
		if (lightning_index == 0 && lu->time < 10000.0)		
		{
			_float interpolator = 1.0 - _float(int(lu->time) % 1200) / 1000.0;    
			interpolator = clamp(interpolator, 0.0, 1.0);	
			begin_point =  get_endpoint(3, endpoints);
			end_point = mix(get_endpoint(0, endpoints), get_endpoint(1, endpoints), interpolator);
		}
	
		store_vertex(lightning_index, 0, _float4(begin_point, pack_w(intensity, 1.0)),lightning_buffer);
		store_vertex(lightning_index, 1, _float4(end_point, pack_w(intensity, 0.0)),lightning_buffer);	
        
        // Adaptive division: lightning length is between about 8 and 77
        // Iterations should be between 4 and 9
        _float lightning_length = distance(begin_point, end_point);
        iterations = (int(lightning_length) - 10) / 4 + 4;   
        iterations = clamp(iterations, 4, 9);
        //iterations = 0;
	}		
	
	// Wait for the first thread (Order shared and buffer memory transactions)
	threadgroup_barrier(mem_flags::mem_device_and_threadgroup) ;

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
            _float4 a = load_vertex(lightning_index, idx * 2 + 0,lightning_buffer);
        	_float4 b = load_vertex(lightning_index, idx * 2 + 1,lightning_buffer);
            
            _float dist = length(a.xyz - b.xyz);                                        
            int rnd_seed = num_segments * 2 + idx+  generation;
            _float3 random = rand2_signed(rnd_seed, lightning_index,noise_map);
            
            // Create a new branche          
            //if (passIt > 0 && random.x > -1.5 && dist > MIN_SEGMENT_LENGTH)
            if (i < iterations - 3 && idx > 0 && random.x > 0.3 && dist > MIN_SEGMENT_LENGTH )
            {            
                // Unpack the intensity and the distance from the endpoint			
                _float2 packed_a = unpack_w(a.w);
                _float2 packed_b = unpack_w(b.w);
                _float intensity_a = packed_a.x;
                _float intensity_b = packed_b.x;
                _float end_point_distance_a = packed_a.y;									
                _float end_point_distance_b = packed_b.y;
            
                // The length factor of the branche
                _float branche_delta = dist * 0.204;
            
                // Make the branche less intensive
                _float mid_intensity = intensity_a * 0.7;         
             
                // Get the endpoint of the branche by translate the 'a' point to the direction of the midpoint 
                _float3 midpoint_dir = b.xyz - a.xyz;
                
                int rnd_idx1 = num_segments + lightning_index + generation;
                _float3 offset = rand2_signed(int(_float(rnd_idx1 + 37) * lu->time * 0.00006), lightning_index, noise_map) * branche_delta;		
                                  
                // Average the segment distance
                _float new_endpoint_distance = (end_point_distance_a + end_point_distance_b) * 0.5;               
                
                offset = rand2_signed(0, lightning_index, noise_map) * branche_delta;
                _float3 new_endpoint = a.xyz + midpoint_dir * 0.9 + offset;
                
                // Store the new branche                
                store_vertex(lightning_index, new_pos * 2 + 0, _float4(a.xyz, pack_w(mid_intensity, new_endpoint_distance)),lightning_buffer);
                store_vertex(lightning_index, new_pos * 2 + 1, _float4(new_endpoint, pack_w(mid_intensity, new_endpoint_distance)),lightning_buffer);	
            }
            // Split the segment
            else if (dist > MIN_SEGMENT_LENGTH)
            {		
                 // The center of the segment
                _float3 mid = (a.xyz + b.xyz) * 0.5;
            
                // Unpack the intensity and the distance from the endpoint			
                _float2 packed_a = unpack_w(a.w);
                _float2 packed_b = unpack_w(b.w);
                _float intensity_a = packed_a.x;
                _float intensity_b = packed_b.x;
                _float end_point_distance_a = packed_a.y;									
                _float end_point_distance_b = packed_b.y;
            
                // The direction of the segment	
                _float3 dir = normalize(b.xyz - a.xyz);
                
                // The length factor of the midpoint
                _float mid_delta = dist * 0.17; //0.17
                
                _float interpolator = life_time * 2.0;
                interpolator = clamp(life_time, 0.0, 0.6);

                int seed = num_segments + local_id + generation;                
                _float3 random1 = rand2_signed(seed, lightning_index, noise_map);
                _float3 random2 = rand2_signed(seed + 10, lightning_index, noise_map);                
                _float3 rnd_vector = mix(random1, random2, interpolator) * mid_delta;
                _float d = dot(dir, rnd_vector);
                rnd_vector = rnd_vector - dir * d;

                // Modify the midpoint
                mid += rnd_vector;			
                    
                // Average the segment distance
                _float new_endpoint_distance = (end_point_distance_a + end_point_distance_b) / 2.0;
                                
                // Replace and add the new segments                                
                //store_vertex(lightning_index, idx * 2 + 0, a);
                store_vertex(lightning_index, idx * 2 + 1, _float4(mid, pack_w(intensity_a, new_endpoint_distance)), lightning_buffer);	
                    
                store_vertex(lightning_index, new_pos * 2 + 0, _float4(mid, pack_w(intensity_b, new_endpoint_distance)), lightning_buffer);
                store_vertex(lightning_index, new_pos * 2 + 1, b,lightning_buffer);	      
            }      
            // Just copy the segment    
            else
            {
                store_vertex(lightning_index, new_pos * 2 + 0, a, lightning_buffer);
                store_vertex(lightning_index, new_pos * 2 + 1, b, lightning_buffer);
            }
        }

        // After the barrier all the threads have created one more segment.        
        num_segments = 2 * num_segments;
        
		threadgroup_barrier(mem_flags::mem_device_and_threadgroup) ;	
    }
    
	if (local_id == 0)
	{
		// Write out the number of vertices
		lightning_buffer->vertex_counts[lightning_index] = _float(num_segments * 2);
		
		// Add lights to the first and the last segments
		_float3 first_segment = load_vertex(lightning_index, 1, lightning_buffer).xyz;
		_float3 light_dir = normalize(first_segment - begin_point);			
		
		store_light(lightning_index * 2, begin_point + light_dir * 2.0, intensity, 10.0, lights_buffer);		
		store_light(lightning_index * 2 + 1, end_point, intensity, 10.0, lights_buffer);
	}
}

#endif

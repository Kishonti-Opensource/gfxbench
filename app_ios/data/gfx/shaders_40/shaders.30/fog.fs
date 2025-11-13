/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//#define per_pixel_random_offset

#ifdef USE_UBOs
	#include cameraConsts;
	#include lightShaftConsts;
#else
	uniform vec3 background_color;
	uniform float time;
	uniform vec3 light_color;
	uniform vec3 light_pos;
	uniform float attenuation_parameter;
	uniform vec4 light_x;
	uniform vec4 light_y;
	uniform vec4 light_z;
	uniform vec2 spot_cos;
	uniform mat4 shadow_matrix0;

	uniform vec4 depth_parameters;

	uniform vec3 view_pos; // camera view position
	uniform vec3 view_dir; // camera view direction (normalized vector, looking opposite direction from eye)
	uniform mat4 mv;
#endif

uniform sampler2D texture_unit0;
uniform highp sampler2D texture_unit1; //depth

in vec4 out_pos;
in vec4 out_pos_hom; //homogeneous vertpos
in vec4 shadow_texcoord;

out vec4 frag_color;

#ifdef per_pixel_random_offset
const float offsets[16] = {
		0.52, 0.37, 0.28, 0.68, 
		0.19, 0.03, 0.33, 0.17,
		0.38, 0.54, 0.83, 0.90, 
		0.78, 0.31, 0.18, 0.47 
};
#endif

void main()
{   
//frag_color = vec4(0.0, 1.0, 0.0, 0.9);
//return;

	vec2 depth_texcoord = (out_pos_hom.xy / out_pos_hom.w * 0.5) + vec2(0.5,0.5);

	highp float d = texture( texture_unit1, depth_texcoord).x;
	float z_view_depth = depth_parameters.y / (d - depth_parameters.x);

#ifdef USE_UBOs
	float t = view_posXYZ_normalized_time.w * 8.0;
#else
	float t = time * 8.0;
#endif
	float final_atten = 0.0;

	float v = 0.0;
	float v2 = 0.0;

	float prev_fov_cos = -1000.0; // cos(tracing point cone angle) of previous step;
	float prev_atten_distance = -1000.0;  // previous axial cone attenuation value

	vec3 world_pos = out_pos.xyz; // world position for following texture fetch from shadow_texture
	
#ifdef USE_UBOs
	vec3 light_dir = out_pos.xyz - light_pos_pad.xyz;
	vec3 ray_dir   = out_pos.xyz - view_posXYZ_normalized_time.xyz;
#else
	vec3 light_dir = out_pos.xyz - light_pos;
	vec3 ray_dir   = out_pos.xyz - view_pos;
#endif	
	

	vec3 ray_dir_norm = normalize(ray_dir);
	vec3 ray_dir_step = ray_dir_norm; // scale up if needed
	
#ifdef USE_UBOs
	float start_depth = dot(ray_dir, view_dirXYZ_pad.xyz);
#else
	float start_depth = dot(ray_dir, view_dir);
#endif

	float offset = 0.0;
#ifdef per_pixel_random_offset
	// add random offset of up 1.0 length to starting vector position in 4x4 grid.
	ivec2 off = ivec2(fract(vec2( gl_FragCoord.xy * 0.25 ))*4.0);
	//offset = offsets[off.x << 2 + off.y];
	offset = float(1.0*offsets[off.x << 2 + off.y]); 
			// * fract(z_view_depth); // adds even more randomness per-pixel
	light_dir -= offset * ray_dir_step;
	world_pos -= offset * ray_dir_step;
#endif

	float max_iter = (z_view_depth - start_depth);
	if (max_iter > 1024.0)
	{
		max_iter = 1024.0;
	}

	float i;

#ifdef USE_UBOs
	vec2 spot_cos = spot_cos_attenuation_parameter_pad.xy;
#endif
	
	for (i=0.0; i<max_iter; i+=1.0) // trace only distance to opaque object
	{
		float sq_distance = dot(light_dir, light_dir);
		//vec3 light_dir_norm = light_dir * inversesqrt(sq_distance);
		vec3 light_dir_norm = light_dir / sqrt(sq_distance);

#ifdef USE_UBOs
	float atten = spot_cos_attenuation_parameter_pad.z * sq_distance + 1.0;
#else
	float atten = attenuation_parameter * sq_distance + 1.0;
#endif
		
		// check for exit due distance (exit via bottom of cone)
		if ((atten < 0.0) && (prev_atten_distance > atten)) {
			v2 = 1.0;
			break;
		}
		prev_atten_distance = atten; // if we're going inside cone - previous distance will be smaller vs attenuated distance (it's going from 0.0 to 1.0 near light source)
		// if ((atten > 1.1) break;

		atten = clamp( atten, 0.0, 1.0); //todo: saturate?
				
		float fov_atten = dot(light_dir_norm, light_x.xyz);

		fov_atten = fov_atten - spot_cos.x;

		// check for exit due angle (exit via side of cone)
		if ((fov_atten < 0.0) && (prev_fov_cos > fov_atten)) {
			v = 1.0;
			break;
	}
	prev_fov_cos = fov_atten; // if we're going inside cone - previous cos() will be smaller vs new cos() value of angle
	
	fov_atten *= spot_cos.y;

	atten *= fov_atten;

	if (atten > 1.0/255.0) // if we're above threshold - modulate by noise texture
	{
		vec4 shadow_texcoord_f = shadow_matrix0 * vec4( world_pos, 1.0);

		vec3 out_texcoord0 = shadow_texcoord_f.xyz / shadow_texcoord_f.w;

		float shadow_depth = texture( texture_unit0, out_texcoord0.xy + vec2( t, 0.0)).x;
		atten *= shadow_depth;
	}

	final_atten += atten * (1.0 - final_atten);

	// advance our positions by raymarching step
	light_dir += ray_dir_step; 
	world_pos += ray_dir_step;
}
		
#ifdef USE_UBOs
	frag_color = vec4( light_color_pad.xyz, final_atten) * 0.5;
#else
	frag_color = vec4( light_color.xyz, final_atten) * 0.5;
#endif	

	//frag_color.yw = vec2(i/60.0, 1.0); //debug - visualize number of steps
	//if (v > 0) frag_color.xw = vec2(v); //debug - color red pixels exited via cone sides
	//if (v2 > 0) frag_color.zw = vec2(v2); //debug - color blue pixels exited via cone bottom
	//frag_color = vec4( final_atten, 0, 0, 1); // visualize just attenuation

	//frag_color = vec4(1.0);

	// depth visualization checks
	//frag_color = vec4(z_view_depth/200, start_depth, 0, 1.0);
	//frag_color = vec4(0, (z_view_depth - start_depth)/200, 0, 1.0);

#ifdef SV_31
	frag_color.xyz *= 0.25; //target is GL_RGB10_A2, which has 4x precision compared to GL_RGBA8, so convert it to range, compress it
	frag_color = max(frag_color, vec4(0.0));
#endif		
}

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//TODO: szetbontani 2 pass-ra, 1 init feedback shader, 1 advect feedback shader

layout (location = 0) in vec3 in_Pos; // particle instance's position
layout (location = 1) in vec3 in_Age01_Speed_Accel;

layout (location = 2) in vec3 in_Amplitude;
layout (location = 3) in vec3 in_Phase;
layout (location = 4) in vec3 in_Frequency;
layout (location = 5) in vec3 in_T;
layout (location = 6) in vec3 in_B;
layout (location = 7) in vec3 in_N;
layout (location = 8) in vec4 in_Velocity;

#ifdef USE_UBOs
	#include emitterAdvectConsts;
#else
	uniform vec4 emitter_apertureXYZ_focusdist;
	uniform mat4 emitter_worldmat;
	uniform vec4 emitter_min_freqXYZ_speed;
	uniform vec4 emitter_max_freqXYZ_speed;
	uniform vec4 emitter_min_ampXYZ_accel;
	uniform vec4 emitter_max_ampXYZ_accel;
	uniform vec4 emitter_externalVel_gravityFactor;
	uniform vec4 emitter_maxlifeX_sizeYZ_pad;
	uniform int emitter_numSubsteps;
#endif

#define MAX_SUBSTEPS 5 //MUST match with kcl_particlesystem2.h/KCL::_emitter::max_num_substeps !!!!
uniform ivec4 particleBufferParamsXYZ_pad[ MAX_SUBSTEPS ]; //startBirthIdx, endBirthIdx, noOverflow

uniform sampler2D texture_unit0; //noise

//NOTE: cannot overwrite uniform values in shader
//NOTE: have to fill ALL components of ALL output values
//NOTE: UBOs cannot contain anything other than vec4-s

out vec3 out_Pos;
out vec3 out_Age01_Speed_Accel;
out vec3 out_Amplitude;
out vec3 out_Phase;
out vec3 out_Frequency;
out vec3 out_T;
out vec3 out_B;
out vec3 out_N;
out vec4 out_Velocity;


void main()
{
	const float deltaTime = 0.025;
	int VertexID = int(in_Velocity.w);

	vec3 temp_Pos = in_Pos;
	vec3 temp_Age01_Speed_Accel = in_Age01_Speed_Accel;
	vec3 temp_Amplitude = in_Amplitude;
	vec3 temp_Phase = in_Phase;
	vec3 temp_Frequency = in_Frequency;
	vec3 temp_T = in_T;
	vec3 temp_B = in_B;
	vec3 temp_N = in_N;
	vec4 temp_Velocity = in_Velocity;
	
#ifdef USE_UBOs
	int emitter_numSubsteps = int(emitter_maxlifeX_sizeYZ_numSubsteps.w);
	vec4 emitter_maxlifeX_sizeYZ_pad = emitter_maxlifeX_sizeYZ_numSubsteps;
#endif

	for( int i = 0; i < emitter_numSubsteps; ++i ) 
	{

		int startBirthIdx = particleBufferParamsXYZ_pad[ i ].x;
		int endBirthIdx = particleBufferParamsXYZ_pad[ i ].y;
		int noOverflowI = particleBufferParamsXYZ_pad[ i ].z;

		//decide birth
		bool overflow = ( noOverflowI == 0 );
			
		bool b0 = ( startBirthIdx <= VertexID );
		bool b1 = ( VertexID < endBirthIdx );
		
		bool init = bool( (!overflow && b0 && b1) || (overflow && (b0 || b1)) );

		//init or advect
		if(init)
		{
			//get "rnd" init data based on particle index
			vec2 uv = vec2( (VertexID / 16) % 16, VertexID % 16); //16x16 noise map
			vec3 randomValue_0_1_3D = textureLod(texture_unit0, uv / 16.0, 0.0).xyz;	
			vec3 randomValue_n1_p1_3D = randomValue_0_1_3D * 2.0 - 1.0;
	
			//lokalis pozicio offset az aperturaban
			vec3 pos_offset = emitter_apertureXYZ_focusdist.xyz * randomValue_n1_p1_3D.xyz;
		
			//globalis fokusz offszet
			vec3 focus_offset = emitter_worldmat[1].xyz * emitter_apertureXYZ_focusdist.w;
		
			//globalis pozicio offszet
			vec3 delta;
			delta.x = pos_offset.x * emitter_worldmat[0][0] + pos_offset.y * emitter_worldmat[1][0] +  pos_offset.z * emitter_worldmat[2][0];
			delta.y = pos_offset.x * emitter_worldmat[0][1] + pos_offset.y * emitter_worldmat[1][1] +  pos_offset.z * emitter_worldmat[2][1];
			delta.z = pos_offset.x * emitter_worldmat[0][2] + pos_offset.y * emitter_worldmat[1][2] +  pos_offset.z * emitter_worldmat[2][2];
		
			//globalis pozicio
			temp_Pos = delta + emitter_worldmat[3].xyz;
		
			//globalis elore vektor
			vec3 n;
			if( emitter_apertureXYZ_focusdist.w > 0.0)
			{
				n = focus_offset - delta;
			}
			else
			{
				n = delta - focus_offset;
			}
			temp_N = normalize(n);
			
			vec3 b = cross( n, vec3( emitter_worldmat[0][0], emitter_worldmat[0][1], emitter_worldmat[0][2]));
			temp_B = normalize(b);
			
			vec3 t = cross( b, n);
			temp_T = normalize(t);
			
			temp_Age01_Speed_Accel.x = 0.0;
			temp_Age01_Speed_Accel.y = mix( emitter_min_freqXYZ_speed.w, emitter_max_freqXYZ_speed.w, randomValue_0_1_3D.x);
			temp_Age01_Speed_Accel.z = mix( emitter_min_ampXYZ_accel.w, emitter_max_ampXYZ_accel.w, randomValue_0_1_3D.y);
			
			temp_Phase = randomValue_n1_p1_3D.xyz;
			
			temp_Frequency = mix( emitter_min_freqXYZ_speed.xyz, emitter_max_freqXYZ_speed.xyz, randomValue_0_1_3D.xyz);
			temp_Amplitude = mix( emitter_min_ampXYZ_accel.xyz, emitter_max_ampXYZ_accel.xyz, randomValue_0_1_3D.xyz);
			
			temp_Velocity = vec4(0.0, 0.0, 0.0, in_Velocity.w);
		}
		else //advect in not expired
		{
			if(temp_Age01_Speed_Accel.x < 1.0)
			{
				float realAge = temp_Age01_Speed_Accel.x * emitter_maxlifeX_sizeYZ_pad.x;
			
				float t_turb = sin(realAge * temp_Frequency.x + temp_Phase.x) * temp_Amplitude.x;
				float b_turb = sin(realAge * temp_Frequency.y + temp_Phase.y) * temp_Amplitude.y;
				float n_turb = sin(realAge * temp_Frequency.z + temp_Phase.z) * temp_Amplitude.z;

				vec3 velocity = temp_N * (temp_Age01_Speed_Accel.y + realAge * temp_Age01_Speed_Accel.z);
				
				//add turbulence
				velocity = temp_T * t_turb + temp_B * b_turb + temp_N * n_turb;
				
				//add ext velocity
				velocity += emitter_externalVel_gravityFactor.xyz;

				//add gravity
				velocity.y -= temp_Age01_Speed_Accel.x * emitter_externalVel_gravityFactor.w;

				temp_Pos = temp_Pos + velocity;
				temp_Velocity = vec4(normalize(velocity), temp_Velocity.w);
				
				//age the particle towards 1, the more life it has the slower to age it
				temp_Age01_Speed_Accel = temp_Age01_Speed_Accel;
				temp_Age01_Speed_Accel.x += deltaTime / emitter_maxlifeX_sizeYZ_pad.x;
			}
			else //place it behind the far plane to cull from rendering in main VS
			{			
				temp_Pos = vec3(10000.0,10000.0,10000.0);
				temp_Age01_Speed_Accel = in_Age01_Speed_Accel;
				temp_Age01_Speed_Accel.x = 2.0; //invalid normalized age
			}
		}
	}

	out_Pos = temp_Pos;
	out_Age01_Speed_Accel = temp_Age01_Speed_Accel;
	out_Amplitude = temp_Amplitude;
	out_Phase = temp_Phase;
	out_Frequency = temp_Frequency;
	out_T = temp_T;
	out_B = temp_B;
	out_N = temp_N;
	out_Velocity = temp_Velocity;

	gl_Position = vec4(in_Pos, 1.0);
}

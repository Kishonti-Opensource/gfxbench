/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//TODO: szetbontani 2 pass-ra, 1 init feedback shader, 1 advect feedback shader

#include "../include/types.hlsl"
#include "../include/functions.hlsl"
#include "../include/cbuffer_emitter.hlsli"

ParticleAnimationData main(ParticleAnimationData input, uint vertexId : SV_VertexID)
{
	ParticleAnimationData output;
	const float deltaTime = 0.025;

	//decide birth
	bool overflow = emitter_startBirthIdx > emitter_endBirthIdx;
	bool idxSmallEnough = vertexId < emitter_endBirthIdx;
	bool idxBigEnough = vertexId >= emitter_startBirthIdx;
	bool init = (!overflow && idxSmallEnough && idxBigEnough) ||
				(overflow && (idxSmallEnough || idxBigEnough));

	//init or advect
	if (init)
	{
		//get "rnd" init data based on particle index
		////float2 uv = float2( (vertexId / 16) % 16, vertexId % 16); //16x16 noise map
		float3 randomValue_0_1_3D = randVec3(vertexId); // TODO: texture0.SampleLevel(SampleType, uv / 16.0, 0.0).xyz;
		float3 randomValue_n1_p1_3D = randomValue_0_1_3D * 2.0 - 1.0;
	
		//lokalis pozicio offset az aperturaban
		float3 pos_offset = emitter_aperture * randomValue_n1_p1_3D.xyz;

		//globalis fokusz offszet
		float3 focus_offset = emitter_worldmat[1].xyz * emitter_focusdist;

		//globalis pozicio offszet
		float3 delta;
		delta.x = pos_offset.x * emitter_worldmat[0][0] + pos_offset.y * emitter_worldmat[1][0] +  pos_offset.z * emitter_worldmat[2][0];
		delta.y = pos_offset.x * emitter_worldmat[0][1] + pos_offset.y * emitter_worldmat[1][1] +  pos_offset.z * emitter_worldmat[2][1];
		delta.z = pos_offset.x * emitter_worldmat[0][2] + pos_offset.y * emitter_worldmat[1][2] +  pos_offset.z * emitter_worldmat[2][2];

		//globalis pozicio
		//output.Pos = delta + emitter_worldmat[3].xyz;
		output.Pos = mul(float4(pos_offset, 1.0), emitter_worldmat).xyz;

		//globalis elore vektor
		float3 n;
		if (emitter_focusdist > 0.0)
		{
			n = focus_offset - delta;
		}
		else
		{
			n = delta - focus_offset;
		}
		output.N = normalize(n);

		float3 b = cross( n, emitter_worldmat[0].xyz);
		output.B = normalize(b);

		float3 t = cross( b, n);
		output.T = normalize(t);

		output.Age01_Speed_Accel.x = 0.0;
		output.Age01_Speed_Accel.y = lerp( emitter_min_speed, emitter_max_speed, randomValue_0_1_3D.x);
		output.Age01_Speed_Accel.z = lerp( emitter_min_accel, emitter_max_accel, randomValue_0_1_3D.y);
		output.Phase = randomValue_n1_p1_3D;

		output.Frequency = lerp( emitter_min_freq, emitter_max_freq, randomValue_0_1_3D);
		output.Amplitude = lerp( emitter_min_amplitude, emitter_max_amplitude, randomValue_0_1_3D);
	
		output.Velocity = float3(0.0, 0.0, 0.0);
	}
	else if (input.Age01_Speed_Accel.x <= 1.0)	//advect if not expired
	{
		float realAge = input.Age01_Speed_Accel.x * emitter_maxlife;
		
		float t_turb = sin(realAge * input.Frequency.x + input.Phase.x) * input.Amplitude.x;
		float b_turb = sin(realAge * input.Frequency.y + input.Phase.y) * input.Amplitude.y;
		float n_turb = sin(realAge * input.Frequency.z + input.Phase.z) * input.Amplitude.z;

		float3 velocity = input.N * (input.Age01_Speed_Accel.y + realAge * input.Age01_Speed_Accel.z);
			
		//add turbulence
		velocity = input.T * t_turb + input.B * b_turb + input.N * n_turb;
			
		//add ext velocity
		velocity += emitter_externalVelocity;

		//add gravity
		velocity.y -= input.Age01_Speed_Accel.x * emitter_gravityFactor;

		output.Pos = input.Pos + velocity;
		output.Velocity = normalize(velocity);
			
		//age the particle towards 1, the more life it has the slower to age it
		output.Age01_Speed_Accel = input.Age01_Speed_Accel;
		output.Age01_Speed_Accel.x += deltaTime / emitter_maxlife;

		output.Amplitude = input.Amplitude;
		output.Phase = input.Phase;
		output.Frequency = input.Frequency;
		output.T = input.T;
		output.B = input.B;
		output.N = input.N;
	}
	else //place it behind the far plane to cull from rendering in main VS
	{			
		output.Pos = float3(10000.0f,10000.0f,10000.0f);
		output.Age01_Speed_Accel = input.Age01_Speed_Accel;
		output.Age01_Speed_Accel.x = 2.0f; //invalid normalized age

		output.Amplitude = input.Amplitude;
		output.Phase = input.Phase;
		output.Frequency = input.Frequency;
		output.T = input.T;
		output.B = input.B;
		output.N = input.N;
		output.Velocity = input.Velocity;
	}

	return output;
}

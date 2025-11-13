/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//TODO: szetbontani 2 pass-ra, 1 init feedback shader, 1 advect feedback shader

#ifdef IOS
vertex void shader_main(constant ParticleVertexInput* input [[buffer(0)]],
#else
vertex void shader_main(device   ParticleVertexInput* input [[buffer(0)]],
#endif
								device ParticleVertexInput* output [[buffer(2)]],
								constant EmitterAdvectConsts* advectConsts [[buffer(14)]],
								texture2d<_float> noiseTexture [[texture(0)]],
								sampler sam [[sampler(0)]],
								uint vid [[vertex_id]])
{
	
	const _float deltaTime = 0.025;
	//TODO is this necessary? can't just use vid/iid?
	int VertexID = int(input[vid].in_Velocity.w);
	
	_float3 temp_Pos = input[vid].in_Pos.xyz;
	_float3 temp_Age01_Speed_Accel = input[vid].in_Age01_Speed_Accel.xyz;
	_float3 temp_Amplitude = input[vid].in_Amplitude.xyz;
	_float3 temp_Phase = input[vid].in_Phase.xyz;
	_float3 temp_Frequency = input[vid].in_Frequency.xyz;
	_float3 temp_T = input[vid].in_T.xyz;
	_float3 temp_B = input[vid].in_B.xyz;
	_float3 temp_N = input[vid].in_N.xyz;
	_float4 temp_Velocity = input[vid].in_Velocity;
	
	for( int i = 0; i < advectConsts[0].emitter_numSubstepsX_pad_pad_pad.x; ++i )
	{
		int startBirthIdx = advectConsts[0].particleBufferParamsXYZ_pad[ i ].x;
		int endBirthIdx = advectConsts[0].particleBufferParamsXYZ_pad[ i ].y;
		int noOverflowI = advectConsts[0].particleBufferParamsXYZ_pad[ i ].z;
		
		//decide birth
		bool overflow = ( noOverflowI == 0 );
		
		bool b0 = ( startBirthIdx <= VertexID );
		bool b1 = ( VertexID < endBirthIdx );
		
		bool init = bool( (!overflow && b0 && b1) || (overflow && (b0 || b1)) );
		
		//init or advect
		if(init)
		{
			//get "rnd" init data based on particle index
			_float2 uv = _float2( (VertexID / 16) % 16, VertexID % 16); //16x16 noise map
			_float3 randomValue_0_1_3D = noiseTexture.sample(sam, uv / 16.0f, level(0)).xyz;
			_float3 randomValue_n1_p1_3D = randomValue_0_1_3D * _float(2.0) - _float(1.0);
			
			//lokalis pozicio offset az aperturaban
			_float3 pos_offset = advectConsts[0].emitter_apertureXYZ_focusdist.xyz * randomValue_n1_p1_3D.xyz;
			
			//globalis fokusz offszet
			_float3 focus_offset = advectConsts[0].emitter_worldmat[1].xyz * advectConsts[0].emitter_apertureXYZ_focusdist.w;
			
			//globalis pozicio offszet
			_float3 delta;
			//NOTE: modified from emitter_worldmat[0][0] to emitter_worldmat[0].x, etc
			delta.x = pos_offset.x * advectConsts[0].emitter_worldmat[0].x + pos_offset.y * advectConsts[0].emitter_worldmat[1].x +  pos_offset.z * advectConsts[0].emitter_worldmat[2].x;
			delta.y = pos_offset.x * advectConsts[0].emitter_worldmat[0].y + pos_offset.y * advectConsts[0].emitter_worldmat[1].y +  pos_offset.z * advectConsts[0].emitter_worldmat[2].y;
			delta.z = pos_offset.x * advectConsts[0].emitter_worldmat[0].z + pos_offset.y * advectConsts[0].emitter_worldmat[1].z +  pos_offset.z * advectConsts[0].emitter_worldmat[2].z;
			
			//globalis pozicio
			temp_Pos = delta + _float3(advectConsts[0].emitter_worldmat[3].xyz);
			
			//globalis elore vektor
			_float3 n;
			if( _float(advectConsts[0].emitter_apertureXYZ_focusdist.w) > _float(0.0) )
			{
				n = focus_offset - delta;
			}
			else
			{
				n = delta - focus_offset;
			}
			temp_N = normalize(n);
			
			_float3 b = cross( n, advectConsts[0].emitter_worldmat[0].xyz);
			temp_B = normalize(b);
			
			_float3 t = cross( b, n);
			temp_T = normalize(t);
			
			temp_Age01_Speed_Accel.x = 0.0;
			temp_Age01_Speed_Accel.y = mix( advectConsts[0].emitter_min_freqXYZ_speed.w, advectConsts[0].emitter_max_freqXYZ_speed.w, randomValue_0_1_3D.x);
			temp_Age01_Speed_Accel.z = mix( advectConsts[0].emitter_min_ampXYZ_accel.w, advectConsts[0].emitter_max_ampXYZ_accel.w, randomValue_0_1_3D.y);
			
			temp_Phase = randomValue_n1_p1_3D.xyz;
			
			temp_Frequency = mix( advectConsts[0].emitter_min_freqXYZ_speed.xyz, advectConsts[0].emitter_max_freqXYZ_speed.xyz, randomValue_0_1_3D.xyz);
			temp_Amplitude = mix( advectConsts[0].emitter_min_ampXYZ_accel.xyz, advectConsts[0].emitter_max_ampXYZ_accel.xyz, randomValue_0_1_3D.xyz);
			
			temp_Velocity = _float4(0.0, 0.0, 0.0, input[vid].in_Velocity.w);
		}
		else //advect in not expired
		{
			if( _float(temp_Age01_Speed_Accel.x) < _float(1.0) )
			{
				_float realAge = temp_Age01_Speed_Accel.x * advectConsts[0].emitter_maxlifeX_sizeYZ_pad.x;
				
				_float t_turb = sin(realAge * temp_Frequency.x + temp_Phase.x) * temp_Amplitude.x;
				_float b_turb = sin(realAge * temp_Frequency.y + temp_Phase.y) * temp_Amplitude.y;
				_float n_turb = sin(realAge * temp_Frequency.z + temp_Phase.z) * temp_Amplitude.z;
				
				_float3 velocity = temp_N * (temp_Age01_Speed_Accel.y + realAge * temp_Age01_Speed_Accel.z);
				
				//add turbulence
				velocity = temp_T * t_turb + temp_B * b_turb + temp_N * n_turb;
				
				//add ext velocity
				velocity += advectConsts[0].emitter_externalVel_gravityFactor.xyz;
				
				//add gravity
				velocity.y -= temp_Age01_Speed_Accel.x * advectConsts[0].emitter_externalVel_gravityFactor.w;
				
				temp_Pos = temp_Pos + velocity;

                _float3 normalizedVelocity = normalize(velocity);
				temp_Velocity = _float4(normalizedVelocity, temp_Velocity.w);
				
				//age the particle towards 1, the more life it has the slower to age it
				temp_Age01_Speed_Accel = temp_Age01_Speed_Accel;
				temp_Age01_Speed_Accel.x += deltaTime / advectConsts[0].emitter_maxlifeX_sizeYZ_pad.x;
			}
			else //place it behind the far plane to cull from rendering in main VS
			{
				temp_Pos = _float3(10000.0f,10000.0f,10000.0f);
				temp_Age01_Speed_Accel = input[vid].in_Age01_Speed_Accel.xyz;
				temp_Age01_Speed_Accel.x = 2.0f; //invalid normalized age
			}
		}
	}
	
	output[vid].in_Pos.xyz = temp_Pos;
	output[vid].in_Age01_Speed_Accel.xyz = temp_Age01_Speed_Accel;
	output[vid].in_Amplitude.xyz = temp_Amplitude;
	output[vid].in_Phase.xyz = temp_Phase;
	output[vid].in_Frequency.xyz = temp_Frequency;
	output[vid].in_T.xyz = temp_T;
	output[vid].in_B.xyz = temp_B;
	output[vid].in_N.xyz = temp_N;
	output[vid].in_Velocity = temp_Velocity;

}


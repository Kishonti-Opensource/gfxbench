/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
//http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
//https://github.com/bartwronski/CSharpRenderer/blob/master/shaders/optimized-ggx.hlsl

out float4 out_res { color(0) };

#define PI 3.14152
#define saturate(X) clamp(X, 0.0, 1.0)

uint ReverseBits32( uint bits )
{
	bits = ( bits << uint(16) ) | ( bits >> uint(16) );
	//bits = ( (bits & uint(0x00ff00ff)) << uint(8) ) | ( (bits & uint(0xff00ff00)) >> uint(8) );
	//bits = ( (bits & uint(0x0f0f0f0f)) << uint(4) ) | ( (bits & uint(0xf0f0f0f0)) >> uint(4) );
	//bits = ( (bits & uint(0x33333333)) << uint(2) ) | ( (bits & uint(0xcccccccc)) >> uint(2) );
	//bits = ( (bits & uint(0x55555555)) << uint(1) ) | ( (bits & uint(0xaaaaaaaa)) >> uint(1) );
	bits = ( (bits & 16711935u) << uint(8) ) | ( (bits & 4278255360u) >> uint(8) );
	bits = ( (bits & 252645135u) << uint(4) ) | ( (bits & 4042322160u) >> uint(4) );
	bits = ( (bits & 858993459u) << uint(2) ) | ( (bits & 3435973836u) >> uint(2) );
	bits = ( (bits & 1431655765u) << uint(1) ) | ( (bits & 2863311530u) >> uint(1) );
	return bits;
}

float2 Hammersley( uint Index, uint NumSamples )
{
	float E1 = fract( float(Index) / float(NumSamples) );
	//float E2 = float( ReverseBits32(Index) ) * 2.3283064365386963e-10;
	float E2 = float( ReverseBits32(Index) ) * 0.00000000023283064365386963;
	return float2( E1, E2 );
}


float3 ImportanceSampleGGX( float2 Xi, float Roughness, float3 N )
{
	float a = Roughness * Roughness;

	float Phi = 2.0 * PI * Xi.x;
	float CosTheta = sqrt( (1.0 - Xi.y) / ( 1.0 + (a*a - 1.0) * Xi.y ) );
	float SinTheta = sqrt( 1.0 - CosTheta * CosTheta );
	float3 H;
	H.x = SinTheta * cos( Phi );
	H.y = SinTheta * sin( Phi );
	H.z = CosTheta;

	float3 Upfloat2tor = abs(N.z) < 0.999 ? float3(0.0,0.0,1.0) : float3(1.0,0.0,0.0);
	float3 TangentX = normalize( cross( Upfloat2tor, N ) );
	float3 TangentY = cross( N, TangentX );

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}


uniform samplerCube<float> envmap0;
uniform float roughness;


float3 PrefilterEnvMap( float Roughness , float3 R )
{
	float3 N = R;
	float3 V = R;
	float3 PrefilteredColor = float3(0.0,0.0,0.0);
	float TotalWeight = 0.0;
	//const uint NumSamples = uint(2048);
	const uint NumSamples = uint(1024);

	for (uint i = uint(0); i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2.0 * dot(V, H) * H - V;
		float NoL = saturate( dot( N, L ) );

		if( NoL > 0.0 )
		{
			//float3 color = texture( envmap0, L).rgb;
			float3 color = textureLod( envmap0, L, 0.0).rgb;

			PrefilteredColor += color * NoL;
			TotalWeight += NoL;
		}
	}
	float3 result = PrefilteredColor / max(TotalWeight, 0.00001);
	return result;
}

float4 EncodeRGBE8( float3 rgb )
{
	float4 vEncoded;
	float maxComponent = max(max(rgb.r, rgb.g), rgb.b );
	float fExp = ceil( log2(maxComponent) );
	vEncoded.rgb = rgb / exp2(fExp);
	vEncoded.a = (fExp + 128.0) / 255.0;
	return vEncoded;
}

in float3 position;

void main()
{
	out_res.xyz = PrefilterEnvMap( roughness, normalize(position));
	out_res.w = 1.0;
	
#ifdef ENCODE_RGBE8888	
	out_res = EncodeRGBE8(out_res.xyz);
#endif
}

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

	float3 UpVector = abs(N.z) < 0.999 ? float3(0.0,0.0,1.0) : float3(1.0,0.0,0.0);
	float3 TangentX = normalize( cross( UpVector, N ) );
	float3 TangentY = cross( N, TangentX );

	// Tangent to world space
	return TangentX * H.x + TangentY * H.y + N * H.z;
}

float G_Smith( float a2, float Cos )
{
	return (2.0 * Cos) / (Cos + sqrt(a2 + (1.0 - a2) * (Cos * Cos)));
}

float G_Smith( float Roughness, float NoV, float NoL )
{
	float a = max(Roughness * Roughness, 0.0001);
	float a2 = a*a;


	float G_SmithV = NoV + sqrt( NoV * (NoV - NoV * a2) + a2 );
	float G_SmithL = NoL + sqrt( NoL * (NoL - NoL * a2) + a2 );
//	return 1.0 / ( G_SmithV * G_SmithL );


	float G = (G_Smith(a2, NoL) * G_Smith(a2, max(0.00001, NoV)));
	return G;
}

float2 IntegrateBRDF(float Roughness, float NoV)
{
	float3 V;

	float3 N = float3(0.0,0.0,1.0);

	V.x = sqrt(1.0 - NoV * NoV); // sin
	V.y = 0.0;
	V.z = NoV; // cos
	float A = 0.0;
	float B = 0.0;
	// const uint NumSamples = uint(1024);
	const uint NumSamples = uint(2048);

	for (uint i = uint(0); i < NumSamples; i++)
	{
		float2 Xi = Hammersley(i, NumSamples);
		float3 H = ImportanceSampleGGX(Xi, Roughness, N);
		float3 L = 2.0 * dot(V, H) * H - V;
		float NoL = saturate(L.z);
		float NoH = saturate(H.z);
		float VoH = saturate(dot(V, H));

		//NoL = saturate(L.z);
		//NoH = saturate(H.z);

		if (NoL > 0.0)
		{
			float G = G_Smith(Roughness, NoV, NoL);

			//float G_Vis = G * VoH / (NoH * NoV);
			//float G_Vis = 4.0 * G * VoH * (NoL / NoH);

			float G_Vis = G * VoH / (NoH * max(NoV, 0.000001));

			float Fc = pow(1.0 - VoH, 5.0);

			A += (1.0 - Fc) * G_Vis;
			B += Fc * G_Vis;
		}
	}
	//return float2(1.0);

	return float2(A, B) / float(NumSamples);
}

in float2 texcoord;

void main()
{
	out_res.xy = IntegrateBRDF( texcoord.x, texcoord.y);
	out_res.z = 0.0;
	out_res.w = 0.0;
}

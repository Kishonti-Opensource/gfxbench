/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */

out float4 out_res { color(0) };

#define PI 3.1415268

#define depth_map gbuffer_depth_texture
#define normal_map gbuffer_normal_texture

uniform sampler2D<float> depth_map;
uniform sampler2D<float> normal_map;
uniform float4 corners[4];
uniform float4 inv_resolution;
uniform float4 depth_parameters;
uniform float sao_projection_scale;
uniform float4x4 view;

// sao 1

//total number of samples at each fragment
#define NUM_SAMPLES_1 9
#define NUM_SPIRAL_TURNS_1 7
#define RADIUS_1 0.25

// sao 2

//total number of samples at each fragment
#define NUM_SAMPLES_2 9
#define NUM_SPIRAL_TURNS_2 7
#define RADIUS_2 1.0

in float2 out_texcoord;

//https://gist.github.com/fisch0920/6770311
//https://research.nvidia.com/publication/scalable-ambient-obscurance

#define VARIATION             1

float rand(float2 co)
{
	return (fract(sin(dot(co.xy ,float2(12.9898,78.233))) * 43758.5453));
}


// reconstructs view-space unit normal from view-space position
float3 getPositionVS(float2 uv, float lod)
{
	float d  = textureLod( depth_map, uv, lod ).x;
	d = depth_parameters.y / (d - depth_parameters.x);

	float3 dir0;
	float3 dir1;
	float3 dir2;

	dir0 = mix( corners[0].xyz, corners[1].xyz, uv.x);
	dir1 = mix( corners[2].xyz, corners[3].xyz, uv.x);
	dir2 = mix( dir0, dir1, uv.y);
	return d * dir2;
}

float3 reconstructNormalVS2( float2 uv)
{
	float2 offset1 = float2(inv_resolution.x, 0.0);
	float2 offset2 = float2(0.0, inv_resolution.y);

	float3 p0 = getPositionVS( uv, 0.0 );
	float3 p1 = getPositionVS( uv + offset1, 0.0 );
	float3 p2 = getPositionVS( uv + offset2, 0.0 );

	return normalize(cross(p1-p0, p2-p0));
}


// returns a unit vector and a screen-space radius for the tap on a unit disk
// (the caller should scale by the actual disk radius)
float2 tapLocation(int sampleNumber, float spinAngle, int num_samples, int num_spiral_turns, out float ssr)
{
  // radius relative to radiusSS
  float alpha = (float(sampleNumber) + 0.5) * (1.0 / float(num_samples));
  float angle = alpha * (float(num_spiral_turns) * 2.0 * PI) + spinAngle;

  ssr = alpha; //screen space radius?
  return float2(alpha * cos(angle), alpha * sin(angle));
}

// If using depth mip levels, the log of the maximum pixel offset before we need to switch to a lower 
// miplevel to maintain reasonable spatial locality in the cache
// If this number is too small (< 3), too many taps will land in the same pixel, and we'll get bad variance that manifests as flashing.
// If it is too high (> 5), we'll get bad performance because we're not using the MIP levels effectively
#define LOG_MAX_OFFSET (3)

// This must be less than or equal to the MAX_MIP_LEVEL defined in SSAO.cpp
#define MAX_MIP_LEVEL (4)

/** Read the camera-space position of the point at screen-space pixel ssP + unitOffset * ssR.  Assumes length(unitOffset) == 1 */
float getMipLevel(float ssR) {
    // Derivation:
    //  mipLevel = floor(log(ssR / MAX_OFFSET));
    int mipLevel = clamp(int(floor(log2(ssR))) - LOG_MAX_OFFSET, 0, MAX_MIP_LEVEL);

	return mipLevel;
	
    /*int2 ssP = int2(ssR * unitOffset) + ssC;
	
    float3 P;

    // We need to divide by 2^mipLevel to read the appropriately scaled coordinate from a MIP-map.  
    // Manually clamp to the texture size because texelFetch bypasses the texture unit
    int2 mipP = clamp(ssP >> mipLevel, int2(0), textureSize(depth_map, mipLevel) - int2(1));
    P.z = texelFetch(depth_map, mipP, mipLevel).r;

    // Offset to pixel center
    P = reconstructCSPosition(float2(ssP) + float2(0.5), P.z);

    return P;*/
}

float sampleAO(float2 uv, float3 positionVS, float3 normalVS, float radius2, float sampleRadiusSS, int tapIndex, float rotationAngle, int num_samples, int num_spiral_turns)
{
	const float epsilon1 = 0.3; //ettol kevesbe kontrasztos az AO es nem lehet nulla, mert akkor nullaval osztas
	const float depth_bias = 0.04; //minimalis res merete amit erzekel, igy ez egyuttal a depth precision hibait is javitva

	float ssr;
	
	// offset on the unit disk, spun for this pixel
	int2 unitOffset = int2(tapLocation(tapIndex, rotationAngle, num_samples, num_spiral_turns) * sampleRadiusSS);
	ssr *= sampleRadiusSS;
	float3 Q = getPositionVS( uv + float2(unitOffset) * inv_resolution.xy, getMipLevel(ssr) );
	float3 v = Q - positionVS;

	float vv = dot(v, v);
	float vn = dot(v, normalVS) - depth_bias;
	
	return getMipLevel(ssr);

#if VARIATION == 0

  // (from the HPG12 paper)
  // Note large epsilon to avoid overdarkening within cracks
  return float(vv < radius2) * max(vn / (epsilon1 + vv), 0.0);

#elif VARIATION == 1 // default / recommended

  // Smoother transition to zero (lowers contrast, smoothing out corners). [Recommended]
  float f = max(radius2 - vv, 0.0) / radius2;
  return f * max(vn / (epsilon1 + vv), 0.0);

#elif VARIATION == 2

  // Medium contrast (which looks better at high radii), no division.  Note that the
  // contribution still falls off with radius^2, but we've adjusted the rate in a way that is
  // more computationally efficient and happens to be aesthetically pleasing.
  float invRadius2 = 1.0 / radius2;
  return 128.0 * max(1.0 - vv * invRadius2, 0.0) * max(vn, 0.0);

#else

  // Low contrast, no division operation
  return 32.0 * float(vv < radius2) * max(vn, 0.0);

#endif
}

void main()
{
	//NOTE: nem erdemes megforditani az algoritmust (konstans mintavetelezesi sugar, vizsgalati sugar novekszik tavolsaggal), mert ronda

	float3 originVS = getPositionVS( out_texcoord, 0.0 );
	float3 normalVS = texture(normal_map, out_texcoord).xyz;
	normalVS = normalVS * 2.0 - 1.0;

	normalVS = (view * float4(normalVS, 0.0)).xyz;

	float randomPatternRotationAngle = 2.0 * PI * rand(out_texcoord);
	
	float occlusion1 = 0.0;
	float occlusion2 = 0.0;
	
	//SAO 1
	{
		float radiusSS = RADIUS_1 * sao_projection_scale / -originVS.z;
		for (int i = 0; i < NUM_SAMPLES_1; ++i)
		{
			occlusion1 += sampleAO(out_texcoord, originVS, normalVS, RADIUS_1 * RADIUS_1, radiusSS, i, randomPatternRotationAngle, NUM_SAMPLES_1, NUM_SPIRAL_TURNS_1);
		}
		occlusion1 = 1.0 - occlusion1 / float(  NUM_SAMPLES_1);
	}
	
	//SAO 2
	{
		float radiusSS = RADIUS_2 * sao_projection_scale / -originVS.z;
		for (int i = 0; i < NUM_SAMPLES_2; ++i)
		{
			occlusion2 += sampleAO(out_texcoord, originVS, normalVS, RADIUS_2 * RADIUS_2, radiusSS, i, randomPatternRotationAngle, NUM_SAMPLES_2, NUM_SPIRAL_TURNS_2);
		}
		occlusion2 = 1.0 - occlusion2 / float(  NUM_SAMPLES_2);
		occlusion2 = pow(occlusion2,0.15);
	}
	
	
	occlusion1 = 1.0 - occlusion1;
	
	if( occlusion1 < 1.0 )
	{
		out_res = float4( 1.0, 0.0, 0.0, 1.0 );
	}
	else if( occlusion1 < 2.0 )
	{
		out_res = float4( 0.0, 1.0, 0.0, 1.0 );
	}
	else if( occlusion1 < 3.0 )
	{
		out_res = float4( 0.0, 0.0, 1.0, 1.0 );
	}
	else if( occlusion1 < 4.0 )
	{
		out_res = float4( 1.0, 0.0, 1.0, 1.0 );
	}
	else if( occlusion1 < 5.0 )
	{
		out_res = float4( 1.0, 1.0, 1.0, 1.0 );
	}
	
	return;

	//float occlusion = occlusion1*occlusion2;
	//float occlusion = (occlusion1+occlusion2)*0.5; //+mid range
	float occlusion = occlusion1;
	//occlusion = occlusion2;
	
	occlusion = pow(occlusion, 128.0);

	out_res = float4( occlusion, occlusion, occlusion, 1.0);
}

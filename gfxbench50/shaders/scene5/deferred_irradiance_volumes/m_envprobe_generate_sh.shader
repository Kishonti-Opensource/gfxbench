/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define envprobe_atlas_inv_size float2(1.0 / (6.0 * 16.0), 1.0 / (16.0 * MAX_NUM_OF_PROBES))

#ifdef TYPE_vertex

in float2 in_position;
in float2 in_texcoord0_;

out float2 v_texcoord;

void main()
{
	gl_Position = float4( in_position.xy, 0.0, 1.0);
	v_texcoord = in_texcoord0_;
}

#endif


#ifdef TYPE_fragment

uniform sampler2D<float> m_envprobe_envmap_atlas;

in float2 v_texcoord;

out float4 FragColor { color(0) }; 


//cosine kernel
#define A0 1.0
#define A1 0.66666
#define A2 0.25


//SH consts
#define SHconst_0 0.28209479177387814347 
#define SHconst_1 0.48860251190291992159 
#define SHconst_2 1.09254843059207907054 
#define SHconst_3 0.31539156525252000603 
#define SHconst_4 0.54627421529603953527 

  
float3 sampleSide(float side, float3x3 rot, float c0, float4 c1, float4 c2)
{
	float3 result = float3(0.0);
	float divider = 0.0;

	for(int i=0; i<256; i++)
	{
		float x = float(i % 16);
		float y = float(i / 16);

		//itt 16-osával ugrálunk, mert akkora egy lightprobe mérete
		//x - 6 oldal van egymás mellé kiterítve
		//y - 47 probe van egymás alatt
		float2 texcoord = (float2(x+side*16.0, y+floor(gl_FragCoord.y)*16.0)+0.5) * envprobe_atlas_inv_size;
		float3 texel = texture(m_envprobe_envmap_atlas, texcoord).xyz;

		//uv alapján normált számolunk és majd face alapján beforgatjuk világba
		float2 sidecoord = ((float2(x,y)+float2(0.5, 0.5))/float2(16.0))*2.0-1.0;
		float3 normal = normalize(float3(sidecoord, -1.0));

		float3 N2 = rot * normal;
		float3 N3 = N2.xyz * N2.yzz;
		float3 N4 = N2.xyz * N2.xyx;
		float4 W1 = float4(N2.y, N2.z, N2.x, N3.x);
		float4 W2 = float4(N3.y, 3.0 * N3.z - 1.0, N4.z, N4.x - N4.y);
		
		float W = c0 + dot(W1, c1) + dot(W2, c2);
		
		//texelt súlyozzuk, amit a koeff sorszáma és világbeli normál alapján számoljuk
		result += W * texel * -normal.z;
		divider += -normal.z;
	}
	return result / divider;
}


const float C0[9] = 
{
	A0 * SHconst_0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0,
	0.0
};

const float4 C1[9] = 
{
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(-A1 * SHconst_1, 0.0, 0.0, 0.0), 
	float4(0.0, A1 * SHconst_1, 0.0, 0.0), 
	float4(0.0, 0.0, -A1 * SHconst_1, 0.0), 
	float4(0.0, 0.0, 0.0, A2 * SHconst_2), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0)
};

const float4 C2[9] =
{
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(0.0, 0.0, 0.0, 0.0), 
	float4(-A2 * SHconst_2, 0.0, 0.0, 0.0), 
	float4(0.0, A2 * SHconst_3, 0.0, 0.0), 
	float4(0.0, 0.0, -A2 * SHconst_2, 0.0), 
	float4(0.0, 0.0, 0.0, A2 * SHconst_4) 
};

const float C3[9] = 
{
	SHconst_0,
	-SHconst_1,
	SHconst_1,
	-SHconst_1,
	SHconst_2,
	-SHconst_2,
	SHconst_3,
	-SHconst_2,
	SHconst_4
};

	
void main()
{
	float3x3 mpx = float3x3(
		float3(  0.0,  0.0, -1.0),
		float3(  0.0, -1.0,  0.0),
		float3( -1.0,  0.0,  0.0)
	);

	float3x3 mnx = float3x3(
		float3(  0.0,  0.0,  1.0),
		float3(  0.0, -1.0,  0.0),
		float3(  1.0,  0.0,  0.0)
	);

	float3x3 mpy = float3x3(
		float3(  1.0,  0.0,  0.0),
		float3(  0.0,  0.0,  1.0),
		float3(  0.0, -1.0,  0.0)
	);

	float3x3 mny = float3x3(
		float3(  1.0,  0.0,  0.0),
		float3(  0.0,  0.0, -1.0),
		float3(  0.0,  1.0,  0.0)
	);
	
	float3x3 mpz = float3x3(
		float3(  1.0,  0.0,  0.0),
		float3(  0.0, -1.0,  0.0),
		float3(  0.0,  0.0, -1.0)
	);

	float3x3 mnz = float3x3(
		float3( -1.0,  0.0,  0.0),
		float3(  0.0, -1.0,  0.0),
		float3(  0.0,  0.0,  1.0)
	);

	int index = int(gl_FragCoord.x);

	float c0 = C0[index];
	float4 c1 = C1[index];
	float4 c2 = C2[index];

	float3 result = (
		sampleSide(0.0, mpx, c0, c1, c2) +
		sampleSide(1.0, mnx, c0, c1, c2) +
		sampleSide(2.0, mpy, c0, c1, c2) +
		sampleSide(3.0, mny, c0, c1, c2) +
		sampleSide(4.0, mpz, c0, c1, c2) +
		sampleSide(5.0, mnz, c0, c1, c2)
	)/6.0;
	
	result *= C3[index];

	FragColor = float4(result, 1.0);
}

#endif

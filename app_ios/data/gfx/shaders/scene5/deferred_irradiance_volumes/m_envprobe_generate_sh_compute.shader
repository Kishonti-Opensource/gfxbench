/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#define envprobe_atlas_inv_size half2( half(1.0 / (6.0 * 16.0)), half(1.0 / (16.0 * MAX_NUM_OF_PROBES)) )

uniform sampler2D<half> m_envprobe_envmap_atlas;
#ifdef USE_TEXTURE_SH_ATLAS
image2D<half> m_envprobe_sh_atlas_texture { rgba16f, writeonly };
#else
buffer float4 m_envprobe_sh_atlas[16 * MAX_NUM_OF_PROBES_INT] { ssbo };
#endif

uniform uint4 envprobe_index[MAX_NUM_OF_PROBES_INT];


//cosine kernel
#define A0 1.0h
#define A1 0.66666h
#define A2 0.25h


//SH consts
#define SHconst_0 0.28209479177387814347h 
#define SHconst_1 0.48860251190291992159h 
#define SHconst_2 1.09254843059207907054h 
#define SHconst_3 0.31539156525252000603h 
#define SHconst_4 0.54627421529603953527h 


shared half4 data[6u*WORKGROUP_SIZE_X];

#define VALUE_PER_THREAD (256u / WORKGROUP_SIZE_X)


void CalcHarmonics(uint cf_id, half3 texels[6u*VALUE_PER_THREAD], half c0, half4 c1, half4 c2)
{
	const half3x3 m[6] =
	{
    half3x3(
		half3(  0.0h,  0.0h, -1.0h),
		half3(  0.0h, -1.0h,  0.0h),
		half3( -1.0h,  0.0h,  0.0h)
	),
    half3x3(
		half3(  0.0h,  0.0h,  1.0h),
		half3(  0.0h, -1.0h,  0.0h),
		half3(  1.0h,  0.0h,  0.0h)
	),
    half3x3(
		half3(  1.0h,  0.0h,  0.0h),
		half3(  0.0h,  0.0h,  1.0h),
		half3(  0.0h, -1.0h,  0.0h)
	),
    half3x3(
		half3(  1.0h,  0.0h,  0.0h),
		half3(  0.0h,  0.0h, -1.0h),
		half3(  0.0h,  1.0h,  0.0h)
	),
	half3x3(
		half3(  1.0h,  0.0h,  0.0h),
		half3(  0.0h, -1.0h,  0.0h),
		half3(  0.0h,  0.0h, -1.0h)
	),
    half3x3(
		half3( -1.0h,  0.0h,  0.0h),
		half3(  0.0h, -1.0h,  0.0h),
		half3(  0.0h,  0.0h,  1.0h)
	)
	};
    
	half4 r[6] = { half4(0.0), half4(0.0), half4(0.0), half4(0.0), half4(0.0), half4(0.0) };
	
	for(uint k = 0u; k < VALUE_PER_THREAD; k++)
	{
		uint i = gl_LocalInvocationID.x * VALUE_PER_THREAD + k;
		
		half x = half(i % 16u);
		half y = half(i / 16u);
		
		//uv alapján normált számolunk és majd face alapján beforgatjuk világba
		half2 sidecoord = ((half2(x,y)+half2(0.5))/half2(16.0))*2.0h-1.0h;
		half3 normal = normalize(half3(sidecoord, -1.0h));

#if 0
		for(uint side = 0u; side < 6u; side++)
		{	
			half3 texel = texels[6u * k + side];

			half3 N2 = m[side] * normal;
			half3 N3 = N2.xyz * N2.yzz;
			half3 N4 = N2.xyz * N2.xyx;
			half4 W1 = half4(N2.y, N2.z, N2.x, N3.x);
			half4 W2 = half4(N3.y, 3.0h * N3.z - 1.0h, N4.z, N4.x - N4.y);
		
			half W = c0 + dot(W1, c1) + dot(W2, c2);
		
			//texelt súlyozzuk, amit a koeff sorszáma és világbeli normál alapján számoljuk
			r[side].xyz += W * texel * -normal.z;
			r[side].w += -normal.z;
		}
#else
		// side == 0u
		half3 texel = texels[6u*k+0u];

		half3 N2 = m[0u]*normal;
		half3 N3 = N2.xyz*N2.yzz;
		half3 N4 = N2.xyz*N2.xyx;
		half4 W1 = half4(N2.y, N2.z, N2.x, N3.x);
		half4 W2 = half4(N3.y, 3.0h * N3.z - 1.0h, N4.z, N4.x - N4.y);

		half W = c0 + dot(W1, c1) + dot(W2, c2);

		r[0u].xyz += W * texel * -normal.z;
		r[0u].w +=  -normal.z;

		// side == 1u
		texel = texels[6u*k+1u];

		N2 = m[1u]*normal;
		N3 = N2.xyz*N2.yzz;
		N4 = N2.xyz*N2.xyx;
		W1 = half4(N2.y,N2.z,N2.x,N3.x);
		W2 = half4(N3.y,3.0h*N3.z-1.0h,N4.z,N4.x-N4.y);

		W = c0+dot(W1,c1)+dot(W2,c2);

		r[1u].xyz += W*texel* -normal.z;
		r[1u].w +=  -normal.z;

		// side == 2u
		texel = texels[6u*k+2u];

		N2 = m[2u]*normal;
		N3 = N2.xyz*N2.yzz;
		N4 = N2.xyz*N2.xyx;
		W1 = half4(N2.y,N2.z,N2.x,N3.x);
		W2 = half4(N3.y,3.0h*N3.z-1.0h,N4.z,N4.x-N4.y);

		W = c0+dot(W1,c1)+dot(W2,c2);

		r[2u].xyz += W*texel* -normal.z;
		r[2u].w +=  -normal.z;

		// side == 3u
		texel = texels[6u*k+3u];

		N2 = m[3u]*normal;
		N3 = N2.xyz*N2.yzz;
		N4 = N2.xyz*N2.xyx;
		W1 = half4(N2.y,N2.z,N2.x,N3.x);
		W2 = half4(N3.y,3.0h*N3.z-1.0h,N4.z,N4.x-N4.y);

		W = c0+dot(W1,c1)+dot(W2,c2);


		r[3u].xyz += W*texel* -normal.z;
		r[3u].w +=  -normal.z;

		// side == 4u
		texel = texels[6u*k+4u];

		N2 = m[4u]*normal;
		N3 = N2.xyz*N2.yzz;
		N4 = N2.xyz*N2.xyx;
		W1 = half4(N2.y,N2.z,N2.x,N3.x);
		W2 = half4(N3.y,3.0h*N3.z-1.0h,N4.z,N4.x-N4.y);

		W = c0+dot(W1,c1)+dot(W2,c2);


		r[4u].xyz += W*texel* -normal.z;
		r[4u].w +=  -normal.z;

		// side == 5u
		texel = texels[6u*k+5u];

		N2 = m[5u]*normal;
		N3 = N2.xyz*N2.yzz;
		N4 = N2.xyz*N2.xyx;
		W1 = half4(N2.y,N2.z,N2.x,N3.x);
		W2 = half4(N3.y,3.0h*N3.z-1.0h,N4.z,N4.x-N4.y);

		W = c0+dot(W1,c1)+dot(W2,c2);


		r[5u].xyz += W*texel* -normal.z;
		r[5u].w +=  -normal.z;
#endif
	}
	
	for (uint side = 0u; side < 6u; side++)
	{
		uint data_id = 6u * gl_LocalInvocationID.x + side;
		data[ data_id ] = r[side];
	}
}


void Reduction()
{
	for(uint s = WORKGROUP_SIZE_X / 2u; s > 0u; s = s >> 1u)
	{
		if(gl_LocalInvocationID.x < s)
		{
			uint data_id = 6u * gl_LocalInvocationID.x;
			
			uint t = 6u* s + data_id;
			data[data_id+0u] += data[t+0u];
			data[data_id+1u] += data[t+1u];
			data[data_id+2u] += data[t+2u];
			data[data_id+3u] += data[t+3u];
			data[data_id+4u] += data[t+4u];
			data[data_id+5u] += data[t+5u];
		}
		workgroupMemoryBarrierShared();
	}
}


numthreads(WORKGROUP_SIZE_X, WORKGROUP_SIZE_Y, WORKGROUP_SIZE_Z);	
void main()
{
	const half C0[9] = 
	{
		A0 * SHconst_0,
		0.0h,
		0.0h,
		0.0h,
		0.0h,
		0.0h,
		0.0h,
		0.0h,
		0.0h
	};

	const half4 C1[9] = 
	{
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(-A1 * SHconst_1, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, A1 * SHconst_1, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, -A1 * SHconst_1, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, A2 * SHconst_2), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h)
	};

	const half4 C2[9] =
	{
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, 0.0h), 
		half4(-A2 * SHconst_2, 0.0h, 0.0h, 0.0h), 
		half4(0.0h, A2 * SHconst_3, 0.0h, 0.0h), 
		half4(0.0h, 0.0h, -A2 * SHconst_2, 0.0h), 
		half4(0.0h, 0.0h, 0.0h, A2 * SHconst_4) 
	};

	const half C3[9] = 
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

	uint envprobe_id = envprobe_index[gl_WorkGroupID.y].x;

	half3 texels[6u*VALUE_PER_THREAD];
	
	// fetchTiles
	for(uint side = 0u; side < 6u; side++)
	{
		for(uint k = 0u; k < VALUE_PER_THREAD; k++)
		{
			uint i = gl_LocalInvocationID.x * VALUE_PER_THREAD + k;
			
			half x = half(i % 16u);
			half y = half(i / 16u);
			
			//itt 16-osával ugrálunk, mert akkora egy lightprobe mérete
			//x - 6 oldal van egymás mellé kiterítve
			//y - 47 probe van egymás alatt
			half2 texcoord = (half2(x+half(side)*16.0h, y+half(envprobe_id)*16.0h)+0.5h) * envprobe_atlas_inv_size;
			texels[6u * k + side] = textureLod(m_envprobe_envmap_atlas, float2(texcoord), 0.0).xyz;
		}
	}
	
	for (uint i = 0u; i < 9u; i++)
	{
		CalcHarmonics(i, texels, C0[i], C1[i], C2[i]);
		
		workgroupMemoryBarrierShared();
		
		Reduction();
		
		if (gl_LocalInvocationID.x == 0u)
		{
			uint id = i;

#if 0
			half4 d0 = data[0];
			half4 d1 = data[1];
			half4 d2 = data[2];
			half4 d3 = data[3];
			half4 d4 = data[4];
			half4 d5 = data[5];
				
			half3 result = (
				d0.xyz/d0.w +
				d1.xyz/d1.w +
				d2.xyz/d2.w +
				d3.xyz/d3.w +
				d4.xyz/d4.w +
				d5.xyz/d5.w
			)/6.0h;
#else
			half3 result = (
				data[0].xyz/data[0].w +
				data[1].xyz/data[1].w +
				data[2].xyz/data[2].w +
				data[3].xyz/data[3].w +
				data[4].xyz/data[4].w +
				data[5].xyz/data[5].w
			)/6.0h;
#endif
		
			result *= C3[id];
#ifdef USE_TEXTURE_SH_ATLAS
			imageStore(m_envprobe_sh_atlas_texture, int2(id, envprobe_id) , half4(result, 1.0h));
#else
			m_envprobe_sh_atlas[(envprobe_id * 16u) + id] = float4(float3(result), 1.0);
#endif
		}
	}
}


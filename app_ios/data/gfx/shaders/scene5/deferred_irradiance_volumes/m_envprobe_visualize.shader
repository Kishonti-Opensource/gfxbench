/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
force_highp;

#ifdef TYPE_vertex

uniform float4x4 mvp;

in float3 in_position;
in float3 in_normal;

out float3 v_normal;

void main()
{
	gl_Position = mvp * float4( in_position.xyz, 1.0);
	v_normal = in_normal;
}

#endif


#ifdef TYPE_fragment

in float3 v_normal;

out float4 FragColor { color(0) };

uniform sampler2D<float> m_envprobe_envmap_atlas;
#ifdef USE_TEXTURE_SH_ATLAS
uniform sampler2D<half> m_envprobe_sh_atlas_texture;
#else
buffer half4 m_envprobe_sh_atlas[9];
#endif
uniform float envprobe_index;

half3 getCoefficient(uint m)
{
#ifdef USE_TEXTURE_SH_ATLAS
	float2 coord = float2(half(m)+0.5, envprobe_index+0.5)/float2(9.0, MAX_NUM_OF_PROBES);
	return texture(m_envprobe_sh_atlas_texture, coord).rgb;
#else
	return m_envprobe_sh_atlas[m].xyz;
#endif
}

half3 sphericalHarmonics(float3 normal){
    float x = normal.x;
    float y = normal.y;
    float z = normal.z;

    half3 l00 = getCoefficient(0u);

    half3 l10 = getCoefficient(1u);
    half3 l11 = getCoefficient(2u);
    half3 l12 = getCoefficient(3u);

    half3 l20 = getCoefficient(4u);
    half3 l21 = getCoefficient(5u);
    half3 l22 = getCoefficient(6u);
    half3 l23 = getCoefficient(7u);
    half3 l24 = getCoefficient(8u);

    half3 result = (
        l00 +
        l10 * y +
        l11 * z +
        l12 * x +
        l20 * x*y +
        l21 * y*z +
        l22 * (3.0*z*z - 1.0) +
        l23 * x*z +
        l24 * (x*x - y*y)
    );
    return max(result, half3(0.0h));
}


//http://stackoverflow.com/questions/22510617/virtual-shadow-depth-cube-texture-vsdct-without-indirection-texture
float3 GetShadowTC( float3 Dir , int index)
{
    float Sc;
    float Tc;
    float Ma;
    int FaceIndex;

    float rx = Dir.x;
    float ry = Dir.y;
    float rz = Dir.z;

    float3 adir = abs(Dir);
    Ma = max( max( adir.x, adir.y ), adir.z );

    // -X +X ok
    if ( adir.x > adir.y && adir.x > adir.z )
    {
        Sc = ( rx > 0.0 ) ? -rz : rz;
        Tc = -ry;
        FaceIndex = ( rx > 0.0 ) ? 0 : 1;
    }
    // -Y +Y ok
    else if ( adir.y > adir.x && adir.y > adir.z )
    {
        Sc = rx;
        Tc = ( ry > 0.0 ) ? rz : -rz;
        FaceIndex = ( ry > 0.0 ) ? 2 : 3;
    }
    // -Z +Z ok
    else
    {
        Sc = ( rz > 0.0 ) ? rx : -rx;
        Tc = -ry;
        FaceIndex = ( rz > 0.0 ) ? 4 : 5;
    }

    float s = 0.5 * ( Sc / Ma + 1.0 );
    float t = 0.5 * ( Tc / Ma + 1.0 );

    s = s / (6.0);
	t = t / (MAX_NUM_OF_PROBES);

    s += (float(FaceIndex % 6 ) * 16.0) / (6.0 * 16.0);
    t += (float(index % int(MAX_NUM_OF_PROBES) ) * 16.0) / (MAX_NUM_OF_PROBES * 16.0);

	if( FaceIndex != 1 )
	{
		//s = 0.0; t = 0.0;
	}

    return float3( s, t, float(FaceIndex));
}


void main()
{
	float3 unwrap_tc = GetShadowTC(normalize(v_normal), int(envprobe_index));
	float3 test_dir;

	{
		float x, y;

		int FaceIndex = int(unwrap_tc.z);
		int index = int(envprobe_index);


		x = unwrap_tc.x - (float(FaceIndex % 6 ) * 16.0) / (6.0 * 16.0);
		y = unwrap_tc.y - (float(index % int(MAX_NUM_OF_PROBES) ) * 16.0) / (MAX_NUM_OF_PROBES * 16.0);

		x = x * (6.0);
		y = y * (MAX_NUM_OF_PROBES);

		x = x * 2.0 - 1.0;
		y = y * 2.0 - 1.0;

		test_dir.x = 0.0;
		test_dir.y = 0.0;
		test_dir.z = 0.0;

		float2 sidecoord = float2(x,y);
		float3 normal = normalize(float3(sidecoord, -1.0));

		if( FaceIndex == 0  )
		{
			float3x3 rot = float3x3(
				float3(0.0,  0.0,  -1.0),
				float3(0.0,  -1.0,  0.0),
				float3(-1.0,  0.0,  0.0)
			);

			test_dir = rot*normal;
		}
		if( FaceIndex == 1  )
		{
			float3x3 rot = float3x3(
				float3(0.0,  0.0,  1.0),
				float3(0.0,  -1.0,  0.0),
				float3(1.0,  0.0,  0.0)
			);

			test_dir = rot*normal;
		}
		if( FaceIndex == 2  )
		{
			float3x3 rot = float3x3(
				float3(1.0,  0.0,  0.0),
				float3(0.0,  0.0,  1.0),
				float3(0.0,  -1.0,  0.0)
			);

			test_dir = rot*normal;
		}
		if( FaceIndex == 3  )
		{
			float3x3 rot = float3x3(
				float3(1.0,  0.0,  0.0),
				float3(0.0,  0.0,  -1.0),
				float3(0.0,  1.0,  0.0)
			);

			test_dir = rot*normal;
		}
		if( FaceIndex == 4  )
		{
			float3x3 rot = float3x3(
				float3(1.0,  0.0,  0.0),
				float3(0.0,  -1.0,  0.0),
				float3(0.0,  0.0,  -1.0)
			);

			test_dir = rot*normal;
		}
		if( FaceIndex == 5  )
		{
			float3x3 rot = float3x3(
				float3(-1.0,  0.0,  0.0),
				float3(0.0,  -1.0,  0.0),
				float3(0.0,  0.0,  1.0)
			);

			test_dir = rot*normal;
		}
	}

	//unwrap_tc = GetShadowTC(normalize(test_dir), int(envprobe_index));

#if RENDER_PROBES_SH
	FragColor.xyz = sphericalHarmonics(normalize(v_normal)) * 1.0;
#else
	FragColor.xyz = texture(m_envprobe_envmap_atlas, unwrap_tc.xy).xyz * 1.0;
#endif
	//FragColor.xyz = test_dir;
	FragColor.w = 1.0;
	//FragColor.xy = unwrap_tc;
	//FragColor += float4(1.0);
}

#endif

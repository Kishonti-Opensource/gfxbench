/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef COMPATIBLE_HI_Z_MIPMAP_GEN
#error COMPATIBLE_HI_Z_MIPMAP_GEN not defined
#endif

struct VertexInput
{
	hfloat3 in_position [[attribute(0)]];
};

struct VertexOutput
{
	hfloat4 out_position [[position]];
	hfloat2 out_texcoord0;
};

#ifdef TYPE_fragment

//uniform mediump int2 texture_size;

//in vec2 out_texcoord0;
//out vec2 frag_color;

#if COMPATIBLE_HI_Z_MIPMAP_GEN
struct HiZFragmentUniforms
{
	hfloat inv_width, inv_height;
};
#endif

struct FragmentOutput
{
	hfloat out_depth [[ depth(any) ]];
};

fragment FragmentOutput shader_main(VertexOutput input [[stage_in]],
									depth2d<hfloat> texture_unit0 [[texture(0)]],
#if COMPATIBLE_HI_Z_MIPMAP_GEN
									constant HiZFragmentUniforms &fu [[ buffer(0) ]],
#endif
									sampler depth_sampler [[sampler(0)]])
{
	FragmentOutput output;

    hfloat4 samples;

#if COMPATIBLE_HI_Z_MIPMAP_GEN
    hfloat2 tc = (floor(input.out_position.xy) + hfloat2(0.25,0.25)) * hfloat2(fu.inv_width,fu.inv_height);
#else
    hfloat2 tc = input.out_texcoord0;
#endif

#if COMPATIBLE_HI_Z_MIPMAP_GEN
    samples.x = texture_unit0.sample(depth_sampler, tc, int2( 0,0));
    samples.y = texture_unit0.sample(depth_sampler, tc, int2( 1,0));
    samples.z = texture_unit0.sample(depth_sampler, tc, int2( 1,1));
    samples.w = texture_unit0.sample(depth_sampler, tc, int2( 0,1));
#else
	samples = texture_unit0.gather(depth_sampler, tc, int2(0, 0));
#endif

    hfloat max_z = max(max(samples.x, samples.y), max(samples.z, samples.w));

    hfloat3 extra;
	uint2 texture_size = {texture_unit0.get_width(), texture_unit0.get_height()};
    int2 src_choords = int2(input.out_position.xy + 0.5) * 2;
    bool2 is_odd = bool2((texture_size.x & 1) != 0, (texture_size.y & 1) != 0);
    if (is_odd.x && src_choords.x == texture_size.x - 3)
    {
         if (is_odd.y && src_choords.y == texture_size.y - 3)
        {
            extra.z = texture_unit0.sample(depth_sampler, tc, int2( 1, 1));
            max_z = max( max_z, extra.z );
        }

        extra.x = texture_unit0.sample(depth_sampler, tc, int2( 1, 0));
        extra.y = texture_unit0.sample(depth_sampler, tc, int2( 1,-1));
        max_z = max( max_z, max( extra.x, extra.y ) );
    }
    else if (is_odd.y && src_choords.y == texture_size.y - 3)
    {
         extra.x = texture_unit0.sample(depth_sampler, tc, int2( -1, 1));
         extra.y = texture_unit0.sample(depth_sampler, tc, int2( 0,1));
         max_z = max( max_z, max( extra.x, extra.y ) );
    }

    output.out_depth = max_z;

	return output;
}

#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = _float4(input.in_position, 1.0);
    output.out_position.y *= -1.0;
    output.out_texcoord0 = input.in_position.xy * 0.5 + 0.5;

    return output;
}

#endif

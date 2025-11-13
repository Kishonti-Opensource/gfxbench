/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

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

#define EPS 0.0001
#define SOFT_Z_EXTENT_INV 10.0

#define SAMPLE_COUNT_DIV_4 (SAMPLE_COUNT/4)

struct MotionBlurConstants
{
	hfloat4 mb_velocity_min_max_sfactor_pad;
	hfloat4 depth_parameters;
};

_float softDepthCompare(_float z_a, _float z_b)
{
  return clamp(1.0 - (z_a - z_b) * SOFT_Z_EXTENT_INV, 0.0, 1.0);
}

_float cone(_float length_xy, _float length_v)
{
    return clamp(1.0 - length_xy/(length_v + 0.001), 0.0, 1.0);
}

_float cylinder(_float length_xy, hfloat length_v)
{
   return 1.0 - smoothstep(0.95 * length_v, 1.05 * length_v, hfloat(length_xy));
}

// Vectorized version
_float4 softDepthCompare4(_float4 z_a, _float4 z_b)
{
  return clamp(_float4(1.0) - (z_a - z_b) * SOFT_Z_EXTENT_INV, _float4(0.0), _float4(1.0));
}

_float4 cone4(_float4 length_xy, _float4 length_v)
{
    return clamp(_float4(1.0) - length_xy/(length_v + 0.001), _float4(0.0), _float4(1.0));
}

_float4 cylinder4(_float4 length_xy, _float4 length_v)
{
   return _float4(1.0) - smoothstep(_float(0.95) * length_v, _float(1.05) * length_v, length_xy);
}

// MODE indicates the algorithm
// 0 - Adaptive
// 1 - Vectorized adaptive
// 2 - Vectorized

// texture_unit0 - color
// texture_unit1 - depth
// texture_unit4 - velocity buffer
// texture_unit6 - neighbor max

constexpr sampler sampler0(coord::normalized, filter::linear, mip_filter::none, address::clamp_to_edge);
constexpr sampler sampler4(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);
constexpr sampler sampler6(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);
constexpr sampler depth_sampler(coord::normalized, filter::nearest, mip_filter::none, address::clamp_to_edge);

fragment _float4 shader_main(VertexOutput input [[stage_in]]
							,constant MotionBlurConstants * mb_consts [[buffer(0)]]
                            ,constant hfloat * tap_offsets [[buffer(1)]]
							,texture2d<_float> texture_unit0 [[texture(TEXTURE_UNIT0_SLOT)]]
							,texture2d<_float> texture_unit4 [[texture(TEXTURE_UNIT4_SLOT)]]
							,texture2d<hfloat> texture_unit6 [[texture(TEXTURE_UNIT6_SLOT)]]
							,depth2d<hfloat> depth_unit0 [[texture(DEPTH_UNIT0_SLOT)]])
{
    int debug_sample_count = 0;

    _float3 res = _float3(0.0, 0.0, 0.0);

    _float3 c0 = texture_unit0.sample(sampler0, input.out_texcoord0).xyz;  // color

#ifdef NEIGHTBOR_TEXTURE_RGBA8
    hfloat2 v_n = unpackVec2FFromVec4Highp(texture_unit6.sample(sampler6, input.out_texcoord0)); // neighbor max
#else
    hfloat2 v_n = texture_unit6.sample(sampler6, input.out_texcoord0).xy; // neighbor max
#endif

	v_n.y = -v_n.y; // [AAPL] Y-FLIP

    hfloat length_vn = length(v_n);
    if ( length_vn < (EPS + 0.5 / hfloat(VP_WIDTH)) )
    {
        res = c0;
    }
    else
// Adaptive
#if MODE == 0
    {
        _float Z_X = -getLinearDepth(depth_unit0, depth_sampler, input.out_texcoord0, mb_consts->depth_parameters);
        //float length_vx = length(unpackVec2FFromVec4(texture_unit4.sample(sampler4, input.out_texcoord0)));
        hfloat length_vx = length(unpack_velocity(texture_unit4, sampler4, input.out_texcoord0));
        hfloat2 X = input.out_texcoord0;

        const _float p = 0.1;
        _float w = 1.0 / (length_vx + p);
        _float3 sum = 1.0 / (length_vx + p) * c0;

        //int sample_count = int (length_vn / mb_consts->mb_velocity_min_max_sfactor_pad.y * _float(SAMPLE_COUNT));
        int sample_count = int(length_vn * mb_consts->mb_velocity_min_max_sfactor_pad.z);
        sample_count = clamp(sample_count, 1, SAMPLE_COUNT);

        debug_sample_count = sample_count;
        for (int i = 0; i < sample_count; i++)
        {
            _float t = tap_offsets[i];

            hfloat2 Y = X + t * v_n;

            _float Z_Y = -getLinearDepth(depth_unit0, depth_sampler, Y, mb_consts->depth_parameters);
            //_float2 V_Y = unpackVec2FFromVec4(texture( texture_unit4, Y));
            hfloat2 V_Y = unpack_velocity(texture_unit4, sampler4, Y);
            hfloat length_vy = length(V_Y);
            _float length_xy = length(X-Y);

            _float f = softDepthCompare(Z_X, Z_Y);
            _float b = softDepthCompare(Z_Y, Z_X);

            _float a = f * cone(length_xy, length_vy);
            a += b * cone(length_xy, length_vx);
            a += cylinder(length_xy, length_vy) * cylinder(length_xy, length_vx) * 2.0;
            w += a;

            sum += a * texture_unit0.sample(sampler0, Y).xyz;
        }
        res = sum / w;
    }

//  res = _float3(normalize(unpackVec2FFromVec4(texture( texture_unit4, input.out_texcoord0))), 0.0);
//  res = _float3(normalize(v_n), 0.0);

// Vectorized adaptive
#elif MODE == 1
    {
        //_float4 vec4_length_vx = vec4(length(unpackVec2FFromVec4(texture( texture_unit4, input.out_texcoord0))));
        _float4 vec4_length_vx = vec4(length(unpack_velocity(texture_unit4, sampler4, input.out_texcoord0)));
        _float4 vec4_zx = vec4(-getLinearDepth(texture_unit1, input.out_texcoord0));
        _float4 vec4_x = input.out_texcoord0.xyxy;
        hfloat4 vec4_vn = v_n.xyxy;

        const _float p = 0.1;
        _float4 vec4_w = vec4(1.0 / (vec4_length_vx.x + p), 0.0, 0.0, 0.0);
        _float3 sum = 1.0 / (vec4_length_vx.x + p) * c0;

        // TODO: this can be pre-calculated on the CPU
        int sample_count = int (vec4_length_vx.x * mb_velocity_min_max_sfactor_pad.z);
        sample_count = (sample_count + 2) / 4;
        sample_count = clamp(sample_count, 1, SAMPLE_COUNT_DIV_4);

        //sample_count = SAMPLE_COUNT_DIV_4;

        debug_sample_count = sample_count * 4;
        for (int i = 0; i < sample_count; i++)
        {
            hfloat4 vec4_t = vectorized_tap_offsets[i];

            hfloat4 Y1 = vec4_x + vec4_vn * vec4_t.xxyy;
            hfloat4 Y2 = vec4_x + vec4_vn * vec4_t.zzww;

            _float4 vec4_Z_Y = -_float4(
                getLinearDepth(texture_unit1, Y1.xy),
                getLinearDepth(texture_unit1, Y1.zw),
                getLinearDepth(texture_unit1, Y2.xy),
                getLinearDepth(texture_unit1, Y2.zw));

            /*
            _float4 vec4_length_vy = _float4(
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.zw))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.zw))));
            */
            _float4 vec4_length_vy = _float4(
                length(unpack_velocity( texture_unit4, Y1.xy)),
                length(unpack_velocity( texture_unit4, Y1.zw)),
                length(unpack_velocity( texture_unit4, Y2.xy)),
                length(unpack_velocity( texture_unit4, Y2.zw)));

            hfloat4 tmp1 = vec4_x - Y1;
            hfloat4 tmp2 = vec4_x - Y2;
            _float4 vec4_length_xy = _float4(length(tmp1.xy), length(tmp1.zw), length(tmp2.xy), length(tmp2.zw));

            _float4 vec4_f = softDepthCompare4(vec4_zx, vec4_Z_Y);
            _float4 vec4_b = softDepthCompare4(vec4_Z_Y, vec4_zx);

            _float4 vec4_a = vec4_f * cone4(vec4_length_xy, vec4_length_vy);
            vec4_a += vec4_b * cone4(vec4_length_xy, vec4_length_vx);
            vec4_a += cylinder4(vec4_length_xy, vec4_length_vy) * cylinder4(vec4_length_xy, vec4_length_vx) * 2.0;

            // Weighted sum of the colors
            sum += vec4_a.x * texture( texture_unit0, Y1.xy).xyz;
            sum += vec4_a.y * texture( texture_unit0, Y1.zw).xyz;
            sum += vec4_a.z * texture( texture_unit0, Y2.xy).xyz;
            sum += vec4_a.w * texture( texture_unit0, Y2.zw).xyz;
            vec4_w += vec4_a;
        }
        res = sum / dot(vec4_w, vec4(1.0));
    }
// Vectorized
#else
    {
        //_float4 vec4_length_vx = _float4(length(unpackVec2FFromVec4(texture( texture_unit4, input.out_texcoord0))));
        _float4 vec4_length_vx = _float4(length(unpack_velocity( texture_unit4, input.out_texcoord0)));
        _float4 vec4_zx = _float4(-getLinearDepth(texture_unit1, input.out_texcoord0));
        _float4 vec4_x = input.out_texcoord0.xyxy;
        hfloat4 vec4_vn = v_n.xyxy;

        const _float p = 0.1;
        _float4 vec4_w = _float4(1.0 / (vec4_length_vx.x + p), 0.0, 0.0, 0.0);
        _float3 sum = 1.0 / (vec4_length_vx.x + p) * c0;

        debug_sample_count = SAMPLE_COUNT_DIV_4 * 4;
        for (int i = 0; i < SAMPLE_COUNT_DIV_4; i++)
        {
            hfloat4 vec4_t = vectorized_tap_offsets[i];

            hfloat4 Y1 = vec4_x + vec4_vn * vec4_t.xxyy;
            hfloat4 Y2 = vec4_x + vec4_vn * vec4_t.zzww;

            _float4 vec4_Z_Y = -_float4(
                getLinearDepth(texture_unit1, Y1.xy),
                getLinearDepth(texture_unit1, Y1.zw),
                getLinearDepth(texture_unit1, Y2.xy),
                getLinearDepth(texture_unit1, Y2.zw));

            /*
            _float4 vec4_length_vy = _float4(
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.zw))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.zw))));
            */
            _float4 vec4_length_vy = _float4(
                length(unpack_velocity( texture_unit4, Y1.xy)),
                length(unpack_velocity( texture_unit4, Y1.zw)),
                length(unpack_velocity( texture_unit4, Y2.xy)),
                length(unpack_velocity( texture_unit4, Y2.zw)));

            hfloat4 tmp1 = vec4_x - Y1;
            hfloat4 tmp2 = vec4_x - Y2;
            _float4 vec4_length_xy = vec4(length(tmp1.xy), length(tmp1.zw), length(tmp2.xy), length(tmp2.zw));

            _float4 vec4_f = softDepthCompare4(vec4_zx, vec4_Z_Y);
            _float4 vec4_b = softDepthCompare4(vec4_Z_Y, vec4_zx);

            _float4 vec4_a = vec4_f * cone4(vec4_length_xy, vec4_length_vy);
            vec4_a += vec4_b * cone4(vec4_length_xy, vec4_length_vx);
            vec4_a += cylinder4(vec4_length_xy, vec4_length_vy) * cylinder4(vec4_length_xy, vec4_length_vx) * 2.0;

            // Weighted sum of the colors
            sum += vec4_a.x * texture_unit0.sample(sampler0, Y1.xy).xyz;
            sum += vec4_a.y * texture_unit0.sample(sampler0, Y1.zw).xyz;
            sum += vec4_a.z * texture_unit0.sample(sampler0, Y2.xy).xyz;
            sum += vec4_a.w * texture_unit0.sample(sampler0, Y2.zw).xyz;
            vec4_w += vec4_a;
        }
        res = sum / dot(vec4_w, vec4(1.0));
    }
#endif // Vectorized

#ifdef DRAW_GS_WIREFRAME
    res = c0;
#endif

    //res = _float3(1.0) * _float(debug_sample_count) / _float(SAMPLE_COUNT);

    // NOTE: Comment this back to disable motion blur
    //res = c0.xyz;
    return _float4(res, _float(1.0));
}

#endif

#ifdef TYPE_vertex

vertex VertexOutput shader_main(VertexInput input [[stage_in]])
{
    VertexOutput output;

    output.out_position = _float4(input.in_position, 1.0);
    output.out_texcoord0 = input.in_position.xy * _float2(0.5, -0.5) + 0.5;

    return output;
}

#endif

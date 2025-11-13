/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

#include "mb_common.h"

#define EPS 0.0001
#define SOFT_Z_EXTENT_INV 10.0

#define SAMPLE_COUNT_DIV_4 (SAMPLE_COUNT/4)

uniform highp vec4 vectorized_tap_offsets[SAMPLE_COUNT_DIV_4];
uniform highp float tap_offsets[SAMPLE_COUNT];

uniform mediump int vectorized_samples[SAMPLE_COUNT];

in vec2 out_texcoord0;
out vec3 frag_color;

float softDepthCompare(float z_a, float z_b)
{
  return clamp(1.0 - (z_a - z_b) * SOFT_Z_EXTENT_INV, 0.0, 1.0);
}

float cone(float length_xy, float length_v)
{
    return clamp(1.0 - length_xy/(length_v + 0.001), 0.0, 1.0);
}

float cylinder(float length_xy, float length_v)
{
   return 1.0 - smoothstep(0.95 * length_v, 1.05 * length_v, length_xy);
}

// Vectorized version
vec4 softDepthCompare4(vec4 z_a, vec4 z_b)
{
  return clamp(vec4(1.0) - (z_a - z_b) * SOFT_Z_EXTENT_INV, vec4(0.0), vec4(1.0));
}

vec4 cone4(vec4 length_xy, vec4 length_v)
{
    return clamp(vec4(1.0) - length_xy/(length_v + 0.001), vec4(0.0), vec4(1.0));
}

vec4 cylinder4(vec4 length_xy, vec4 length_v)
{
   return vec4(1.0) - smoothstep(0.95 * length_v, 1.05 * length_v, length_xy);
}

// MODE indicates the algorithm
// 0 - Adaptive
// 1 - Vectorized adaptive
// 2 - Vectorized

// texture_unit0 - color
// texture_unit1 - depth
// texture_unit4 - velocity buffer
// texture_unit6 - neighbor max
void main()
{
    int debug_sample_count = 0;

    vec3 res = vec3(0.0, 0.0, 0.0);

    vec3 c0 = texture( texture_unit0, out_texcoord0).xyz;  // color

#ifdef NEIGHTBOR_TEXTURE_RGBA8
    highp vec2 v_n = unpackVec2FFromVec4Highp(texture( texture_unit6, out_texcoord0)); // neighbor max
#else
    highp vec2 v_n = texture( texture_unit6, out_texcoord0).xy; // neighbor max
#endif

    float length_vn = length(v_n);
    if ( length_vn < (EPS + 0.5 / float(VP_WIDTH)) )
    {
        res = c0;
    }
    else
// Adaptive
#if MODE == 0
    {
        float Z_X = -getLinearDepth(texture_unit1, out_texcoord0);
        //float length_vx = length(unpackVec2FFromVec4(texture( texture_unit4, out_texcoord0)));
        float length_vx = length(unpack_velocity( texture_unit4, out_texcoord0));
        vec2 X = out_texcoord0;

        const float p = 0.1;
        float w = 1.0 / (length_vx + p);
        vec3 sum = 1.0 / (length_vx + p) * c0;

        //int sample_count = int (length_vn / mb_velocity_min_max_sfactor_pad.y * float(SAMPLE_COUNT));
        int sample_count = int(length_vn * mb_velocity_min_max_sfactor_pad.z);
        sample_count = clamp(sample_count, 1, SAMPLE_COUNT);

        debug_sample_count = sample_count;
        for (int i = 0; i < sample_count; i++)
        {
            float t = tap_offsets[i];

            highp vec2 Y = X + t * v_n;

            float Z_Y = -getLinearDepth(texture_unit1, Y);
            //vec2 V_Y = unpackVec2FFromVec4(texture( texture_unit4, Y));
            highp vec2 V_Y = unpack_velocity( texture_unit4, Y);
            float length_vy = length(V_Y);
            float length_xy = length(X-Y);

            float f = softDepthCompare(Z_X, Z_Y);
            float b = softDepthCompare(Z_Y, Z_X);

            float a = f * cone(length_xy, length_vy);
            a += b * cone(length_xy, length_vx);
            a += cylinder(length_xy, length_vy) * cylinder(length_xy, length_vx) * 2.0;
            w += a;

            sum += a * texture( texture_unit0, Y).xyz;
        }
        res = sum / w;
    }

//  res = vec3(normalize(unpackVec2FFromVec4(texture( texture_unit4, out_texcoord0))), 0.0);
//  res = vec3(normalize(v_n), 0.0);

// Vectorized adaptive
#elif MODE == 1
    {
        //vec4 vec4_length_vx = vec4(length(unpackVec2FFromVec4(texture( texture_unit4, out_texcoord0))));
        vec4 vec4_length_vx = vec4(length(unpack_velocity(texture_unit4, out_texcoord0)));
        vec4 vec4_zx = vec4(-getLinearDepth(texture_unit1, out_texcoord0));
        vec4 vec4_x = out_texcoord0.xyxy;
        highp vec4 vec4_vn = v_n.xyxy;

        const float p = 0.1;
        vec4 vec4_w = vec4(1.0 / (vec4_length_vx.x + p), 0.0, 0.0, 0.0);
        vec3 sum = 1.0 / (vec4_length_vx.x + p) * c0;

        // TODO: this can be pre-calculated on the CPU
        int sample_count = int (vec4_length_vx.x * mb_velocity_min_max_sfactor_pad.z);
        sample_count = (sample_count + 2) / 4;
        sample_count = clamp(sample_count, 1, SAMPLE_COUNT_DIV_4);

        //sample_count = SAMPLE_COUNT_DIV_4;

        debug_sample_count = sample_count * 4;
        for (int i = 0; i < sample_count; i++)
        {
            highp vec4 vec4_t = vectorized_tap_offsets[i];

            highp vec4 Y1 = vec4_x + vec4_vn * vec4_t.xxyy;
            highp vec4 Y2 = vec4_x + vec4_vn * vec4_t.zzww;

            vec4 vec4_Z_Y = -vec4(
                getLinearDepth(texture_unit1, Y1.xy),
                getLinearDepth(texture_unit1, Y1.zw),
                getLinearDepth(texture_unit1, Y2.xy),
                getLinearDepth(texture_unit1, Y2.zw));

            /*
            vec4 vec4_length_vy = vec4(
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.zw))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.zw))));
            */
            vec4 vec4_length_vy = vec4(
                length(unpack_velocity( texture_unit4, Y1.xy)),
                length(unpack_velocity( texture_unit4, Y1.zw)),
                length(unpack_velocity( texture_unit4, Y2.xy)),
                length(unpack_velocity( texture_unit4, Y2.zw)));

            highp vec4 tmp1 = vec4_x - Y1;
            highp vec4 tmp2 = vec4_x - Y2;
            vec4 vec4_length_xy = vec4(length(tmp1.xy), length(tmp1.zw), length(tmp2.xy), length(tmp2.zw));

            vec4 vec4_f = softDepthCompare4(vec4_zx, vec4_Z_Y);
            vec4 vec4_b = softDepthCompare4(vec4_Z_Y, vec4_zx);

            vec4 vec4_a = vec4_f * cone4(vec4_length_xy, vec4_length_vy);
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
        //vec4 vec4_length_vx = vec4(length(unpackVec2FFromVec4(texture( texture_unit4, out_texcoord0))));
        vec4 vec4_length_vx = vec4(length(unpack_velocity( texture_unit4, out_texcoord0)));
        vec4 vec4_zx = vec4(-getLinearDepth(texture_unit1, out_texcoord0));
        vec4 vec4_x = out_texcoord0.xyxy;
        highp vec4 vec4_vn = v_n.xyxy;

        const float p = 0.1;
        vec4 vec4_w = vec4(1.0 / (vec4_length_vx.x + p), 0.0, 0.0, 0.0);
        vec3 sum = 1.0 / (vec4_length_vx.x + p) * c0;

        debug_sample_count = SAMPLE_COUNT_DIV_4 * 4;
        for (int i = 0; i < SAMPLE_COUNT_DIV_4; i++)
        {
            highp vec4 vec4_t = vectorized_tap_offsets[i];

            highp vec4 Y1 = vec4_x + vec4_vn * vec4_t.xxyy;
            highp vec4 Y2 = vec4_x + vec4_vn * vec4_t.zzww;

            vec4 vec4_Z_Y = -vec4(
                getLinearDepth(texture_unit1, Y1.xy),
                getLinearDepth(texture_unit1, Y1.zw),
                getLinearDepth(texture_unit1, Y2.xy),
                getLinearDepth(texture_unit1, Y2.zw));

            /*
            vec4 vec4_length_vy = vec4(
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y1.zw))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.xy))),
                length(unpackVec2FFromVec4(texture( texture_unit4, Y2.zw))));
            */
            vec4 vec4_length_vy = vec4(
                length(unpack_velocity( texture_unit4, Y1.xy)),
                length(unpack_velocity( texture_unit4, Y1.zw)),
                length(unpack_velocity( texture_unit4, Y2.xy)),
                length(unpack_velocity( texture_unit4, Y2.zw)));

            highp vec4 tmp1 = vec4_x - Y1;
            highp vec4 tmp2 = vec4_x - Y2;
            vec4 vec4_length_xy = vec4(length(tmp1.xy), length(tmp1.zw), length(tmp2.xy), length(tmp2.zw));

            vec4 vec4_f = softDepthCompare4(vec4_zx, vec4_Z_Y);
            vec4 vec4_b = softDepthCompare4(vec4_Z_Y, vec4_zx);

            vec4 vec4_a = vec4_f * cone4(vec4_length_xy, vec4_length_vy);
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
#endif // Vectorized

#ifdef DRAW_GS_WIREFRAME
    res = c0;
#endif

    //res = vec3(1.0) * float(debug_sample_count) / float(SAMPLE_COUNT);

    // NOTE: Comment this back to disable motion blur
    //res = c0.xyz;
    frag_color = res;
}

#endif

#ifdef TYPE_vertex

in vec3 in_position;
out vec2 out_texcoord0;

void main()
{
    gl_Position = vec4( in_position, 1.0);
    out_texcoord0 = in_position.xy * 0.5 + 0.5;
}

#endif

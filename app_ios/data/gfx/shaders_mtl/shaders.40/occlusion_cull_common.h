/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// http://rastergrid.com/blog/2010/10/hierarchical-z-map-based-occlusion-culling/
// http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2013/01/Chapter03-SBOT-March_of_The_Froblins.pdf

bool IsVisible(constant OcclusionConstants * consts, depth2d<hfloat> hiz_texture, _float2 screen_min, _float2 screen_max, hfloat depth)
{
    // NDC->Window space
    _float view_size_x = (screen_max.x-screen_min.x) * consts->view_port_size.x;
    _float view_size_y = (screen_max.y-screen_min.y) * consts->view_port_size.y;

    _float mipmap_level = ceil(log2(max(view_size_x, view_size_y) * 0.5));

    constexpr sampler depth_sampler(coord::normalized, filter::nearest, mip_filter::nearest, address::clamp_to_edge);

    hfloat4 samples;
    samples.x = hiz_texture.sample(depth_sampler, hfloat2(screen_min.x, 1.0 - screen_min.y), level(mipmap_level));
    samples.y = hiz_texture.sample(depth_sampler, hfloat2(screen_min.x, 1.0 - screen_max.y), level(mipmap_level));
    samples.z = hiz_texture.sample(depth_sampler, hfloat2(screen_max.x, 1.0 - screen_max.y), level(mipmap_level));
    samples.w = hiz_texture.sample(depth_sampler, hfloat2(screen_max.x, 1.0 - screen_min.y), level(mipmap_level));
    hfloat max_depth = max( max( samples.x, samples.y ), max( samples.z, samples.w ) );

    return depth < max_depth;
}

bool IsVisible(constant OcclusionConstants * consts, depth2d<hfloat> hiz_texture, const device hfloat4 * aabb)
{
    _float4 bounding_box[8];
    for (int i = 0; i < 8; i++)
    {
        // Project the AABB vertex
        bounding_box[i] = _float4(consts->vp * aabb[i]);
        bounding_box[i].xyz /= bounding_box[i].w;

        // After the frustum cull the AABB is in the view frustum.
        // If it has a point behind the camera or behind the near plane it is visiblew  w
        if (bounding_box[i].z < 0.0 || bounding_box[i].z > consts->near_far_ratio)
        {
            return true;
        }
    }

    _float depth = min(min(min(bounding_box[0].z, bounding_box[1].z ),
                              min( bounding_box[2].z, bounding_box[3].z ) ),
                              min( min( bounding_box[4].z, bounding_box[5].z ),
                              min( bounding_box[6].z, bounding_box[7].z ) ) );

    _float2 screen_min;
    _float2 screen_max;
    screen_min.x = min( min( min( bounding_box[0].x, bounding_box[1].x ),
                                  min( bounding_box[2].x, bounding_box[3].x ) ),
                             min( min( bounding_box[4].x, bounding_box[5].x ),
                                  min( bounding_box[6].x, bounding_box[7].x ) ) ) * 0.5 + 0.5;
    screen_min.y = min( min( min( bounding_box[0].y, bounding_box[1].y ),
                                  min( bounding_box[2].y, bounding_box[3].y ) ),
                             min( min( bounding_box[4].y, bounding_box[5].y ),
                                  min( bounding_box[6].y, bounding_box[7].y ) ) ) * 0.5 + 0.5;
    screen_max.x = max( max( max( bounding_box[0].x, bounding_box[1].x ),
                                  max( bounding_box[2].x, bounding_box[3].x ) ),
                             max( max( bounding_box[4].x, bounding_box[5].x ),
                                  max( bounding_box[6].x, bounding_box[7].x ) ) ) * 0.5 + 0.5;
    screen_max.y = max( max( max( bounding_box[0].y, bounding_box[1].y ),
                                  max( bounding_box[2].y, bounding_box[3].y ) ),
                             max( max( bounding_box[4].y, bounding_box[5].y ),
                                  max( bounding_box[6].y, bounding_box[7].y ) ) ) * 0.5 + 0.5;

    // NDC->Window space
    return IsVisible(consts, hiz_texture, screen_min, screen_max, depth);
}

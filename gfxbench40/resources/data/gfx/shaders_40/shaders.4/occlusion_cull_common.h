/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
// http://rastergrid.com/blog/2010/10/hierarchical-z-map-based-occlusion-culling/
// http://amd-dev.wpengine.netdna-cdn.com/wordpress/media/2013/01/Chapter03-SBOT-March_of_The_Froblins.pdf

bool IsVisible(vec2 screen_min, vec2 screen_max, highp float depth)
{
    // NDC->Window space
    float view_size_x = (screen_max.x-screen_min.x) * view_port_size.x;
    float view_size_y = (screen_max.y-screen_min.y) * view_port_size.y;

    float mipmap_level = ceil(log2(max(view_size_x, view_size_y) * 0.5));

    highp vec4 samples;
    samples.x = textureLod(hiz_texture, vec2(screen_min.x, screen_min.y), mipmap_level).x;
    samples.y = textureLod(hiz_texture, vec2(screen_min.x, screen_max.y), mipmap_level).x;
    samples.z = textureLod(hiz_texture, vec2(screen_max.x, screen_max.y), mipmap_level).x;
    samples.w = textureLod(hiz_texture, vec2(screen_max.x, screen_min.y), mipmap_level).x;
    highp float max_depth = max( max( samples.x, samples.y ), max( samples.z, samples.w ) );

    return depth < max_depth;
}

bool IsVisible(highp vec4 aabb[8])
{
    vec4 bounding_box[8];
    for (int i = 0; i < 8; i++)
    {
        // Project the AABB vertex
        bounding_box[i] = vp * aabb[i];
        bounding_box[i].xyz /= bounding_box[i].w;

        // After the frustum cull the AABB is in the view frustum.
        // If it has a point behind the camera or behind the near plane it is visiblew  w
        if (bounding_box[i].z < -1.0 || bounding_box[i].z > near_far_ratio)
        {
            return true;
        }
    }

    float depth = min(min(min(bounding_box[0].z, bounding_box[1].z ),
                              min( bounding_box[2].z, bounding_box[3].z ) ),
                              min( min( bounding_box[4].z, bounding_box[5].z ),
                              min( bounding_box[6].z, bounding_box[7].z ) ) );

    depth = depth * 0.5 + 0.5;

    vec2 screen_min;
    vec2 screen_max;
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
    return IsVisible(screen_min, screen_max, depth);
}

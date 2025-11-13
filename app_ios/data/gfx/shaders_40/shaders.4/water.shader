/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"
#include "attribute_locations.h"

#ifndef G_BUFFER_PASS
#error - CHECK SHADER DEFINES GIVEN TO COMPILER
#endif

#ifdef TYPE_vertex

/***********************************************************************************/
//                          VERTEX SHADER
/***********************************************************************************/

layout(location = ATTR_POSITION_LOCATION)  in vec3 in_position;
layout(location = ATTR_TEXCOORD0_LOCATION) in vec2 in_texcoord0;
layout(location = ATTR_TEXCOORD1_LOCATION) in vec2 in_texcoord1;
layout(location = ATTR_NORMAL_LOCATION)    in vec3 in_normal;
layout(location = ATTR_TANGENT_LOCATION)   in vec3 in_tangent;

out vec2 out_texcoord0;
out vec2 out_texcoord1;
out vec3 out_normal;
out vec3 out_tangent;
out vec3 out_worldpos;

#ifdef VELOCITY_BUFFER
    out vec4 out_scPos;
    out vec4 out_prevScPos;
#endif

void main()
{
    vec3 normal = in_normal;
    vec3 tangent = in_tangent;
    decodeFromByteVec3(normal);
    decodeFromByteVec3(tangent);

    vec4 _scPos = mvp * vec4( in_position, 1.0);
    vec4 world_pos = model * vec4( in_position, 1.0);

    out_worldpos = world_pos.xyz;
    gl_Position = _scPos;

#ifdef VELOCITY_BUFFER
    out_scPos = _scPos;
    out_prevScPos = mvp2 * vec4(in_position, 1.0);
#endif

    vec4 tmp;
    tmp = vec4(normal, 0.0) * inv_model;
    out_normal = tmp.xyz;

    tmp = vec4(tangent, 0.0) * inv_model;
    out_tangent = tmp.xyz;

    out_texcoord0 = in_texcoord0;
    out_texcoord1 = in_texcoord1;
}

#endif

#ifdef TYPE_fragment

/***********************************************************************************/
//                          FRAGMENT SHADER
/***********************************************************************************/

highp in vec2 out_texcoord0; // water shape uv coordinates are bigger than 0...1
in vec2 out_texcoord1;
in vec3 out_normal;
in vec3 out_tangent;
in vec3 out_worldpos;

layout(location = 0) out vec4 frag_color;

#ifdef VELOCITY_BUFFER
layout(location = 1) out VELOCITY_BUFFER_TYPE velocity;
in vec4 out_scPos;
in vec4 out_prevScPos;
#endif

layout(location = 2) out vec4 frag_normal;
layout(location = 3) out vec4 frag_params;

void main()
{
    // View direction
    vec3 V = normalize(view_pos - out_worldpos);

    // Mix the normal vector with some wave like noise
    float time = time_dt_pad2.x * 0.04;
    vec3 tex_normal = texture(texture_unit3, out_texcoord0 + vec2(0.0, -time)).xyz;
    vec3 tex_normal2 = texture(texture_unit3, out_texcoord0 + vec2(-time, 0.0)).xyz;
    tex_normal = (tex_normal + tex_normal2) * 0.5;
    vec3 N = normalize(calcWorldNormal(tex_normal, out_normal, out_tangent));
    vec3 NV = normalize(vec4(N, 0) * inv_view).xyz; //view-space normals

    vec3 refract_dir = refract(-V, N, 1.01);
    float refract_factor = dot(refract_dir, -N);

    // Texture coordinates of the sea floor depth texture
    // 1950 is the half size of the scene
    /*highp*/ vec2 depth_uv = out_worldpos.xz / 1950.0;
    depth_uv += vec2(0.5, 0.5);
    float sea_depth = clamp(1.0 - texture( texture_unit6, depth_uv).z,0.0,1.0);

    // The colours of the ocean...
    const vec3 water_green = vec3(9.0, 143.0, 134.0) / 255.0;
    const vec3 water_albedo = vec3(52.0, 98.0, 140.0) / 255.0 / 4.0;
    vec3 seafloor = texture( texture_unit5, out_texcoord0).xyz;

    // Mix the colours of the ocean...
    vec3 bikini = mix(seafloor, water_green, sea_depth);
    vec3 albedo = mix(bikini, water_albedo, sea_depth);
    albedo = mix(albedo, water_albedo, 1.0 - refract_factor);

    frag_color = vec4(albedo, 0.0); //albedoRGB, emissives
    frag_normal = encodeNormal(NV); //encoded view normal
    frag_params = vec4((1.0 - dot(N, V)) * 0.5, 100.0 / 255.0, 1.0 , 1.0);  //specCol, envmapIdx, smoothness, ao
#ifdef EDITOR_MODE
    if (editor_mesh_selected == 1)
    {
        frag_color = vec4(albedo * editor_select_color , 0.0);
        frag_normal = encodeNormal(NV);
        frag_params = vec4(0.0, 0.0, 0.0, 1.0);
    }
#endif

#ifdef VELOCITY_BUFFER
    //velocity = pack2FloatToVec4(velocityFunction(out_scPos,out_prevScPos));
    velocity = pack_velocity(velocityFunction(out_scPos,out_prevScPos));
#endif

}

#endif // END OF FRAG SHADER

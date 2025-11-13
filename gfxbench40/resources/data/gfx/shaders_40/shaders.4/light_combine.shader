/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "common.h"

#ifdef TYPE_fragment

uniform vec4 carindex_translucency_ssaostr_fovscale;

highp in vec2 out_texcoord0;
out vec4 frag_color;

//TODO: channel view based on user input
//TODO: reflections with indirect reads - write refl.idx into G-Buffer a switch between
//		runtime updated and static baked ones here

/*
	Inputs:
		0 Albedo 	//albedoRGB, emissive
		1 Normals 	//encoded view space normal
		2 Params 	//specCol, smoothness
		3 Depth
		4 Shadows	//blurred SSAO, blurred dynamic shadow
		+ Envmaps
*/

void main()
{
	//sample
	vec4 albedo_emissive_translucency = texture(texture_unit0, out_texcoord0);
	highp vec3 normalVS = decodeNormal(texture_unit1, out_texcoord0);
	vec4 specCol_envmapIdx_smoothness_ao = texture(texture_unit2, out_texcoord0); //specCol, envmapIdx, smoothness, ao

	vec2 dynAO_Shadow = texture(texture_unit4, out_texcoord0).xy;
	//float dynAO = max(dynAO_Shadow.x, carindex_translucency_ssaostr_fovscale.z);
	float dynAO = dynAO_Shadow.x;
	float dynShadow = dynAO_Shadow.y;

	//decode
	vec3 albedo = albedo_emissive_translucency.xyz;
	float emissive_translucency = albedo_emissive_translucency.w;

	highp vec3 normalWS = normalize(inv_view * vec4(normalVS,0.0)).xyz;
	//normalWS = normalize(albedo * 2.0 - 1.0);

	float specCol = specCol_envmapIdx_smoothness_ao.x;
	float roughness = 1.0 - specCol_envmapIdx_smoothness_ao.z;

	float staticAO = specCol_envmapIdx_smoothness_ao.w;

	float shadow = dynShadow * dynAO; //HACK: until we spray negative point-lights for every bush, this darkens their core

	//combine
	highp vec3 originWS = getPositionWS(texture_unit3, out_texcoord0); //reads depth
	highp vec3 view_dir = view_pos - originWS;

	float cubeIndex = specCol_envmapIdx_smoothness_ao.y * 255.0;

	vec3 result_color = getPBRlighting(view_dir, normalWS, roughness, cubeIndex, specCol, albedo, emissive_translucency, staticAO, dynAO, shadow, originWS);

	highp vec3 res = applyFog(result_color, length(view_dir), normalize(view_dir));

#ifdef DEBUG_SHADOW
	vec4 shadow_coord; //not used
	int cascade_index = getMapBasedCascadeIndex(vec4(originWS, 1.0), shadow_coord);
	if (cascade_index = 0)
		res *= vec3(1.0, 0.2, 0.2);
	if (cascade_index = 1)
		res *= vec3(0.2, 1.0, 0.2);
	if (cascade_index = 2)
		res *= vec3(0.2, 0.2, 1.0);
	if (cascade_index = 3)
		res *= vec3(1.0, 0.2, 1.0);
#endif

	frag_color = RGBtoRGBD_lightcombine(res);
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

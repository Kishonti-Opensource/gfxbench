/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
uniform sampler2D color_texture;
uniform sampler2D specular_texture;
uniform sampler2D normal_texture;
uniform vec4 carindex_translucency_ssaostr_fovscale;

//in vec2 out_texcoord0;
//in vec2 out_texcoord1;
in vec3 out_normal;
in vec3 out_tangent;
in highp vec3 out_worldpos;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_specular;
layout(location = 2) out vec4 frag_normal;
layout(location = 3) out vec4 frag_params;
void main()
{
	vec4 albedo_extras = texture( color_texture, vec2(0.0));

	//NOTE: on desktop, DXT can produce artifacts with dark textures unless it is compressed with gamma
	//(relates to "greening" DXT compression artifacts)
#ifndef GL_ES
	//albedo_extras = pow(albedo_extras, gamma_exp);
#endif

	frag_color = vec4(albedo_extras);
	frag_specular = vec4(1.0);
	frag_normal = vec4(out_normal, 1.0);
	//frag_params = vec4(0.0);

/*
	vec3 albedo = albedo_extras.xyz;
	float baked_ao = 1.0;

	baked_ao = texture( texture_unit1, out_texcoord1).x;

	vec4 specCol_emissive = texture( texture_unit2, out_texcoord0);
	vec4 texNorm_smoothness = texture(texture_unit3, out_texcoord0);
	float specCol = specCol_emissive.y;
	float local_emissive = 0.0;
	float smoothness = texNorm_smoothness.w;
	vec3 texNorm = vec3(0.0);
	highp vec3 N = vec3(0.0);

	texNorm = texNorm_smoothness.xyz;
	N = calcWorldNormal(texNorm, out_normal, out_tangent);

	highp vec3 NV = normalize(vec4(N,0) * inv_view).xyz;
	highp vec3 view_dir = view_pos - out_worldpos;
	float ao = baked_ao;

	frag_color = vec4(albedo, 1.0);
	frag_normal = vec4(specCol, envmapIdx, smoothness, ao);
	//gl_FragData[2] = encodeNormal(NV);
	frag_normal = vec4(NV, 1.0);

	*/
	/*
	float emissive_translucency = carindex_translucency_ssaostr_fovscale.y;

	float envmapIdx = carindex_translucency_ssaostr_fovscale.x;

	gl_FragData[0] = vec4(albedo, emissive_translucency);
	gl_FragData[1] = vec4(specCol, envmapIdx, smoothness, ao);
	//gl_FragData[2] = encodeNormal(NV);
	gl_FragData[2] = vec4(NV, 1.0);
	*/
}

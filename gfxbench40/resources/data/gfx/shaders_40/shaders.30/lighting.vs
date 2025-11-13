/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifdef INSTANCING
#include cameraConsts;
#include lightInstancingConsts;

in vec3 in_position;
in vec2 in_texcoord0;

out vec4 out_view_dir;
out vec4 out_pos;
flat out int instanceID;

void main()
{
    instanceID = instance_offset + gl_InstanceID;

	mat4 mvp = vp * lights[instanceID].model;
	vec4 pos = mvp * vec4( in_position, 1.0);

	gl_Position = pos;
	out_pos = pos;

	pos = lights[instanceID].model * vec4( in_position, 1.0);
	
	out_view_dir.xyz = pos.xyz - view_posXYZ_normalized_time.xyz;
	out_view_dir.w = dot( view_dirXYZ_pad.xyz, out_view_dir.xyz);
}
#else //INSTANCING
#ifdef USE_UBOs
	#include cameraConsts;
	#include meshConsts;
	#include staticMeshConsts;
#else
	uniform mat4 mvp;
	uniform mat4 model;
	
	uniform vec3 view_pos;
	uniform vec3 view_dir;
#endif	

in vec3 in_position;
in vec2 in_texcoord0;

out vec4 out_view_dir;
out vec4 out_pos;

void main()
{
	//vec4 pos = mvp * vec4( in_position, 1.0);
	mat4 mvp_1 = mvp;
	vec4 pos = mvp_1 * vec4( in_position, 1.0);

	gl_Position = pos;
	
	out_pos = pos;
	
	//pos = model * vec4( in_position, 1.0);
	mat4 model_1 = model;
	pos = model_1 * vec4( in_position, 1.0);

#ifdef USE_UBOs
	out_view_dir.xyz = pos.xyz - view_posXYZ_normalized_time.xyz;
	out_view_dir.w = dot( view_dirXYZ_pad.xyz, out_view_dir.xyz);
#else
	out_view_dir.xyz = pos.xyz - view_pos;
	out_view_dir.w = dot( view_dir, out_view_dir.xyz);
#endif
}
#endif //INSTANCING

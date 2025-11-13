/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SHADER_CONSTANTS_40_H
#define MTL_SHADER_CONSTANTS_40_H


// MUST match declarations in shaders.40/common.h

#define VERTEX_CONSTANTS_SLOT		1
#define INSTANCE_CONSTANTS_SLOT		2
#define TESSELLATION_CONSTANTS_SLOT	3
#define USER_PER_PATCH_SLOT			4
#define INDEX_BUFFER_SLOT			6
#define VERTEX_BUFFER_SLOT			7
#define TESS_FACTORS_SLOT			8
#define CONTROL_DATA_SLOT			9
#define ORIGNAL_DRAWS_SLOT			10
#define DRAWS_SLOT 					11
#define DRAW_IDX_COUNT_SLOT			12
#define DRAW_IDX_OFFSET_SLOT        13
#define OCCLUSION_UNIFORMS_SLOT		14
#define COMMAND_IDX_SLOT            15
#define DRAW_PATCH_INDIRECT_SLOT    16

#define FRAME_CONSTANTS_SLOT		0
#define FRAGMENT_CONSTANTS_SLOT		1

#define FILTER_CONSTANTS_SLOT		1
#define LUMINANCE_BUFFER_SLOT		2

enum TextureBindings
{
	TEXTURE_UNIT0_SLOT = 0,
	TEXTURE_UNIT1_SLOT = 1,
	TEXTURE_UNIT2_SLOT = 2,
	TEXTURE_UNIT3_SLOT = 3,
	TEXTURE_UNIT4_SLOT = 4,
	TEXTURE_UNIT5_SLOT = 5,
	TEXTURE_UNIT6_SLOT = 6,
	TEXTURE_UNIT7_SLOT = 7,
	DEPTH_UNIT0_SLOT = 8,
	CASCADED_SHADOW_TEXTURE_ARRAY_SLOT = 9,
	ENVMAP1_DP_SLOT = 10,
	ENVMAP2_DP_SLOT = 11,
    HIZ_TEXTURE_SLOT = 12,
	STATIC_ENVMAPS_SLOT_0 = 13,
	STATIC_ENVMAPS_SLOT_1 = 14,
	STATIC_ENVMAPS_SLOT_2 = 15,
	STATIC_ENVMAPS_SLOT_3 = 16,
	STATIC_ENVMAPS_SLOT_4 = 17,
	STATIC_ENVMAPS_SLOT_5 = 18,
	STATIC_ENVMAPS_SLOT_6 = 19,
	STATIC_ENVMAPS_SLOT_7 = 20,
	STATIC_ENVMAPS_SLOT_8 = 21,
};


struct FrameConstants
{
	// DO NOT REORDER: SERIALIZED DATA!
	KCL::Vector4D time_dt_pad2;
	KCL::Vector4D global_light_dir;
	KCL::Vector4D global_light_color;
	KCL::Vector4D mb_velocity_min_max_sfactor_pad;
	KCL::Vector4D ABCD;
	KCL::Vector4D EFW_tau;
	KCL::Vector4D fogCol;
	KCL::Vector4D sky_color;
	KCL::Vector4D ground_color;
	KCL::Vector4D exposure_bloomthreshold_tone_map_white_pad;
	KCL::Vector4D ambient_colors[54];
};

struct VertexConstants
{
	KCL::Matrix4x4 view;
	KCL::Matrix4x4 vp;
	KCL::Matrix4x4 prev_vp;
	KCL::Matrix4x4 car_ao_matrix0;
	KCL::Matrix4x4 car_ao_matrix1;
	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 mvp2;
	KCL::Matrix4x4 mv;
	KCL::Matrix4x4 model;
	KCL::Matrix4x4 inv_model;
	KCL::Vector4D cam_near_far_pid_vpscale;
};

struct TessellationConstants
{
	KCL::Vector4D tessellation_factor;
	KCL::Vector4D frustum_planes[6];
	float tessellation_multiplier;
	float pad0[3];
};

struct FragmentConstants
{
	KCL::Matrix4x4 cascaded_shadow_matrices[4];
	KCL::Matrix4x4 inv_view;
	KCL::Matrix4x4 dpcam_view;
	KCL::Vector4D cascaded_frustum_distances;
	KCL::Vector4D gamma_exp;
	KCL::Vector4D carindex_translucency_ssaostr_fovscale;
	KCL::Vector3D view_pos;
	float pad0;
};

struct FilterConstants
{
	KCL::Matrix4x4 inv_view;
	KCL::Matrix4x4 dpcam_view;
	KCL::Matrix4x4 cascaded_shadow_matrices[4];
	KCL::Vector4D carindex_translucency_ssaostr_fovscale;
	KCL::Vector4D cascaded_frustum_distances;
	KCL::Vector4D depth_parameters;
	KCL::Vector4D corners[4];
	KCL::Vector3D view_pos;
	float pad0;
	KCL::Vector2D inv_resolution;
	KCL::Vector2D offset_2d;
	float camera_focus;
	float camera_focus_inv;
	float dof_strength;
	float dof_strength_inv;
	float gauss_lod_level;
	float projection_scale;
	float pad1[2];
};

struct SSAOConstants
{
	KCL::Vector4D depth_parameters;
	KCL::Vector4D corners[4];
	float projection_scale;
	float pad_0[3];
};

struct SSDSConstants
{
	KCL::Matrix4x4 cascaded_shadow_matrices[4];
	KCL::Vector4D cascaded_frustum_distances;
	KCL::Vector4D depth_parameters;
	KCL::Vector4D corners[4];
	KCL::Vector3D view_pos;
	float pad_0;
};

struct HDRConsts
{
	KCL::Vector4D ABCD;
	KCL::Vector4D EFW_tau;
	KCL::Vector4D exposure_bloomthreshold_minmax_lum;
};

struct MotionBlurConstants
{
    KCL::Vector4D mb_velocity_min_max_sfactor_pad;
    KCL::Vector4D depth_parameters;
};



struct LuminanceBuffer
{
	//.x - adaptive luminance
	//.y - current frame average luminance
	//.zw - padding
	KCL::Vector4D adaptive_avg_pad2;
};

struct OcclusionConstants
{
	KCL::Matrix4x4 vp;
	KCL::Vector2D view_port_size;
	KCL::uint32 id_count;
	KCL::uint32 instance_id_offset;
	KCL::uint32 frame_counter;
	float near_far_ratio;
	float pad_0[2];
};


#endif

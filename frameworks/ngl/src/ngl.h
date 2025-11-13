/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef NGL_H
#define NGL_H


#include <string.h>//memset
#include <string>
#include <vector>
#include <stdint.h>
#include "job_statistic.h"

#ifndef _LOGF
#define _LOGF
typedef void(*PFNLOGF) (const char *format, ...);
#endif
#define NGL_CUSTOM_ACTION_SWAPBUFFERS (~0U)
#define NGL_CUSTOM_ACTION_WAIT_FINISH (~0U - 1)
#define NGL_WAIT_FINISH_RT_WIDTH 1
#define NGL_WAIT_FINISH_RT_HEIGHT 1
#define nglDrawFrontSided(RENDER,SHADER_CODE,VBO,EBO,PARAMETERS) nglDraw(RENDER, NGL_TRIANGLES, SHADER_CODE, 1, &VBO, EBO, NGL_FRONT_SIDED, PARAMETERS);
#define nglDrawTwoSided(RENDER,SHADER_CODE,VBO,EBO,PARAMETERS) nglDraw(RENDER, NGL_TRIANGLES, SHADER_CODE, 1, &VBO, EBO, NGL_TWO_SIDED, PARAMETERS);
#define nglDrawBackSided(RENDER,SHADER_CODE,VBO,EBO,PARAMETERS) nglDraw(RENDER, NGL_TRIANGLES, SHADER_CODE, 1, &VBO, EBO, NGL_BACK_SIDED, PARAMETERS);
#define nglBlendState0(RENDER,BLEND) nglBlendState(RENDER, 0, BLEND, NGL_CHANNEL_ALL);
#define nglBlendStateAll(RENDER,BLEND) {for(uint32_t i=0; i<8; i++) nglBlendState(RENDER, i, BLEND, NGL_CHANNEL_ALL); }
#define nglGetApi() ((NGL_api)nglGetInteger(NGL_API))
#define nglGetMajor() (nglGetInteger(NGL_MAJOR_VERSION))
#define nglGetMinor() (nglGetInteger(NGL_MINOR_VERSION))

enum NGL_api
{
	NGL_OPENGL,
	NGL_OPENGL_ES,
	NGL_DIRECT3D_11,
	NGL_DIRECT3D_12,
	NGL_METAL_IOS,
	NGL_METAL_MACOS,
	NGL_VULKAN,
	NGL_NULL_DRIVER
};


enum NGL_rasterization_control_mode
{
	NGL_ORIGIN_LOWER_LEFT = 0,
	NGL_ORIGIN_UPPER_LEFT,
	NGL_ORIGIN_UPPER_LEFT_AND_NDC_FLIP,
};


enum NGL_depth_mode
{
	NGL_ZERO_TO_ONE = 0,
	NGL_NEGATIVE_ONE_TO_ONE
};


enum NGL_backend_property
{
	NGL_API = 0,
	NGL_MAJOR_VERSION,
	NGL_MINOR_VERSION,
	NGL_VENDOR,
	NGL_RENDERER,
	NGL_VERSION,
	NGL_RASTERIZATION_CONTROL_MODE,
	NGL_DEPTH_MODE,
	NGL_NEED_SWAPBUFFERS,
	NGL_TESSELLATION,
	NGL_PIPELINE_STATISTICS,
	NGL_FLOATING_POINT_RENDERTARGET,
	NGL_TEXTURE_COMPRESSION_ASTC,
	NGL_TEXTURE_COMPRESSION_DXT1,
	NGL_TEXTURE_COMPRESSION_DXT5,
	NGL_TEXTURE_COMPRESSION_ETC1,
	NGL_TEXTURE_COMPRESSION_ETC2,
	NGL_TEXTURE_MAX_ANISOTROPY,
	NGL_TEXTURE_MAX_SIZE_2D,
	NGL_TEXTURE_MAX_SIZE_CUBE,
	NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X,
	NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y,
	NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z,
	NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS,
	NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE,
	NGL_SUBPASS_ENABLED,
	NGL_D16_LINEAR_SHADOW_FILTER,
	NGL_D24_LINEAR_SHADOW_FILTER,
	NGL_PIPELINE_MAX_PUSH_CONSTANT_SIZE,
	//-1 - unknown format->call nglGetString
	NGL_SWAPCHAIN_COLOR_FORMAT,
	NGL_SWAPCHAIN_DEPTH_FORMAT,
	NGL_NUM_PROPERTIES
};


enum NGL_format
{
	NGL_UNDEFINED = 0,
	NGL_R8_UINT,
	NGL_R8_G8_UINT,
	NGL_R8_G8_B8_UINT,
	NGL_R8_G8_B8_A8_UINT,
	NGL_R10_G10_B10_A2_UINT,
	NGL_R8_UNORM,
	NGL_R8_G8_UNORM,
	NGL_R8_G8_B8_UNORM,
	NGL_R8_G8_B8_UNORM_SRGB,
	NGL_R8_G8_B8_A8_UNORM,
	NGL_R8_G8_B8_A8_UNORM_SRGB,
	NGL_R10_G10_B10_A2_UNORM,
	NGL_R16_UINT,
	NGL_R32_UINT,
	NGL_R32_FLOAT,
	NGL_R16_FLOAT,
	NGL_R32_G32_FLOAT,
	NGL_R32_G32_B32_FLOAT,
	NGL_R32_G32_B32_A32_FLOAT,
	NGL_R16_G16_FLOAT,
	NGL_R16_G16_B16_FLOAT,
	NGL_R16_G16_B16_A16_FLOAT,
	NGL_R11_B11_B10_FLOAT,
	NGL_D16_UNORM,
	NGL_D24_UNORM,
	NGL_D32_UNORM,
	NGL_R9_G9_B9_E5_SHAREDEXP,
	NGL_R8_G8_B8_ETC2_UNORM,
	NGL_R8_G8_B8_ETC2_UNORM_SRGB,
	NGL_R8_G8_B8_A1_ETC2_UNORM,
	NGL_R8_G8_B8_A1_ETC2_UNORM_SRGB,
	NGL_R8_G8_B8_A8_ETC2_UNORM,
	NGL_R8_G8_B8_A8_ETC2_UNORM_SRGB,
	NGL_R8_G8_B8_DXT1_UNORM,
	NGL_R8_G8_B8_A1_DXT1_UNORM,
	NGL_R8_G8_B8_DXT1_UNORM_SRGB,
	NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB,
	NGL_R8_G8_B8_A8_DXT5_UNORM,
	NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB,
	NGL_R8_G8_B8_A8_ASTC_4x4_UNORM,
	NGL_R8_G8_B8_A8_ASTC_4x4_UNORM_SRGB,
	NGL_R8_G8_B8_A8_ASTC_6x6_UNORM,
	NGL_R8_G8_B8_A8_ASTC_6x6_UNORM_SRGB
};


enum NGL_texture_type
{
	NGL_TEXTURE_2D = 0,
	NGL_TEXTURE_CUBE,
	NGL_TEXTURE_2D_ARRAY,
	NGL_RENDERBUFFER
};


enum NGL_texture_filter
{
	NGL_NEAREST = 0,
	NGL_LINEAR,
	NGL_NEAREST_MIPMAPPED,
	NGL_LINEAR_MIPMAPPED,
	NGL_ANISO_4,
};


enum NGL_texture_wrap_mode
{
	NGL_REPEAT_ALL = 0,
	NGL_CLAMP_ALL,
};


enum NGL_cull_mode
{
	NGL_FRONT_SIDED = 0,
	NGL_BACK_SIDED,
	NGL_TWO_SIDED
};


enum NGL_shader_type
{
	NGL_VERTEX_SHADER = 0,
	NGL_FRAGMENT_SHADER,
	NGL_GEOMETRY_SHADER,
	NGL_TESS_CONTROL_SHADER,
	NGL_TESS_EVALUATION_SHADER,
	NGL_COMPUTE_SHADER,
	NGL_NUM_SHADER_TYPES,
	NGL_NUM_CLASSIC_SHADER_TYPES = NGL_GEOMETRY_SHADER
};


enum NGL_shader_uniform_format
{
	NGL_FLOAT16 = 0,
	NGL_FLOAT4,
	NGL_TEXTURE,
	NGL_TEXTURE_SUBRESOURCE,
	NGL_FLOAT2,
	NGL_FLOAT,
	NGL_INT,
	NGL_INT2,
	NGL_INT4,
	NGL_UINT,
	NGL_UINT2,
	NGL_UINT4,
	NGL_BUFFER,
	NGL_BUFFER_SUBRESOURCE,
	NGL_UNIFORM_FORMAT_UNDEFINED
};


enum NGL_blend_func
{
	NGL_BLEND_DISABLED = 0,
	NGL_BLEND_ADDITIVE,
	NGL_BLEND_ALFA,
	NGL_BLEND_DECAL,
	NGL_BLEND_MODULATIVE,
	NGL_BLEND_ADDITIVE_ALFA,
	NGL_BLEND_ADDITIVE_INVERSE_ALFA,
	NGL_BLEND_TRANSPARENT_ACCUMULATION,
};


enum NGL_color_channel_mask
{
	NGL_CHANNEL_R = (1 << 0),
	NGL_CHANNEL_G = (1 << 1),
	NGL_CHANNEL_B = (1 << 2),
	NGL_CHANNEL_A = (1 << 3),
	NGL_CHANNEL_ALL = NGL_CHANNEL_R | NGL_CHANNEL_G | NGL_CHANNEL_B | NGL_CHANNEL_A,
	NGL_CHANNEL_NONE = 0
};


enum NGL_depth_func
{
	NGL_DEPTH_DISABLED = 0,
	NGL_DEPTH_LESS,
	NGL_DEPTH_LEQUAL,
	NGL_DEPTH_EQUAL,
	NGL_DEPTH_GREATER,
	NGL_DEPTH_TO_FAR,
	NGL_DEPTH_LESS_WITH_OFFSET,
	NGL_DEPTH_ALWAYS,
};


enum NGL_primitive_type
{
	NGL_POINTS = 0,
	NGL_LINES,
	NGL_TRIANGLES,
	NGL_PATCH3,
	NGL_PATCH4,
	NGL_PATCH16,
};


struct NGL_vertex_attrib
{
	std::string m_semantic;
	NGL_format m_format;
	uint32_t m_offset;

	NGL_vertex_attrib() : m_format(NGL_UNDEFINED), m_offset(0)
	{
	}
};


struct NGL_vertex_descriptor
{
	std::vector<NGL_vertex_attrib> m_attribs;
	uint32_t m_stride;
	bool m_unordered_access;

	NGL_vertex_descriptor() : m_stride(0), m_unordered_access(false)
	{
	}
};


enum NGL_shader_uniform_group
{
	NGL_GROUP_PER_DRAW = (1 << 0),
	NGL_GROUP_PER_RENDERER_CHANGE = (1 << 1),
	NGL_GROUP_MANUAL = (1 << 2),
};


struct NGL_shader_uniform
{
	std::string m_name;
	NGL_shader_uniform_format m_format;
	uint32_t m_size;
	NGL_shader_uniform_group m_group;

	NGL_shader_uniform()
		: m_format(NGL_UNIFORM_FORMAT_UNDEFINED)
		, m_size(0)
		, m_group(NGL_GROUP_PER_DRAW)
	{
	}

	NGL_shader_uniform(const char *name, NGL_shader_uniform_group group, NGL_shader_uniform_format format = NGL_UNIFORM_FORMAT_UNDEFINED)
	{
		m_name = name;
		m_group = group;
		m_size = 0;
		m_format = format;
	}
};


struct NGL_shader_source_descriptor
{
	std::vector<uint8_t> m_binary_data;
	std::string m_source_data;
	std::string m_entry_point;
	std::string m_version;
	std::string m_info_string;
	uint32_t m_work_group_size[3];

	std::vector<NGL_vertex_attrib> m_used_vertex_attribs;
	std::vector<NGL_shader_uniform> m_used_uniforms;
	std::vector<NGL_shader_uniform> m_used_uniform_textures;
	std::vector<NGL_shader_uniform> m_used_uniform_buffers;
	std::vector<NGL_shader_uniform> m_used_readonly_buffers;
	std::vector<NGL_shader_uniform> m_used_readonly_images;

	NGL_shader_source_descriptor()
	{
		memset(m_work_group_size, 0, 3 * sizeof(uint32_t));
	}
};


struct NGL_buffer_subresource
{
	uint32_t m_buffer;
	uint32_t m_offset;
	uint32_t m_size;
	NGL_buffer_subresource(uint32_t idx, uint32_t offset = 0, uint32_t size = (uint32_t)~0) : m_buffer(idx), m_offset(offset), m_size(size)
	{
	}
};


struct NGL_texture_descriptor
{
	std::string m_name;
	NGL_texture_type m_type;
	NGL_format m_format;
	NGL_texture_wrap_mode m_wrap_mode;
	NGL_texture_filter m_filter;
	NGL_texture_filter m_shadow_filter;
	// m_num_levels: 0 - invalid, 1 - base level, ~0 - full chain
	uint32_t m_num_levels;
	//number of surfaces for texture array
	uint32_t m_num_array;
	bool m_is_renderable;
	uint32_t m_size[3];
	float m_clear_value[4];
	bool m_unordered_access;
	bool m_input_attachment;
	bool m_memoryless;
	int samples;
	bool m_is_transfer_source;

	NGL_texture_descriptor()
	{
		m_type = NGL_TEXTURE_2D;
		m_format = NGL_UNDEFINED;
		m_wrap_mode = NGL_REPEAT_ALL;
		m_filter = NGL_NEAREST;
		m_shadow_filter = NGL_NEAREST;
		m_num_levels = 1;
		m_num_array = 1;
		m_is_renderable = false;
		m_size[0] = m_size[1] = m_size[2] = 1;
		m_clear_value[0] = m_clear_value[1] = m_clear_value[2] = 0.0f;
		m_clear_value[3] = 1.0f;
		m_unordered_access = false;
		m_input_attachment = false;
		m_memoryless = false;
		samples = 1;
		m_is_transfer_source = false;
	}
	void SetAllClearValue(float f)
	{
		m_clear_value[0] = m_clear_value[1] = m_clear_value[2] = m_clear_value[3] = f;
	}
};


struct NGL_texture_subresource
{
	uint32_t m_idx;
	//mipmap level
	uint32_t m_level;
	//layer in array or in 3D
	uint32_t m_layer;
	//face in cube
	uint32_t m_face;
	NGL_texture_subresource(uint32_t idx, uint32_t level = 0, uint32_t layer = 0, uint32_t face = 0) : m_idx(idx), m_level(level), m_layer(layer), m_face(face)
	{
	}
};


enum NGL_attachment_load_op
{
	NGL_LOAD_OP_LOAD = 0,
	NGL_LOAD_OP_CLEAR,
	NGL_LOAD_OP_DONT_CARE
};


enum NGL_attachment_store_op
{
	NGL_STORE_OP_STORE = 0,
	NGL_STORE_OP_DONT_CARE
};


struct NGL_attachment_descriptor
{
	NGL_texture_subresource	m_attachment;
	NGL_attachment_load_op m_attachment_load_op;
	NGL_attachment_store_op m_attachment_store_op;

	NGL_attachment_descriptor() : m_attachment(0, 0, 0, 0)
	{
		m_attachment_load_op = NGL_LOAD_OP_DONT_CARE;
		m_attachment_store_op = NGL_STORE_OP_DONT_CARE;
	}
};


enum NGL_resource_state
{
	NGL_COLOR_ATTACHMENT = 0,
	NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE,
	NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT,

	NGL_DEPTH_ATTACHMENT,
	NGL_READ_ONLY_DEPTH_ATTACHMENT,
	NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE,

	NGL_SHADER_RESOURCE,
	NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS,
	NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE,
	NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE,
};


struct NGL_subpass
{
	std::string m_name;
	std::vector<NGL_resource_state> m_usages;
};


struct NGL_texture_subresource_transition
{
	NGL_texture_subresource m_texture;
	NGL_resource_state m_old_state;
	NGL_resource_state m_new_state;

	NGL_texture_subresource_transition(const NGL_texture_subresource& r, NGL_resource_state o, NGL_resource_state n) : m_texture(r), m_old_state(o), m_new_state(n)
	{
	}
};


struct NGL_buffer_transition
{
	uint32_t m_idx;
	NGL_resource_state m_old_state;
	NGL_resource_state m_new_state;

	NGL_buffer_transition(uint32_t idx, NGL_resource_state o, NGL_resource_state n) : m_idx(idx), m_old_state(o), m_new_state(n)
	{
	}
};


struct NGL_job_descriptor;

typedef void(*PFN_LoadShader)(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES], std::vector<NGL_shader_uniform> &application_uniforms);
typedef void(*PFN_Error)(NGL_job_descriptor &jd, uint32_t pass, uint32_t shader_code, uint32_t &error_code, std::string &error_string);

struct NGL_job_descriptor
{
	bool m_is_compute;
	std::vector<NGL_attachment_descriptor> m_attachments;
	std::vector<NGL_subpass> m_subpasses;
	PFN_LoadShader m_load_shader_callback;
	void *m_user_data;
	uint32_t m_num_threads;
	uint32_t m_caching_behaviour_flags;

	NGL_job_descriptor()
	{
		m_is_compute = false;
		m_num_threads = 1;
		m_user_data = 0;
		m_caching_behaviour_flags = 0;
		m_load_shader_callback = 0;
	}
};


struct NGL_context_descriptor
{
	std::string m_selected_device_id;

	NGL_api m_api;
	uint32_t m_major_version;
	uint32_t m_minor_version;
	bool m_enable_validation;
	bool m_enable_vsync;
	bool m_system_attachment_is_transfer_source;

	uint32_t m_display_width;
	uint32_t m_display_height;

	PFNLOGF __logf;
	//win: hinstance, hwnd
	std::vector<void*> m_user_data;

	NGL_context_descriptor() : m_api(NGL_NULL_DRIVER), m_major_version(1), m_minor_version(0), m_enable_validation(false), m_enable_vsync(false), m_system_attachment_is_transfer_source(false), m_display_width(0), m_display_height(0), __logf(0)
	{
	}
};


typedef bool(*PFN_GenIndexBuffer)(uint32_t &buffer, NGL_format format, uint32_t num, void *data);
typedef bool(*PFN_GenVertexBuffer)(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data);
typedef bool(*PFN_GenTexture)(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas);
typedef uint32_t(*PFN_GenJob)(NGL_job_descriptor &info);
typedef void(*PFN_BeginCommandBuffer)(uint32_t idx);
typedef void(*PFN_EndCommandBuffer)(uint32_t idx);
typedef void(*PFN_SubmitCommandBuffer)(uint32_t idx);
typedef void(*PFN_Begin)(uint32_t job, uint32_t idx);
typedef void(*PFN_NextSubpass)(uint32_t job);
typedef void(*PFN_End)(uint32_t job);
typedef void(*PFN_BlendState)(uint32_t job, uint32_t attachment, NGL_blend_func func, NGL_color_channel_mask mask);
typedef void(*PFN_DepthState)(uint32_t job, NGL_depth_func func, bool mask);
typedef void(*PFN_Draw)(uint32_t job, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_mode, const void** parameters);
typedef void(*PFN_DrawInstanced)(uint32_t job, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_mode, const void** parameters, uint32_t instance_count);
typedef void(*PFN_DrawIndirect)(uint32_t job, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_mode, const void** parameters, uint32_t indirect_buffer, void* indirect_buffer_offset);
typedef bool(*PFN_Dispatch)(uint32_t job, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters);
typedef bool(*PFN_DispatchIndirect)(uint32_t job, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters, uint32_t indirect_buffer, void* indirect_buffer_offset);
typedef void(*PFN_ViewportScissor)(uint32_t job, int32_t viewport[4], int32_t scissor[4]);
typedef void(*PFN_DeletePipelines)(uint32_t job);
typedef void(*PFN_CustomAction)(uint32_t job, uint32_t parameter);
typedef void(*PFN_LineWidth)(uint32_t job, float width);
typedef bool(*PFN_GetTextureContent)(uint32_t texture, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data);
typedef bool(*PFN_GetVertexBufferContent)(uint32_t buffer, NGL_resource_state state, std::vector<uint8_t> &data);
typedef const char*(*PFN_GetString)(NGL_backend_property prop);
typedef int32_t(*PFN_GetInteger)(NGL_backend_property prop);
typedef bool(*PFN_ResizeTextures)(uint32_t num_textures, uint32_t *textures, uint32_t size[3]);
typedef void(*PFN_BeginStatistic)(NGLStatistic& statistic_vector);
typedef void(*PFN_EndStatistic)();
typedef void(*PFN_GetStatistic)();
typedef void(*PFN_Flush)();
typedef void(*PFN_Finish)();
typedef void(*PFN_DestroyContext)();
typedef void(*PFN_Barrier)(uint32_t cmd_buffer, std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers);

bool nglCreateContext(NGL_context_descriptor& descriptor);
extern PFN_BeginCommandBuffer nglBeginCommandBuffer;
extern PFN_EndCommandBuffer nglEndCommandBuffer;
extern PFN_SubmitCommandBuffer nglSubmitCommandBuffer;
extern PFN_GenJob nglGenJob;
extern PFN_Begin nglBegin;
extern PFN_NextSubpass nglNextSubpass;
extern PFN_End nglEnd;
extern PFN_BlendState nglBlendState;
extern PFN_DepthState nglDepthState;
extern PFN_Draw nglDraw;
extern PFN_DrawInstanced nglDrawInstanced;
extern PFN_DrawIndirect nglDrawIndirect;
extern PFN_Dispatch nglDispatch;
extern PFN_DispatchIndirect nglDispatchIndirect;
extern PFN_GenTexture nglGenTexture;
extern PFN_GenVertexBuffer nglGenVertexBuffer;
extern PFN_GenIndexBuffer nglGenIndexBuffer;
extern PFN_ViewportScissor nglViewportScissor;
extern PFN_GetString nglGetString;
extern PFN_GetInteger nglGetInteger;
extern PFN_DeletePipelines nglDeletePipelines;
extern PFN_CustomAction nglCustomAction;
extern PFN_LineWidth nglLineWidth;
extern PFN_GetTextureContent nglGetTextureContent;
extern PFN_GetVertexBufferContent nglGetVertexBufferContent;
extern PFN_ResizeTextures nglResizeTextures;
extern PFN_Flush nglFlush;
extern PFN_Finish nglFinish;
extern PFN_DestroyContext nglDestroyContext;
extern PFN_Barrier nglBarrier;

//Profiler:
extern PFN_BeginStatistic nglBeginStatistic;
extern PFN_EndStatistic nglEndStatistic;
extern PFN_GetStatistic nglGetStatistic;

#endif

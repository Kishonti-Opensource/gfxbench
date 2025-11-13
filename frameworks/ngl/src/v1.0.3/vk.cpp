/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "ngl_internal.h"
#define VK_PROTOTYPES
#if defined DISPLAY_PROTOCOL_NONE
#define VK_USE_KHR_DISPLAY
#elif defined DISPLAY_PROTOCOL_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif defined DISPLAY_PROTOCOL_ANDROID
#define VK_USE_PLATFORM_ANDROID_KHR
#elif defined DISPLAY_PROTOCOL_WAYLAND
#include <dlfcn.h>
#define VK_USE_PLATFORM_WAYLAND_KHR
#elif defined DISPLAY_PROTOCOL_XCB
#define VK_USE_PLATFORM_XCB_KHR
#elif defined DISPLAY_PROTOCOL_SCREEN
#if !defined(VK_USE_PLATFORM_SCREEN_QNX)
#define VK_USE_PLATFORM_SCREEN_QNX
#endif /* VK_USE_PLATFORM_SCREEN_QNX */
#include <dlfcn.h>
#endif

#ifdef ANDROID
#include "vulkan_wrapper.h"
#endif

#ifdef VK_USE_PLATFORM_SCREEN_QNX
//Use QNX Vulkan SDK headers instead of the ones included in benchmarks
#include <vulkan/vulkan.h>
#ifndef VK_API_VERSION
#define VK_API_VERSION VK_API_VERSION_1_0
#endif
#else
#include "vulkan/vulkan.h"
#endif
#include "glsl_compiler.h"
#include <map>
#include <assert.h>
#include <sstream>
#include <exception>

#ifdef VK_USE_PLATFORM_SCREEN_QNX
#define VK_FORMAT_RANGE_SIZE (VK_FORMAT_ASTC_12x12_SRGB_BLOCK - VK_FORMAT_UNDEFINED + 1)
#endif

#if 1
//#define VK_IMAGE_LAYOUT_UNDEFINED VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_PREINITIALIZED VK_IMAGE_LAYOUT_GENERAL
//#define VK_IMAGE_LAYOUT_PRESENT_SRC_KHR VK_IMAGE_LAYOUT_GENERAL
#endif

#define DEVICE VK_instance::This->m_device
#define D(TYPE,NAME,STRUCT_TYPE) TYPE NAME[1]; C(NAME[0],STRUCT_TYPE)
#define GET_INSTANCE_PROC_ADDR(inst, entrypoint)                        \
{                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) vkGetInstanceProcAddr(inst, "vk"#entrypoint); \
    if (fp##entrypoint == NULL) {                                 \
        _logf("vkGetInstanceProcAddr failed to find vk"#entrypoint,  \
                 "vkGetInstanceProcAddr Failure");                      \
	    }                                                                   \
}

#define GET_DEVICE_PROC_ADDR(dev, entrypoint)                           \
{                                                                       \
    fp##entrypoint = (PFN_vk##entrypoint) vkGetDeviceProcAddr(dev, "vk"#entrypoint);   \
    if (fp##entrypoint == NULL) {                                 \
        _logf("vkGetDeviceProcAddr failed to find vk"#entrypoint,    \
                 "vkGetDeviceProcAddr Failure");                        \
	    }                                                                   \
}

namespace VULKAN
{
#include "vk.h"

void BeginQuery(uint32_t job_);
void ResetQuery(VkCommandBuffer cb, VK_job *job);
void EndQuery(uint32_t job_);

VKAPI_ATTR VkBool32 VKAPI_CALL ValidationCallback(
	VkDebugReportFlagsEXT                       flags,
	VkDebugReportObjectTypeEXT                  objectType,
	uint64_t                                    object,
	size_t                                      location,
	int32_t                                     messageCode,
	const char*                                 pLayerPrefix,
	const char*                                 pMessage,
	void*                                 pUserData)
{
	std::string msg_type_str;

	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		msg_type_str += " INFO ";
	}
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		msg_type_str += " WARNING ";
	}
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		msg_type_str += " PERFORMANCE WARNING ";
	}
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		msg_type_str += " ERROR ";
	}
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		msg_type_str += " DEBUG ";
	}

	if (msg_type_str.empty())
	{
		msg_type_str = " UNKNOWN ";
	}

	_logf("[%s] Validation layer: %s Message: %s", msg_type_str.c_str(), pLayerPrefix, pMessage);

	/*
	* false indicates that layer should not bail-out of an
	* API call that had validation failures. This may mean that the
	* app dies inside the driver due to invalid parameter(s).
	* That's what would happen without validation layers, so we'll
	* keep that behavior here.
	*/
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
#if 0
		VK_instance *I = VK_instance::This;
		//VK_texture *t = 0;
		uint32_t id = 0;
		std::string s = pMessage;
		if (s.find("Cannot submit cmd buffer using image (") != std::string::npos)
		{
			//"Cannot submit cmd buffer using image (0x1337)";
			sscanf(s.c_str() + 38, "%x", &id);

			for (size_t i = 0; i < I->m_textures.size(); i++)
			{
				if (I->m_textures[i].m_image == (VkImage)(size_t)id)
				{
					//t = &I->m_textures[i];
				}
			}
		}
#endif
		assert(0);
	}
	return false;
}


static VkImageLayout GetImageLayout(NGL_resource_state s)
{
	switch (s)
	{
	case NGL_COLOR_ATTACHMENT:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
		return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	case NGL_DEPTH_ATTACHMENT:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	case NGL_READ_ONLY_DEPTH_ATTACHMENT:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	case NGL_SHADER_RESOURCE:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS:
		return VK_IMAGE_LAYOUT_GENERAL;

	case NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE:
		return VK_IMAGE_LAYOUT_GENERAL;

	default:
		return VK_IMAGE_LAYOUT_GENERAL;
	}
}


#if 0
static void RM_dst(VkPipelineStageFlags &PipelineStageFlags, VkAccessFlags &AccessFlags, const NGL_resource_state &s)
{
	if (s == NGL_COLOR_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		AccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else if (s == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		AccessFlags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	else if (s == NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT)
	{
		PipelineStageFlags |= 0;
		AccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else if (s == NGL_DEPTH_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		AccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else if (s == NGL_READ_ONLY_DEPTH_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		AccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	else if (s == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		AccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}
}
#endif


static void RM_src(VkPipelineStageFlags &PipelineStageFlags, VkAccessFlags &AccessFlags, const NGL_resource_state &s)
{
	if (s == NGL_COLOR_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		AccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else if (s == NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		AccessFlags |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	}
	else if (s == NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		AccessFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	}
	else if (s == NGL_DEPTH_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		AccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	}
	else if (s == NGL_READ_ONLY_DEPTH_ATTACHMENT)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		AccessFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	else if (s == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT;
	}
	else if (s == NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE)
	{
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		AccessFlags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
	}
}


uint32_t GetSubresourceId(const NGL_texture_descriptor& descriptor, uint32_t level, uint32_t layer, uint32_t face)
{
	switch (descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		return level;

	case NGL_TEXTURE_2D_ARRAY:
		return level + layer * descriptor.m_num_levels;

	case NGL_TEXTURE_CUBE:
		return level * 6 + face;

	default:
		_logf("Unknown texture type: %d", descriptor.m_type);
		assert(0);
		return 0;
	}
}


void DecomposeSubresource(const NGL_texture_descriptor& descriptor, uint32_t id, uint32_t &base_level, uint32_t &base_layer)
{
	switch (descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		base_layer = 0;
		base_level = id;
		break;

	case NGL_TEXTURE_2D_ARRAY:
	case NGL_TEXTURE_CUBE:
		base_layer = id / descriptor.m_num_levels;
		base_level = id - descriptor.m_num_levels * base_layer;
		break;

	default:
		_logf("Unknown texture type: %d", descriptor.m_type);
		assert(0);
	}
}


uint32_t GetNumSubresources(const NGL_texture_descriptor& descriptor)
{
	switch (descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		return descriptor.m_num_levels;

	case NGL_TEXTURE_2D_ARRAY:
		return descriptor.m_num_levels * descriptor.m_num_array;

	case NGL_TEXTURE_CUBE:
		return descriptor.m_num_levels * 6;

	default:
		_logf("Unknown texture type: %d", descriptor.m_type);
		assert(0);
		return 0;
	}
}


uint32_t GetNumLayers(const NGL_texture_descriptor& descriptor)
{
	switch (descriptor.m_type)
	{
	case NGL_TEXTURE_2D:
		return 1;

	case NGL_TEXTURE_2D_ARRAY:
		return descriptor.m_num_array;

	case NGL_TEXTURE_CUBE:
		return descriptor.m_num_array * 6;

	default:
		_logf("Unknown texture type: %d", descriptor.m_type);
		assert(0);
		return 0;
	}
}


VK_job::~VK_job()
{
	for (size_t i = 0; i < m_swapchain_framebuffers.size(); i++)
	{
		if (m_swapchain_framebuffers[i] != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(DEVICE, m_swapchain_framebuffers[i], nullptr);
		}
	}

	for (size_t j = 0; j < m_passes.size(); j++)
	{
		VK_pass &pass = m_passes[j];

		if (pass.m_framebuffer != VK_NULL_HANDLE)
		{
			if (pass.m_renderpass != VK_NULL_HANDLE)
			{
				vkDestroyRenderPass(DEVICE, pass.m_renderpass, nullptr);
			}
			if (pass.m_framebuffer != VK_NULL_HANDLE && m_swapchain_framebuffers[0] == VK_NULL_HANDLE)
			{
				vkDestroyFramebuffer(DEVICE, pass.m_framebuffer, nullptr);
			}
		}
	}
}


NGL_renderer* VK_job::CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos)
{
	_shader_reflection reflection;
	NGL_shader_source_descriptor ssd[NGL_NUM_SHADER_TYPES];
	std::vector<NGL_shader_uniform> application_uniforms;

	VK_renderer *renderer = new VK_renderer;

	renderer->m_my_state = sh;

	VkResult err;
	std::vector<VkPipelineShaderStageCreateInfo> PipelineShaderStageCreateInfos;
	std::vector<VkShaderModule> ShaderModules;

	m_descriptor.m_load_shader_callback(m_descriptor, sh.m_subpass, sh.m_shader.m_shader_code, ssd, application_uniforms);

	CompileGLSL(ssd, reflection);
	for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
	{
		VkShaderStageFlagBits VK_shader_types[6] =
		{
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			VK_SHADER_STAGE_COMPUTE_BIT
		};

		if (ssd[shader_type].m_binary_data.size())
		{
			VkResult err;
			VkShaderModule ShaderModule;
			VkPipelineShaderStageCreateInfo vs;
			VkShaderModuleCreateInfo ShaderModuleCreateInfo;

			C(ShaderModuleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
			ShaderModuleCreateInfo.codeSize = ssd[shader_type].m_binary_data.size();
			ShaderModuleCreateInfo.pCode = (uint32_t*)&ssd[shader_type].m_binary_data[0];

			err = vkCreateShaderModule(DEVICE, &ShaderModuleCreateInfo, 0, &ShaderModule);
			LOGVKERROR(err);

			C(vs, VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO);
			vs.stage = VK_shader_types[shader_type];
			vs.module = ShaderModule;
			vs.pName = ssd[shader_type].m_entry_point.c_str();
			vs.pName = "main";

			PipelineShaderStageCreateInfos.push_back(vs);
			ShaderModules.push_back(ShaderModule);
		}
	}

	renderer->GetActiveResources3(reflection, application_uniforms);

	m_renderers.push_back(renderer);

	if (!renderer->m_used_uniforms[0].size() && !renderer->m_used_uniforms[1].size() && !renderer->m_used_uniforms[1].size() && !renderer->m_used_uniforms[2].size())
	{
		_logf("Warning: no uniforms in renderer of pass %s\n", G().m_name.c_str());
	}

	{
		VkPipelineCacheCreateInfo PipelineCacheCreateInfo;
		C(PipelineCacheCreateInfo, VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO);

		err = vkCreatePipelineCache(DEVICE, &PipelineCacheCreateInfo, 0, &renderer->m_pipeline_cache);
		LOGVKERROR(err);
	}

	if (m_descriptor.m_is_compute)
	{
		VkComputePipelineCreateInfo ComputePipelineCreateInfo;

		C(ComputePipelineCreateInfo, VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO);
		ComputePipelineCreateInfo.flags = 0;
		ComputePipelineCreateInfo.stage = PipelineShaderStageCreateInfos[0];
		ComputePipelineCreateInfo.layout = renderer->m_pipeline_layout;

		err = vkCreateComputePipelines(DEVICE, renderer->m_pipeline_cache, 1, &ComputePipelineCreateInfo, 0, &renderer->m_pipeline);
		LOGVKERROR(err);
	}
	else
	{
		std::vector<VkVertexInputBindingDescription> VertexBindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> VertexAttributeDescriptions;
		VkPipelineColorBlendAttachmentState pAttachments[16];
		VkDynamicState DynamicStates[2];

		D(VkPipelineVertexInputStateCreateInfo, VertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
		D(VkPipelineInputAssemblyStateCreateInfo, InputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
		D(VkPipelineTessellationStateCreateInfo, TessellationState, VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO);
		D(VkPipelineViewportStateCreateInfo, ViewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
		D(VkPipelineRasterizationStateCreateInfo, RasterState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
		D(VkPipelineMultisampleStateCreateInfo, MultisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
		D(VkPipelineDepthStencilStateCreateInfo, DepthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
		D(VkPipelineColorBlendStateCreateInfo, ColorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
		D(VkPipelineDynamicStateCreateInfo, DynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);

		bool shader_matches_with_mesh = renderer->GetActiveAttribs2(VertexBindingDescriptions, VertexAttributeDescriptions, reflection, num_vbos, vbos);
		if (!shader_matches_with_mesh)
		{
			_logf("shader_matches_with_mesh\n");
		}

		{
			VertexInputState[0].vertexBindingDescriptionCount = (uint32_t)VertexBindingDescriptions.size();
			VertexInputState[0].pVertexBindingDescriptions = VertexBindingDescriptions.data();
			VertexInputState[0].vertexAttributeDescriptionCount = (uint32_t)VertexAttributeDescriptions.size();
			VertexInputState[0].pVertexAttributeDescriptions = VertexAttributeDescriptions.data();
		}

		switch (sh.m_primitive_type)
		{
		case NGL_POINTS:
		{
			InputAssemblyState[0].topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
			break;
		}
		case NGL_LINES:
		{
			InputAssemblyState[0].topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
			break;
		}
		case NGL_TRIANGLES:
		{
			InputAssemblyState[0].topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			break;
		}
		case NGL_PATCH3:
		{
			InputAssemblyState[0].topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			TessellationState[0].patchControlPoints = 3;
			break;
		}
		case NGL_PATCH4:
		{
			InputAssemblyState[0].topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			TessellationState[0].patchControlPoints = 4;
			break;
		}
		case NGL_PATCH16:
		{
			InputAssemblyState[0].topology = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
			TessellationState[0].patchControlPoints = 16;
			break;
		}
		default:
			assert(0);
		}
		{
			ViewportState[0].viewportCount = 1;
			//ViewportState[0].pViewports = &m_viewport;
			ViewportState[0].scissorCount = 1;
			//ViewportState[0].pScissors = &m_scissor;
		}

		RasterState[0].lineWidth = 1.0f;

		switch (sh.m_cull_mode)
		{
		case NGL_TWO_SIDED:
		{
			RasterState[0].polygonMode = VK_POLYGON_MODE_FILL;
			RasterState[0].frontFace = VK_FRONT_FACE_CLOCKWISE;
			RasterState[0].cullMode = VK_CULL_MODE_NONE;
			break;
		}
		case NGL_FRONT_SIDED:
		{
			RasterState[0].polygonMode = VK_POLYGON_MODE_FILL;
			RasterState[0].frontFace = VK_FRONT_FACE_CLOCKWISE;
			RasterState[0].cullMode = VK_CULL_MODE_BACK_BIT;
			break;
		}
		case NGL_BACK_SIDED:
		{
			RasterState[0].polygonMode = VK_POLYGON_MODE_FILL;
			RasterState[0].frontFace = VK_FRONT_FACE_CLOCKWISE;
			RasterState[0].cullMode = VK_CULL_MODE_FRONT_BIT;
			break;
		}
		}

		if (sh.m_depth_state.m_func == NGL_DEPTH_LESS_WITH_OFFSET)
		{
			RasterState[0].depthBiasEnable = true;
			RasterState[0].depthBiasConstantFactor = 200.0f;
			RasterState[0].depthBiasSlopeFactor = 1.0f;
		}

		{
			MultisampleState[0].rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		}
		if (G().m_depth_attachments_remap.size())
		{
			DepthStencilState[0].depthWriteEnable = VkBool32(m_current_state.m_depth_state.m_mask);

			switch (m_current_state.m_depth_state.m_func)
			{
			case NGL_DEPTH_DISABLED:
			{
				break;
			}
			case NGL_DEPTH_LESS:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_LESS;
				break;
			}
			case NGL_DEPTH_LEQUAL:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				break;
			}
			case NGL_DEPTH_EQUAL:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_EQUAL;
				break;
			}
			case NGL_DEPTH_GREATER:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_GREATER;
				break;
			}
			case NGL_DEPTH_TO_FAR:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
				break;
			}
			case NGL_DEPTH_LESS_WITH_OFFSET:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_LESS;
				break;
			}
			case NGL_DEPTH_ALWAYS:
			{
				DepthStencilState[0].depthTestEnable = VK_TRUE;
				DepthStencilState[0].depthCompareOp = VK_COMPARE_OP_ALWAYS;
				break;
			}
			default:
				_logf("Unknown depth func: %d", m_current_state.m_depth_state.m_func);
				assert(0);
			}
		}
		{

			memset(pAttachments, 0, 16 * sizeof(VkPipelineColorBlendAttachmentState));

			ColorBlendState[0].attachmentCount = (uint32_t)G().m_color_attachments_remap.size();
			ColorBlendState[0].pAttachments = pAttachments;

			for (size_t i = 0; i < G().m_color_attachments_remap.size(); i++)
			{
				const uint32_t &reference = G().m_color_attachments_remap[i];

				pAttachments[i].colorWriteMask =
					((m_current_state.m_blend_state.m_masks[reference] & NGL_CHANNEL_R) > 0) * VK_COLOR_COMPONENT_R_BIT +
					((m_current_state.m_blend_state.m_masks[reference] & NGL_CHANNEL_G) > 0) * VK_COLOR_COMPONENT_G_BIT +
					((m_current_state.m_blend_state.m_masks[reference] & NGL_CHANNEL_B) > 0) * VK_COLOR_COMPONENT_B_BIT +
					((m_current_state.m_blend_state.m_masks[reference] & NGL_CHANNEL_A) > 0) * VK_COLOR_COMPONENT_A_BIT;

				switch (m_current_state.m_blend_state.m_funcs[reference])
				{
				case NGL_BLEND_DISABLED:
					break;
				case NGL_BLEND_ADDITIVE:
				{
					pAttachments[i].blendEnable = 1;
					pAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
					pAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

					break;
				}
				case NGL_BLEND_ADDITIVE_ALFA:
				{
					pAttachments[i].blendEnable = 1;
					pAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					pAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					pAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
					pAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

					break;
				}
				case NGL_BLEND_ADDITIVE_INVERSE_ALFA:
				{
					pAttachments[i].blendEnable = 1;
					pAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					pAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					pAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
					pAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

					break;
				}
				case NGL_BLEND_ALFA:
				{
					pAttachments[i].blendEnable = 1;
					pAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					pAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					pAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					pAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					pAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
					pAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

					break;
				}
				case NGL_BLEND_MODULATIVE:
				{
					pAttachments[i].blendEnable = 1;
					pAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
					pAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
					pAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					pAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					pAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
					pAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

					break;
				}
				case NGL_BLEND_TRANSPARENT_ACCUMULATION:
				{
					pAttachments[i].blendEnable = 1;
					pAttachments[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					pAttachments[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					pAttachments[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					pAttachments[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					pAttachments[i].colorBlendOp = VK_BLEND_OP_ADD;
					pAttachments[i].alphaBlendOp = VK_BLEND_OP_ADD;

					break;
				}
				default:
					assert(0);
				}
			}
		}
		{
			DynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
			DynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;

			DynamicState[0].dynamicStateCount = 2;
			DynamicState[0].pDynamicStates = DynamicStates;
		}

		{
			VkGraphicsPipelineCreateInfo GraphicsPipelineCreateInfo;
			C(GraphicsPipelineCreateInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
			VkPipelineCreateFlags flags = 0;

			GraphicsPipelineCreateInfo.stageCount = (uint32_t)PipelineShaderStageCreateInfos.size();
			GraphicsPipelineCreateInfo.pStages = &PipelineShaderStageCreateInfos[0];
			GraphicsPipelineCreateInfo.pVertexInputState = VertexInputState;
			GraphicsPipelineCreateInfo.pInputAssemblyState = InputAssemblyState;
			if (VK_instance::This->m_PhysicalDeviceFeatures.tessellationShader && TessellationState[0].patchControlPoints)
			{
				GraphicsPipelineCreateInfo.pTessellationState = TessellationState;
			}
			GraphicsPipelineCreateInfo.pViewportState = ViewportState;
			GraphicsPipelineCreateInfo.pRasterizationState = RasterState;
			GraphicsPipelineCreateInfo.pMultisampleState = MultisampleState;
			GraphicsPipelineCreateInfo.pDepthStencilState = DepthStencilState;
			GraphicsPipelineCreateInfo.pColorBlendState = ColorBlendState;
			GraphicsPipelineCreateInfo.pDynamicState = DynamicState;
			GraphicsPipelineCreateInfo.flags = flags;
			GraphicsPipelineCreateInfo.layout = renderer->m_pipeline_layout;
			if (m_passes.size() > 1)
			{
				GraphicsPipelineCreateInfo.renderPass = m_passes[sh.m_subpass].m_renderpass;
				GraphicsPipelineCreateInfo.subpass = 0;
			}
			else
			{
				GraphicsPipelineCreateInfo.renderPass = m_passes[0].m_renderpass;
				GraphicsPipelineCreateInfo.subpass = sh.m_subpass;
			}

			err = vkCreateGraphicsPipelines(DEVICE, renderer->m_pipeline_cache, 1, &GraphicsPipelineCreateInfo, 0, &renderer->m_pipeline);
			LOGVKERROR(err);
		}
	}

	for (size_t i = 0; i < ShaderModules.size(); i++)
	{
		vkDestroyShaderModule(DEVICE, ShaderModules[i], NULL);
	}

	return renderer;
}


void VK_job::Reset1()
{
	for (size_t i = 0; i < m_renderers.size(); i++)
	{
		VK_renderer* r = (VK_renderer*)m_renderers[i];

		r->m_num_used_desc_pools[m_cmd_buffer_idx] = 0;
	}
}


VK_renderer::~VK_renderer()
{
	for (int g = 0; g < 3; ++g)
	{
		if (m_uniform_groups[g].m_descriptor_set.m_bindings.size() > 0)
		{
			vkDestroyDescriptorSetLayout(DEVICE, m_uniform_groups[g].m_descriptor_set.m_set_layout, nullptr);
		}
	}
	for (int g = 0; g < NUM_MAX_CMD_BUFFERS; ++g)
	{
		for (size_t s = 0; s < m_desc_pools[g].size(); ++s)
		{
			vkDestroyDescriptorPool(DEVICE, m_desc_pools[g][s].m_desc_pool, nullptr);
		}
	}

	vkDestroyPipeline(DEVICE, m_pipeline, NULL);
	vkDestroyPipelineCache(DEVICE, m_pipeline_cache, NULL);
	vkDestroyPipelineLayout(DEVICE, m_pipeline_layout, NULL);
}


uint32_t VK_renderer::CompileShader(uint32_t shader_type, const char *str)
{
	return 0;
}


void VK_renderer::LinkShader()
{
}


bool VK_renderer::GetActiveAttribs2(std::vector<VkVertexInputBindingDescription> &VertexBindingDescriptions, std::vector<VkVertexInputAttributeDescription> &VertexAttributeDescriptions, _shader_reflection &reflection, uint32_t num_vbos, uint32_t *vbos)
{
	if (!reflection.attributes.size())
	{
		return true;
	}

	VkVertexInputBindingDescription VertexInputBindingDescription;

	for (size_t i = 0; i <reflection.attributes.size(); i++)
	{
		_shader_reflection::Block &srva = reflection.attributes[i];
		std::string attrib_semantic_name;
		uint32_t attrib_size = 0;
		VkVertexInputAttributeDescription VertexInputAttributeDescription;

		attrib_semantic_name = srva.name;
		switch (srva.format)
		{
		case 0:
		{
			VertexInputAttributeDescription.format = VK_FORMAT_R32_SFLOAT;
			attrib_size = 1;
			break;
		}
		case 1:
		{
			VertexInputAttributeDescription.format = VK_FORMAT_R32G32_SFLOAT;
			attrib_size = 2;
			break;
		}
		case 2:
		{
			VertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
			attrib_size = 3;
			break;
		}
		case 3:
		{
			VertexInputAttributeDescription.format = VK_FORMAT_R32G32B32A32_SFLOAT;
			attrib_size = 4;
			break;
		}
		default:
			assert(0);
		}

		bool found;
		VK_vertex_buffer *vb = nullptr;
		uint32_t idx = 0;
		uint32_t buffer_idx;

		for (buffer_idx = 0; buffer_idx<num_vbos; buffer_idx++)
		{
			vb = &VK_instance::This->m_vertex_buffers[vbos[buffer_idx]];
			found = SearchAttribBySemanticAndSize(vb->m_vertex_descriptor, idx, attrib_semantic_name, attrib_size);
			if (found)
			{
				break;
			}
		}

		VertexInputAttributeDescription.binding = 0;
		VertexInputAttributeDescription.location = (uint32_t)srva.binding_or_offset_or_location;
		VertexInputAttributeDescription.offset = vb->m_vertex_descriptor.m_attribs[idx].m_offset;

		VertexAttributeDescriptions.push_back(VertexInputAttributeDescription);

		VertexInputBindingDescription.stride = vb->m_vertex_descriptor.m_stride;
	}


	VertexInputBindingDescription.binding = 0;
	//vi_bindings[0].strideInBytes = offset;
	VertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VertexBindingDescriptions.push_back(VertexInputBindingDescription);

	return true;
}


void VK_descriptor_set::AddUniform(_shader_reflection::Block &mb)
{
	VkShaderStageFlags stage_flags[NGL_NUM_SHADER_TYPES] =
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_COMPUTE_BIT
	};

	VkDescriptorSetLayoutBinding slb;

	if (mb.format == 1000)
	{
		slb.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	}
	else if (mb.format == 1001 || mb.format == 1002)
	{
		slb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	}
	else if (mb.format == 2000 || mb.format == 2001)
	{
		slb.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	}
	else if (mb.format == 2002)
	{
		slb.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	}
	else if (mb.format == 2003)
	{
		slb.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	}
	else
	{
		assert(0);
	}

	slb.binding = mb.binding_or_offset_or_location;
	slb.descriptorCount = 1;
	slb.pImmutableSamplers = 0;
	slb.stageFlags = stage_flags[mb.stage];

	m_bindings.push_back(slb);
}


void VK_descriptor_set::CreateWriteDescriptorSets()
{
	m_WriteDescriptorSet.resize(m_bindings.size());
	m_DescriptorInfos.resize(m_bindings.size());

	for (size_t i = 0; i < m_bindings.size(); i++)
	{
		VkDescriptorSetLayoutBinding &slb = m_bindings[i];
		VkWriteDescriptorSet &wd = m_WriteDescriptorSet[i];
		VkDescriptorInfo &di = m_DescriptorInfos[i];

		wd.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		wd.pNext = NULL;
		wd.dstSet = 0;
		wd.dstBinding = slb.binding;
		wd.dstArrayElement = 0;
		wd.descriptorCount = 1;
		wd.pImageInfo = &di.ImageInfo;
		wd.pBufferInfo = &di.BufferInfo;
		wd.pTexelBufferView = &di.TexelBufferView;
		wd.descriptorType = slb.descriptorType;
	}

	VkResult err;

	const VkDescriptorSetLayoutCreateInfo descriptor_set_layout =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		(uint32_t)m_bindings.size(),
		m_bindings.data()
	};

	err = vkCreateDescriptorSetLayout(DEVICE, &descriptor_set_layout, 0, &m_set_layout);
	LOGVKERROR(err);
}


void VK_renderer::Alloc(uint32_t fr, uint32_t num)
{
	VkResult err;
	std::vector<VkDescriptorPoolSize> sizes;
	std::vector<VkDescriptorSetLayout> set_layouts;

	set_layouts.reserve(num * 3);
	sizes.reserve(num * 3 * 16);

	for (size_t j = 0; j < num; j++)
	{
		for (uint32_t g = 0; g < 3; g++)
		{
			VK_uniform_group &ugroup = m_uniform_groups[g];

			if (ugroup.m_descriptor_set.m_bindings.size())
			{
				for (size_t i = 0; i < ugroup.m_descriptor_set.m_bindings.size(); i++)
				{
					VkDescriptorSetLayoutBinding &slb = ugroup.m_descriptor_set.m_bindings[i];
					VkDescriptorPoolSize ps;

					ps.descriptorCount = 1;
					ps.type = slb.descriptorType;

					sizes.push_back(ps);
				}

				set_layouts.push_back(ugroup.m_descriptor_set.m_set_layout);
			}
		}
	}

	if (!set_layouts.size())
	{
		return;
	}

	{
		VK_descriptor_pool pool = {0};
		m_desc_pools[fr].push_back(pool);
	}

	VK_descriptor_pool &pool = m_desc_pools[fr].back();

	const VkDescriptorPoolCreateInfo descriptor_pool =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		num * m_num_used_uniform_groups_with_descriptor_sets,
		(uint32_t)sizes.size(),
		sizes.data()
	};

	err = vkCreateDescriptorPool(DEVICE, &descriptor_pool, 0, &pool.m_desc_pool);
	LOGVKERROR(err);

	const VkDescriptorSetAllocateInfo alloc_info =
	{
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		0,
		pool.m_desc_pool,
		num * m_num_used_uniform_groups_with_descriptor_sets,
		set_layouts.data()
	};

	pool.m_desc_sets.resize(num * m_num_used_uniform_groups_with_descriptor_sets);

	err = vkAllocateDescriptorSets(DEVICE, &alloc_info, pool.m_desc_sets.data());
	LOGVKERROR(err);
}


VkDescriptorSet* VK_renderer::UseNextSet(uint32_t fr)
{
	const uint32_t desc_pool_granularity = 128;
	uint32_t G = m_num_used_desc_pools[fr] / desc_pool_granularity;
	uint32_t H = m_num_used_desc_pools[fr] % desc_pool_granularity;

	m_num_used_desc_pools[fr]++;

	if (G >= m_desc_pools[fr].size())
	{
		Alloc(fr, desc_pool_granularity);
	}

	return &m_desc_pools[fr][G].m_desc_sets[H * m_num_used_uniform_groups_with_descriptor_sets];
}


void VK_renderer::GetActiveResources3(_shader_reflection &reflection, std::vector<NGL_shader_uniform> &application_uniforms)
{
	std::vector<VkDescriptorSetLayout> set_layouts;
	std::map<std::string, NGL_used_uniform> uniforms[3];
	std::map<std::string, NGL_used_uniform> textures[3];
	NGL_shader_type shader_types[6] =
	{
		NGL_VERTEX_SHADER,
		NGL_TESS_CONTROL_SHADER,
		NGL_TESS_EVALUATION_SHADER,
		NGL_GEOMETRY_SHADER,
		NGL_FRAGMENT_SHADER,
		NGL_COMPUTE_SHADER
	};
	int push_constant_min_offset[3][NGL_NUM_SHADER_TYPES];
	uint32_t push_constant_max_size[3][NGL_NUM_SHADER_TYPES];

	m_num_used_uniform_groups_with_descriptor_sets = 0;

	for (size_t g = 0; g < 3; g++)
	{
		m_uniform_groups[g].m_aligned_memory_size = 0;

		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			m_uniform_groups[g].m_ubo_memory_sizes[shader_type] = 0;
			m_uniform_groups[g].m_range[shader_type].offset = 0;
			m_uniform_groups[g].m_range[shader_type].size = 0;
			push_constant_min_offset[g][shader_type] = 9999;
			push_constant_max_size[g][shader_type] = 0;
		}
	}

	for (size_t i = 0; i < reflection.uniforms.size(); i++)
	{
		_shader_reflection::Block &mb = reflection.uniforms[i];

		uint32_t group;
		int32_t application_location;

		FindUniform(group, application_location, application_uniforms, mb.name);

		if (mb.format == 1000)
		{
			if (mb.name.find("uniformObject") != std::string::npos)
			{
				for (size_t i = 0; i < mb.blocks.size(); i++)
				{
					_shader_reflection::Block &b = mb.blocks[i];

					FindUniform(group, application_location, application_uniforms, b.name);

					NGL_used_uniform &uu = uniforms[group][b.name];
					uint32_t size_in_bytes = 0;

					uu.m_uniform.m_name = b.name;
					uu.m_uniform.m_size = b.size;
					switch (b.format)
					{
					case 0:
					{
						uu.m_uniform.m_format = NGL_FLOAT;
						size_in_bytes = 4;
						break;
					}
					case 1:
					{
						uu.m_uniform.m_format = NGL_FLOAT2;
						size_in_bytes = 8;
						break;
					}
					case 3:
					{
						uu.m_uniform.m_format = NGL_FLOAT4;
						size_in_bytes = 16;
						break;
					}
					case 4:
					{
						uu.m_uniform.m_format = NGL_FLOAT16;
						size_in_bytes = 64;
						break;
					}
					case 10:
					{
						uu.m_uniform.m_format = NGL_INT;
						size_in_bytes = 4;
						break;
					}
					case 20:
					{
						uu.m_uniform.m_format = NGL_UINT;
						size_in_bytes = 4;
						break;
					}
					case 23:
					{
						uu.m_uniform.m_format = NGL_UINT4;
						size_in_bytes = 16;
						break;
					}
					default:
					{
						_logf("Warning!! unhandled reflection case!!\n");
						assert(0);
					}
					}
					uu.m_application_location = application_location;
					uu.m_shader_location[shader_types[b.stage]] = b.binding_or_offset_or_location;

					if (mb.set == 3)
					{
						NGL_shader_type stage = shader_types[mb.stage];

						uu.m_binding_type |= (1 << stage);

						if (push_constant_min_offset[group][stage] > b.binding_or_offset_or_location)
						{
							push_constant_min_offset[group][stage] = b.binding_or_offset_or_location;
						}

						uint32_t new_ubo_size = size_in_bytes * b.size + b.binding_or_offset_or_location;
						if (new_ubo_size > push_constant_max_size[group][stage])
						{
							push_constant_max_size[group][stage] = new_ubo_size;
						}
					}
					else
					{
						uint32_t new_ubo_size = size_in_bytes * b.size + b.binding_or_offset_or_location;
						uint32_t old_ubo_size = m_uniform_groups[group].m_ubo_memory_sizes[shader_types[b.stage]];

						m_uniform_groups[group].m_ubo_memory_sizes[shader_types[b.stage]] = new_ubo_size > old_ubo_size ? new_ubo_size : old_ubo_size;
					}
				}

				if (mb.set != 3)
				{
					m_uniform_groups[group].m_descriptor_set.where_is_the_ubo[shader_types[mb.stage]] = m_uniform_groups[group].m_descriptor_set.m_bindings.size();
				}
			}
			else
			{
				NGL_used_uniform &uu = textures[group][mb.name];

				uu.m_uniform.m_name = mb.name;
				uu.m_uniform.m_size = mb.size;
				uu.m_uniform.m_format = NGL_BUFFER;
				uu.m_application_location = application_location;
				uu.m_shader_location[shader_types[mb.stage]] = (int32_t)m_uniform_groups[group].m_descriptor_set.m_bindings.size();

				if (application_uniforms[application_location].m_format == NGL_BUFFER_SUBRESOURCE)
				{
					uu.m_uniform.m_format = NGL_BUFFER_SUBRESOURCE;
				}
			}
		}
		else if (mb.format == 1001 || mb.format == 1002)
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_BUFFER;
			uu.m_application_location = application_location;
			uu.m_shader_location[shader_types[mb.stage]] = (int32_t)m_uniform_groups[group].m_descriptor_set.m_bindings.size();

			if (application_uniforms[application_location].m_format == NGL_BUFFER_SUBRESOURCE)
			{
				uu.m_uniform.m_format = NGL_BUFFER_SUBRESOURCE;
			}
		}
		else if (mb.format == 2000 || mb.format == 2001)
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_binding_type = mb.format == 2001 ? 1 : 0;
			uu.m_application_location = application_location;
			uu.m_shader_location[shader_types[mb.stage]] = (int32_t)m_uniform_groups[group].m_descriptor_set.m_bindings.size();

			if (application_uniforms[application_location].m_format == NGL_TEXTURE_SUBRESOURCE)
			{
				uu.m_uniform.m_format = NGL_TEXTURE_SUBRESOURCE;
			}
		}
		else if (mb.format == 2002)
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_application_location = application_location;
			uu.m_binding_type = 2;
			uu.m_shader_location[shader_types[mb.stage]] = (int32_t)m_uniform_groups[group].m_descriptor_set.m_bindings.size();
		}
		else if (mb.format == 2003)
		{
			NGL_used_uniform &uu = textures[group][mb.name];

			uu.m_uniform.m_name = mb.name;
			uu.m_uniform.m_size = mb.size;
			uu.m_uniform.m_format = NGL_TEXTURE;
			uu.m_application_location = application_location;
			uu.m_binding_type = 3;
			uu.m_shader_location[shader_types[mb.stage]] = (int32_t)m_uniform_groups[group].m_descriptor_set.m_bindings.size();
		}
		else
		{
			assert(0);
		}

		if (mb.set != 3)
		{
			m_uniform_groups[group].m_descriptor_set.AddUniform(mb);
		}
	}

	for (int g = 0; g < 3; g++)
	{
		for (std::map<std::string, NGL_used_uniform>::iterator i = uniforms[g].begin(); i != uniforms[g].end(); i++)
		{
			m_used_uniforms[g].push_back(i->second);
		}

		for (std::map<std::string, NGL_used_uniform>::iterator i = textures[g].begin(); i != textures[g].end(); i++)
		{
			m_used_uniforms[g].push_back(i->second);
		}

		if (m_used_uniforms[g].size())
		{
			for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				m_uniform_groups[g].m_aligned_memory_size += u_align(m_uniform_groups[g].m_ubo_memory_sizes[shader_type], (uint32_t)VK_instance::This->m_PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
			}

			if (m_uniform_groups[g].m_descriptor_set.m_bindings.size())
			{
				m_num_used_uniform_groups_with_descriptor_sets++;

				m_uniform_groups[g].m_descriptor_set.CreateWriteDescriptorSets();

				set_layouts.push_back(m_uniform_groups[g].m_descriptor_set.m_set_layout);
			}
		}
	}

	{
		VkResult err;
		std::vector<VkPushConstantRange> push_constant_ranges;
		VkShaderStageFlags stage_flags[NGL_NUM_SHADER_TYPES] =
		{
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			VK_SHADER_STAGE_COMPUTE_BIT
		};

		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			int push_constant_min_offset2 = 999999;
			uint32_t push_constant_max_size2 = 0;

			for (size_t g = 0; g < 3; g++)
			{
				if (push_constant_max_size[g][shader_type])
				{
					if (push_constant_min_offset2 > push_constant_min_offset[g][shader_type])
					{
						push_constant_min_offset2 = push_constant_min_offset[g][shader_type];
					}

					if (push_constant_max_size[g][shader_type] > push_constant_max_size2)
					{
						push_constant_max_size2 = push_constant_max_size[g][shader_type];
					}

					VkPushConstantRange &range = m_uniform_groups[g].m_range[shader_type];

					range.stageFlags = stage_flags[shader_type];
					range.offset = (uint32_t)push_constant_min_offset[g][shader_type];
					range.size = push_constant_max_size[g][shader_type] - push_constant_min_offset[g][shader_type];
				}
			}

			if (push_constant_min_offset2 != 999999)
			{
				VkPushConstantRange range;

				range.stageFlags = stage_flags[shader_type];
				range.offset = (uint32_t)push_constant_min_offset2;
				range.size = push_constant_max_size2 - push_constant_min_offset2;

				push_constant_ranges.push_back(range);
			}
		}

		const VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo =
		{
			VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			NULL,
			0,
			(uint32_t)set_layouts.size(),
			set_layouts.data(),
			(uint32_t)push_constant_ranges.size(),
			push_constant_ranges.data()
		};

		err = vkCreatePipelineLayout(DEVICE, &pPipelineLayoutCreateInfo, 0, &m_pipeline_layout);
		LOGVKERROR(err);
	}
}


void VK_uniform_group::BindUniform(VK_job *job, const NGL_used_uniform &uu, const void *ptr)
{
	uint32_t uu_size = 0;

	switch (uu.m_uniform.m_format)
	{
	case NGL_TEXTURE_SUBRESOURCE:
	case NGL_TEXTURE:
	{
		VK_texture *texture;
		VkImageView *view;

		if (uu.m_uniform.m_format == NGL_TEXTURE)
		{
			texture = &VK_instance::This->m_textures[*(uint32_t *)ptr];
			view = &texture->m_resource_view_all;
		}
		else
		{
			NGL_texture_subresource *ts = (NGL_texture_subresource*)ptr;

			texture = &VK_instance::This->m_textures[ts->m_idx];
			view = &texture->m_resource_views[ts->m_level];
		}

		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			if (uu.m_shader_location[shader_type] > -1)
			{
				VkDescriptorInfo &di = m_descriptor_set.m_DescriptorInfos[uu.m_shader_location[shader_type]];

				di.ImageInfo.imageView = *view;

				switch (uu.m_binding_type)
				{
				case 0:
				{
					//normal
					di.ImageInfo.sampler = texture->m_samplers[0];
					di.ImageInfo.imageLayout = texture->m_last_layout;
					break;
				}
				case 1:
				{
					//shadow
					di.ImageInfo.sampler = texture->m_samplers[1];
					di.ImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
					break;
				}
				case 2:
				{
					//input attachment
					di.ImageInfo.imageLayout = texture->m_is_color ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					break;
				}
				case 3:
				{
					//image
					di.ImageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
					break;
				}
				}
			}
		}
		break;
	}
	case NGL_BUFFER:
	{
		uint32_t *buffer_idx = (uint32_t *)ptr;
		VK_vertex_buffer &v = VK_instance::This->m_vertex_buffers[*buffer_idx];

		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			if (uu.m_shader_location[shader_type] > -1)
			{
				VkDescriptorInfo &di = m_descriptor_set.m_DescriptorInfos[uu.m_shader_location[shader_type]];

				di.BufferInfo.buffer = v.m_buffer;
				di.BufferInfo.offset = 0;
				di.BufferInfo.range = (VkDeviceSize)v.m_datasize;
			}
		}
		break;
	}
	case NGL_BUFFER_SUBRESOURCE:
	{
		NGL_buffer_subresource *bsr = (NGL_buffer_subresource*)ptr;
		VK_vertex_buffer &v = VK_instance::This->m_vertex_buffers[bsr->m_buffer];

		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			if (uu.m_shader_location[shader_type] > -1)
			{
				VkDescriptorInfo &di = m_descriptor_set.m_DescriptorInfos[uu.m_shader_location[shader_type]];

				di.BufferInfo.buffer = v.m_buffer;
				di.BufferInfo.offset = (VkDeviceSize)bsr->m_offset;
				di.BufferInfo.range = (VkDeviceSize)(bsr->m_size == (uint32_t)~0 ? v.m_datasize : bsr->m_size);
			}
		}
		break;
	}
	case NGL_FLOAT16:
	{
		uu_size = 64;

		break;
	}
	case NGL_FLOAT4:
	{
		uu_size = 16;
		break;
	}
	case NGL_FLOAT:
	{
		uu_size = 4;
		break;
	}
	case NGL_FLOAT2:
	{
		uu_size = 8;
		break;
	}
	case NGL_INT:
	{
		uu_size = 4;
		break;
	}
	case NGL_INT2:
	{
		uu_size = 8;
		break;
	}
	case NGL_INT4:
	{
		uu_size = 16;
		break;
	}
	case NGL_UINT:
	{
		uu_size = 4;
		break;
	}
	case NGL_UINT2:
	{
		uu_size = 8;
		break;
	}
	case NGL_UINT4:
	{
		uu_size = 16;
		break;
	}
	default:
	{
		_logf("Warning!! Unhandled bind uniform!!\n");
	}
	}
	if (uu_size)
	{
		for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
		{
			if (uu.m_shader_location[shader_type] > -1)
			{
				if ((uu.m_binding_type & (1 << shader_type)) == 0)
				{
					memcpy(m_ubo_ptrs[shader_type] + uu.m_shader_location[shader_type], ptr, uu_size * uu.m_uniform.m_size);
				}
				else
				{
					memcpy(m_data[shader_type] + uu.m_shader_location[shader_type] - m_range[shader_type].offset, ptr, uu_size * uu.m_uniform.m_size);
				}
			}
		}
	}
}


void VK_uniform_group::SetUpUBO(uint32_t shader_type, VK_ubo &ubo)
{
	size_t U = m_descriptor_set.where_is_the_ubo[shader_type];
	VkDescriptorInfo &di = m_descriptor_set.m_DescriptorInfos[U];

	m_ubo_ptrs[shader_type] = ubo.m_mapped_ptr + ubo.m_major_offset + ubo.m_minor_offset;

	di.BufferInfo.buffer = ubo.m_buffer;
	di.BufferInfo.offset = ubo.m_major_offset + ubo.m_minor_offset;
	di.BufferInfo.range = m_ubo_memory_sizes[shader_type];

	ubo.m_minor_offset += u_align(m_ubo_memory_sizes[shader_type], (uint32_t)VK_instance::This->m_PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
}


bool GenTexture(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas)
{
	if (texture_layout.m_is_renderable && datas)
	{
		//error;
		return 0;
	}
	if (!datas)
	{
		if (!texture_layout.m_is_renderable && !texture_layout.m_unordered_access)
		{
			//error;
			return 0;
		}
	}

	if (buffer && buffer >= VK_instance::This->m_textures.size())
	{
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		VK_texture texture;

		texture.m_texture_descriptor = texture_layout;

		VK_instance::This->m_textures.push_back(texture);
		buffer = (uint32_t)VK_instance::This->m_textures.size() - 1;
	}

	VK_texture &texture = VK_instance::This->m_textures[buffer];
	VkImageCreateFlags image_create_flag_bits = 0;
	VkFilter shadow_filter = VK_FILTER_NEAREST;

	VkImageUsageFlags transfer_src_bit = texture_layout.m_is_transfer_source ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0;

	texture.m_texture_descriptor = texture_layout;
	texture.m_is_color = true;
	texture.m_aspect = VK_IMAGE_ASPECT_COLOR_BIT;

	switch (texture_layout.m_type)
	{
	case NGL_TEXTURE_2D:
	{
		texture.m_image_type = VK_IMAGE_TYPE_2D;
		texture.m_image_view_type = VK_IMAGE_VIEW_TYPE_2D;
		break;
	}
	case NGL_TEXTURE_CUBE:
	{
		texture.m_image_type = VK_IMAGE_TYPE_2D;
		texture.m_image_view_type = VK_IMAGE_VIEW_TYPE_CUBE;
		image_create_flag_bits = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
		break;
	}
	case NGL_TEXTURE_2D_ARRAY:
	{
		texture.m_image_type = VK_IMAGE_TYPE_2D;
		texture.m_image_view_type = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		break;
	}
	default:
		_logf("Unknown texture type: %d\n", texture_layout.m_type);
		assert(0);
	}

	uint32_t block_dim_x = 1;
	uint32_t block_dim_y = 1;
	uint32_t block_size = 0; // in bits
	bool need_RGB888toRGBA8888_conversion = false;

	switch (texture_layout.m_format)
	{
	case NGL_R8_UNORM:
	{
		texture.m_format = VK_FORMAT_R8_UNORM;
		block_size = 8;
		break;
	}
	case NGL_R8_G8_B8_UNORM:
	{
		texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
		block_size = 24;
		need_RGB888toRGBA8888_conversion = true;
		break;
	}
	case NGL_R8_G8_B8_UNORM_SRGB:
	{
		texture.m_format = VK_FORMAT_R8G8B8A8_SRGB;
		block_size = 24;
		need_RGB888toRGBA8888_conversion = true;
		break;
	}
	case NGL_R8_G8_B8_A8_UNORM:
	{
		texture.m_format = VK_FORMAT_R8G8B8A8_UNORM;
		block_size = 32;
		break;
	}
	case NGL_R8_G8_B8_A8_UNORM_SRGB:
	{
		texture.m_format = VK_FORMAT_R8G8B8A8_SRGB;
		block_size = 32;
		break;
	}
	case NGL_R16_FLOAT:
	{
		texture.m_format = VK_FORMAT_R16_SFLOAT;
		block_size = 16;
		break;
	}
	case NGL_R32_FLOAT:
	{
		texture.m_format = VK_FORMAT_R32_SFLOAT;
		block_size = 32;
		break;
	}
	case NGL_D16_UNORM:
	{
		texture.m_format = VK_FORMAT_D16_UNORM;
		texture.m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		texture.m_is_color = false;
		block_size = 16;

		if (VK_instance::This->m_propertiesi[NGL_D16_LINEAR_SHADOW_FILTER])
		{
			if (texture_layout.m_shadow_filter == NGL_LINEAR)
			{
				shadow_filter = VK_FILTER_LINEAR;
			}
		}
		break;
	}
	case NGL_D24_UNORM:
	{
		texture.m_format = VK_instance::This->m_D24_format;
		texture.m_aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		texture.m_is_color = false;
		block_size = 32;

		if (VK_instance::This->m_propertiesi[NGL_D24_LINEAR_SHADOW_FILTER])
		{
			if (texture_layout.m_shadow_filter == NGL_LINEAR)
			{
				shadow_filter = VK_FILTER_LINEAR;
			}
		}
		break;
	}
	case NGL_R16_G16_FLOAT:
	{
		texture.m_format = VK_FORMAT_R16G16_SFLOAT;
		block_size = 1;
		block_size = 32;
		break;
	}
	case NGL_R32_G32_B32_FLOAT:
	{
		texture.m_format = VK_FORMAT_R32G32B32_SFLOAT;
		block_size = 96;
		break;
	}
	case NGL_R32_G32_B32_A32_FLOAT:
	{
		texture.m_format = VK_FORMAT_R32G32B32A32_SFLOAT;
		block_size = 128;
		break;
	}
	case NGL_R11_B11_B10_FLOAT:
	{
		texture.m_format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
		block_size = 32;
		break;
	}
	case NGL_R16_G16_B16_FLOAT:
	{
		texture.m_format = VK_FORMAT_R16G16B16_SFLOAT;
		block_size = 48;
		break;
	}
	case NGL_R16_G16_B16_A16_FLOAT:
	{
		texture.m_format = VK_FORMAT_R16G16B16A16_SFLOAT;
		block_size = 64;
		break;
	}
	case NGL_R8_G8_B8_ETC2_UNORM:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		texture.m_format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_ETC2_UNORM_SRGB:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		texture.m_format = VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_ETC2_UNORM:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		texture.m_format = VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_ETC2_UNORM_SRGB:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		texture.m_format = VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_DXT1_UNORM:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		texture.m_format = VK_FORMAT_BC1_RGB_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A1_DXT1_UNORM:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		texture.m_format = VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_DXT1_UNORM_SRGB:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		texture.m_format = VK_FORMAT_BC1_RGB_SRGB_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A1_DXT1_UNORM_SRGB:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 64;
		texture.m_format = VK_FORMAT_BC1_RGBA_SRGB_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_DXT5_UNORM:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		texture.m_format = VK_FORMAT_BC3_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_DXT5_UNORM_SRGB:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		texture.m_format = VK_FORMAT_BC3_SRGB_BLOCK;
		break;
	}
#if 0

	// compressed
	case _texture_layout::DXT5:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	}
	case _texture_layout::DXT5_SRGB:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
		texture.m_GLformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	}

	case _texture_layout::ETC2_RGB888:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_RGB8_ETC2;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}
	case _texture_layout::ETC2_SRBG888:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ETC2;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}

	case _texture_layout::ETC2_RGBA8888:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_RGBA8_ETC2_EAC;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}
	case _texture_layout::ETC2_SRGB888_ALPHA8:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}

	case _texture_layout::ASTC_4x4_RGBA8888:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_4x4_KHR;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}
	case _texture_layout::ASTC_4x4_SRGB888_ALPHA8:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}

	case _texture_layout::ASTC_5x5_RGBA8888:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_5x5_KHR;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}
	case _texture_layout::ASTC_5x5_SRGB888_ALPHA8:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}

	case _texture_layout::ASTC_8x8_RGBA8888:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_RGBA_ASTC_8x8_KHR;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}
	case _texture_layout::ASTC_8x8_SRGB888_ALPHA8:
	{
		is_compressed = true;
		texture.m_GLinternalformat = GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR;
		texture.m_GLformat = texture.m_GLinternalformat;
		break;
	}
#endif

	case NGL_R8_G8_B8_A8_ASTC_4x4_UNORM:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		texture.m_format = VK_FORMAT_ASTC_4x4_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_ASTC_4x4_UNORM_SRGB:
	{
		block_dim_x = 4;
		block_dim_y = 4;
		block_size = 128;
		texture.m_format = VK_FORMAT_ASTC_4x4_SRGB_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_ASTC_6x6_UNORM:
	{
		block_dim_x = 6;
		block_dim_y = 6;
		block_size = 128;
		texture.m_format = VK_FORMAT_ASTC_6x6_UNORM_BLOCK;
		break;
	}
	case NGL_R8_G8_B8_A8_ASTC_6x6_UNORM_SRGB:
	{
		block_dim_x = 6;
		block_dim_y = 6;
		block_size = 128;
		texture.m_format = VK_FORMAT_ASTC_6x6_SRGB_BLOCK;
		break;
	}

	case NGL_R9_G9_B9_E5_SHAREDEXP:
	{
		block_size = 32;
		texture.m_format = VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
		break;
	}
	case NGL_R10_G10_B10_A2_UINT:
	{
		block_size = 32;
		texture.m_format = VK_FORMAT_A2R10G10B10_UINT_PACK32;
		break;
	}
	case NGL_R10_G10_B10_A2_UNORM:
	{
		block_size = 32;
		texture.m_format = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
		break;
	}

	default:
	{
		_logf("ERROR: unhandled texture format: %s - %u\n", texture_layout.m_name.c_str(), texture_layout.m_format);
		assert(0);
	}
	}

	VkExtent3D texture_extent = { texture.m_texture_descriptor.m_size[0], texture.m_texture_descriptor.m_size[1], texture.m_texture_descriptor.m_size[2] };
	uint32_t num_layers = GetNumLayers(texture.m_texture_descriptor);

	if (datas)
	{
		if (texture.m_is_color)
		{
			uint32_t num_texture_mip_levels = texture.m_texture_descriptor.m_num_levels;
			VkCommandBuffer setup_cmd;

			// Create target image

			VK_instance::This->CreateImage(
				texture, 
				texture.m_image_type, 
				texture.m_format,
				texture_extent,
				num_texture_mip_levels,
				num_layers,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | transfer_src_bit,
				image_create_flag_bits);

			setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();

			VK_instance::This->set_image_layout(
				setup_cmd,
				texture.m_image,
				VK_IMAGE_LAYOUT_PREINITIALIZED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				num_texture_mip_levels,
				0,
				num_layers);

			VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);
			
			// Determine staging buffer size

			uint32_t data_vector_idx = 0;
			uint32_t staging_buffer_size = 0;
			for (uint32_t m = 0; m < num_texture_mip_levels; m++)
			{
				uint32_t extent_width = texture.m_texture_descriptor.m_size[0] / (1 << m);
				uint32_t extent_height = texture.m_texture_descriptor.m_size[1] / (1 << m);
				uint32_t extent_depth = 1;

				extent_width = extent_width == 0 ? 1 : extent_width;
				extent_height = extent_height == 0 ? 1 : extent_height;

				for (uint32_t l = 0; l < num_layers; l++)
				{
					uint32_t src_data_size = static_cast<uint32_t>((*datas)[data_vector_idx].size());
					if (data_vector_idx < (*datas).size() && src_data_size)
					{
						if (need_RGB888toRGBA8888_conversion)
						{
							src_data_size = extent_width * extent_height * 4;
						}
					}
					else
					{
						src_data_size = extent_width * extent_height * extent_depth * 4;
					}

					staging_buffer_size += src_data_size;
					data_vector_idx++;
				}
			}

			// Create staging buffer

			VK_staging_buffer staging_buffer;
			VK_instance::This->CreateStagingBuffer(staging_buffer, staging_buffer_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

			// Fill staging buffer

			VkResult err;
			uint8_t *mapped_data = 0;
			err = vkMapMemory(DEVICE, staging_buffer.m_mem, 0, VK_WHOLE_SIZE, 0, (void **)&mapped_data);
			LOGVKERROR(err);

			data_vector_idx = 0;
			uint32_t offset = 0;
			std::vector<uint8_t> tmp_data;
			std::vector<VkBufferImageCopy> bufferCopyRegions;
			for (uint32_t m = 0; m < num_texture_mip_levels; m++)
			{
				uint32_t extent_width = texture.m_texture_descriptor.m_size[0] / (1 << m);
				uint32_t extent_height = texture.m_texture_descriptor.m_size[1] / (1 << m);
				uint32_t extent_depth = 1;

				extent_width = extent_width == 0 ? 1 : extent_width;
				extent_height = extent_height == 0 ? 1 : extent_height;

				// Calculate the number of blocks for the mipmap
				//uint32_t block_count_x = (extent_width + block_dim_x - 1) / block_dim_x;
				//uint32_t block_count_y = (extent_height + block_dim_y - 1) / block_dim_y;
				(void)block_dim_x;
				(void)block_dim_y;

				for (uint32_t l = 0; l < num_layers; l++)
				{
					uint8_t *src_data = nullptr;
					uint32_t src_data_size = static_cast<uint32_t>((*datas)[data_vector_idx].size());
					if (data_vector_idx < (*datas).size() && src_data_size)
					{
						src_data = &(*datas)[data_vector_idx][0];

						if (need_RGB888toRGBA8888_conversion)
						{
							tmp_data.resize(extent_width * extent_height * 4);
							RGB888toRGBA8888(extent_width, extent_height, src_data, tmp_data.data());
							src_data = tmp_data.data();
							src_data_size = static_cast<uint32_t>(tmp_data.size());
							block_size = 32;
						}
					}
					else
					{
						tmp_data.clear();
						tmp_data.resize(extent_width * extent_height * extent_depth * 4, 0);
						src_data = tmp_data.data();
						src_data_size = static_cast<uint32_t>(tmp_data.size());
					}

					(void)block_size;

					memcpy(&mapped_data[offset], src_data, src_data_size);

					VkBufferImageCopy bufferCopyRegion = {};
					bufferCopyRegion.bufferOffset = offset;
					//size_t src_row_in_bytes = block_count_x * block_size / 8;
					bufferCopyRegion.bufferRowLength = 0; // if 0, vulkan assumes the data is tightly packed
					bufferCopyRegion.bufferImageHeight = 0;
					bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					bufferCopyRegion.imageSubresource.mipLevel = m;
					bufferCopyRegion.imageSubresource.baseArrayLayer = l;
					bufferCopyRegion.imageSubresource.layerCount = 1;
					bufferCopyRegion.imageExtent.width = extent_width;
					bufferCopyRegion.imageExtent.height = extent_height;
					bufferCopyRegion.imageExtent.depth = extent_depth;
					bufferCopyRegions.push_back(bufferCopyRegion);

					offset += src_data_size;
					data_vector_idx++;
				}
			}

			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = NULL;
			range.memory = staging_buffer.m_mem;
			range.offset = 0;
			range.size = VK_WHOLE_SIZE;
			err = vkFlushMappedMemoryRanges(DEVICE, 1, &range);
			LOGVKERROR(err);

			vkUnmapMemory(DEVICE, staging_buffer.m_mem);

			// Copy from staging buffer to image

			setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();
			
			vkCmdCopyBufferToImage(
				setup_cmd,
				staging_buffer.m_buffer,
				texture.m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				static_cast<uint32_t>(bufferCopyRegions.size()),
				bufferCopyRegions.data());

			VK_instance::This->set_image_layout(
				setup_cmd,
				texture.m_image,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				num_texture_mip_levels,
				0,
				num_layers);

			VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);

			texture.m_last_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

			// Free staging buffer

			vkDestroyBuffer(DEVICE, staging_buffer.m_buffer, 0);
			vkFreeMemory(DEVICE, staging_buffer.m_mem, 0);
		}
	}
	else
	{
		VkImageUsageFlags usage = 0;
		VkImageLayout layout;

		if (texture.m_is_color)
		{
			layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
		else
		{
			layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}

		if (texture_layout.m_type != NGL_RENDERBUFFER)
		{
			usage += VK_IMAGE_USAGE_SAMPLED_BIT;
		}
		if (texture_layout.m_unordered_access)
		{
			usage += VK_IMAGE_USAGE_STORAGE_BIT;
			layout = VK_IMAGE_LAYOUT_GENERAL;
		}
		if (texture_layout.m_input_attachment)
		{
			usage += VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		}
		if (texture_layout.m_is_renderable)
		{
			if (texture.m_is_color)
			{
				usage += VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			}
			else
			{
				usage += VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			}
		}

		VK_instance::This->CreateImage(texture, texture.m_image_type, texture.m_format, texture_extent, texture.m_texture_descriptor.m_num_levels, num_layers, VK_IMAGE_TILING_OPTIMAL, usage | transfer_src_bit, image_create_flag_bits);

		VkCommandBuffer setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();
		VK_instance::This->set_image_layout(
			setup_cmd, texture.m_image,
			VK_IMAGE_LAYOUT_PREINITIALIZED, layout,
			texture.m_aspect, 0, texture.m_texture_descriptor.m_num_levels, 0, num_layers);

		VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);

		texture.m_last_layout = layout;
	}

	if (texture_layout.m_type != NGL_RENDERBUFFER)
	{
		// Create the shader resource view
		{
			VkResult err;
			VkImageViewCreateInfo view_info = {
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				NULL,
#ifdef VK_USE_PLATFORM_SCREEN_QNX
				0,
#else
				VK_NULL_HANDLE,
#endif
				texture.m_image,
				texture.m_image_view_type,
				texture.m_format,
				{ VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A, },
				{ 0, 0, 1, 0, 1 }
			};

			view_info.subresourceRange.levelCount = texture.m_texture_descriptor.m_num_levels;

			// TODO: Mipmapping: Set level count
			switch (texture_layout.m_type)
			{
			case NGL_TEXTURE_2D:
				break;

			case NGL_TEXTURE_2D_ARRAY:
				view_info.subresourceRange.layerCount = texture_layout.m_num_array;
				break;
			case NGL_TEXTURE_CUBE:
				view_info.subresourceRange.layerCount = texture_layout.m_num_array * 6;
				break;

			default:
				_logf("Unknown texture type: %d", texture_layout.m_type);
				assert(0);
			}

			view_info.subresourceRange.aspectMask = texture.m_aspect;

			err = vkCreateImageView(DEVICE, &view_info, 0, &texture.m_resource_view_all);
			LOGVKERROR(err);

			for (uint32_t i = 0; i < texture.m_texture_descriptor.m_num_levels; i++)
			{
				VkImageView view;

				view_info.subresourceRange.baseMipLevel = i;
				view_info.subresourceRange.levelCount = 1;

				err = vkCreateImageView(DEVICE, &view_info, 0, &view);
				LOGVKERROR(err);

				texture.m_resource_views.push_back(view);
			}
		}

		// Create render target views
		if (texture_layout.m_is_renderable)
		{
			VkResult err;
			VkImageViewCreateInfo view_info = {
				VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				NULL,
#ifdef VK_USE_PLATFORM_SCREEN_QNX
				0,
#else
				VK_NULL_HANDLE,
#endif
				texture.m_image,
				VK_IMAGE_VIEW_TYPE_2D,
				texture.m_format,
				{ VK_COMPONENT_SWIZZLE_R,
				VK_COMPONENT_SWIZZLE_G,
				VK_COMPONENT_SWIZZLE_B,
				VK_COMPONENT_SWIZZLE_A, },
				{ 0, 0, 1, 0, 1 }
			};

			view_info.subresourceRange.aspectMask = texture.m_aspect;

			uint32_t num_subresources = GetNumSubresources(texture_layout);

			texture.m_render_target_views.resize(num_subresources);

			for (uint32_t i = 0; i < num_subresources; i++)
			{
				DecomposeSubresource(texture.m_texture_descriptor, i, view_info.subresourceRange.baseMipLevel, view_info.subresourceRange.baseArrayLayer);

				err = vkCreateImageView(DEVICE, &view_info, 0, &texture.m_render_target_views[i]);
				LOGVKERROR(err);
			}
		}
		{
			VkSamplerCreateInfo sampler_info;

			C(sampler_info, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);

			sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

			switch (texture_layout.m_filter)
			{
			case NGL_NEAREST:
			{
				sampler_info.magFilter = VK_FILTER_NEAREST;
				sampler_info.minFilter = VK_FILTER_NEAREST;
				sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				sampler_info.maxAnisotropy = 1;
				sampler_info.maxLod = 0.0f;
				break;
			}
			case NGL_LINEAR:
			{
				sampler_info.magFilter = VK_FILTER_LINEAR;
				sampler_info.minFilter = VK_FILTER_LINEAR;
				sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				sampler_info.maxAnisotropy = 1;
				sampler_info.maxLod = 0.0f;
				break;
			}
			case NGL_NEAREST_MIPMAPPED:
			{
				sampler_info.magFilter = VK_FILTER_NEAREST;
				sampler_info.minFilter = VK_FILTER_NEAREST;
				sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
				sampler_info.maxAnisotropy = 1;
				sampler_info.maxLod = (float)texture.m_texture_descriptor.m_num_levels;
				break;
			}
			case NGL_LINEAR_MIPMAPPED:
			{
				sampler_info.magFilter = VK_FILTER_LINEAR;
				sampler_info.minFilter = VK_FILTER_LINEAR;
				sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				sampler_info.maxAnisotropy = 1;
				sampler_info.maxLod = (float)texture.m_texture_descriptor.m_num_levels;
				break;
			}
			case NGL_ANISO_4:
			{
				sampler_info.magFilter = VK_FILTER_LINEAR;
				sampler_info.minFilter = VK_FILTER_LINEAR;
				sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				sampler_info.maxAnisotropy = 4.0f;
				sampler_info.maxLod = (float)texture.m_texture_descriptor.m_num_levels;
				sampler_info.anisotropyEnable = true;
				break;
			}
			default:
				_logf("Unkown texture filter type: %d", texture_layout.m_filter);
				break;
			}

			switch (texture_layout.m_wrap_mode)
			{
			case NGL_REPEAT_ALL:
			{
				sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				break;
			}
			case NGL_CLAMP_ALL:
			{
				sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				break;
			}
			default:
				_logf("Unkown texture wrap mode: %d", texture_layout.m_wrap_mode);
				break;
			}

			texture.m_samplers[0] = VK_instance::This->GetSampler(sampler_info);

			if (!texture.m_is_color)
			{
				sampler_info.magFilter = shadow_filter;
				sampler_info.minFilter = shadow_filter;
				sampler_info.compareOp = VK_COMPARE_OP_LESS;
				sampler_info.compareEnable = 1;

				texture.m_samplers[1] = VK_instance::This->GetSampler(sampler_info);
			}
		}
	}
	return true;
}


bool GenVertexBuffer(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data)
{
	if (buffer && buffer >= VK_instance::This->m_vertex_buffers.size())
	{
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		size_t old_size = VK_instance::This->m_vertex_buffers.size();

		VK_instance::This->m_vertex_buffers.resize(old_size + 1);
		buffer = (uint32_t)old_size;
	}

	VK_vertex_buffer &vb = VK_instance::This->m_vertex_buffers[buffer];

	vb.m_hash = GenerateHash(vertex_layout);
	vb.m_vertex_descriptor = vertex_layout;
	vb.m_datasize = num * vertex_layout.m_stride;

	if (vertex_layout.m_unordered_access)
	{
		VK_instance::This->CreateSSBO(vb);
	}
	else
	{
		VK_instance::This->CreateVBO(vb, data);
	}

	return true;
}


bool GenIndexBuffer(uint32_t &buffer, NGL_format format, uint32_t num, void *data)
{
	if (buffer && buffer >= VK_instance::This->m_index_buffers.size())
	{
		buffer = 0xBADF00D;
		return false;
	}

	if (!buffer)
	{
		size_t old_size = VK_instance::This->m_index_buffers.size();

		VK_instance::This->m_index_buffers.resize(old_size + 1);
		buffer = (uint32_t)old_size;
	}

	VK_index_buffer &ib = VK_instance::This->m_index_buffers[buffer];

	ib.m_format = format;
	ib.m_num_indices = num;

	uint32_t stride = 0;
	switch (format)
	{
	case NGL_R16_UINT:
	{
		ib.m_data_type = VK_INDEX_TYPE_UINT16;
		stride = 2;
		break;
	}
	case NGL_R32_UINT:
	{
		ib.m_data_type = VK_INDEX_TYPE_UINT32;
		stride = 4;
		break;
	}
	default:
		assert(0);
	}

	ib.m_datasize = num * stride;

	VK_instance::This->CreateIBO(ib, data);

	return true;
}


static void CreatePasses(std::vector<VK_pass> &passes, NGL_job_descriptor &jd)
{
	std::vector<bool> is_cleared(jd.m_attachments.size(), false);

	passes.resize(jd.m_subpasses.size());

	for (size_t sp_idx = 0; sp_idx < jd.m_subpasses.size(); sp_idx++)
	{
		NGL_subpass &src_sp = jd.m_subpasses[sp_idx];
		VK_pass &dst_pass = passes[sp_idx];

		dst_pass.m_subpasses.resize(1);

		VK_subpass &sp = dst_pass.m_subpasses[0];
		bool need_clear = false;

		sp.m_name = src_sp.m_name;

		VkSubpassDescription SubpassDescription;
		std::vector<VkAttachmentReference> input_attachment_references;
		std::vector<VkAttachmentReference> color_attachment_references;
		std::vector<VkAttachmentReference> depth_attachment_references;
		std::vector<uint32_t> preserve_attachment_references;
		std::vector<VkAttachmentDescription> attachment_descriptions;
		std::vector<VkImageView> attachment_bind_infos;

		for (size_t reference = 0; reference < src_sp.m_usages.size(); reference++)
		{
			const NGL_resource_state &f = src_sp.m_usages[reference];
			const NGL_attachment_descriptor &atd = jd.m_attachments[reference];
			const VK_texture &t = VK_instance::This->m_textures[atd.m_attachment.m_idx];

			if (
				f == NGL_COLOR_ATTACHMENT ||
				f == NGL_DEPTH_ATTACHMENT ||
				f == NGL_READ_ONLY_DEPTH_ATTACHMENT ||
				f == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE
				)
			{
				VkAttachmentDescription attachment_description;
				VkClearValue attachment_clear_value;
				VkAttachmentReference r;

				attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
				attachment_description.flags = 0;
				attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

				if (!is_cleared[reference])
				{
					switch (atd.m_attachment_load_op)
					{
					case NGL_LOAD_OP_LOAD:
					{
						attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
						break;
					}
					case NGL_LOAD_OP_DONT_CARE:
					{
						attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
						break;
					}
					case NGL_LOAD_OP_CLEAR:
					{
						attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
						need_clear = true;
						break;
					}
					}

					is_cleared[reference] = true;
				}
				else
				{
					attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				}

				if (sp_idx == jd.m_subpasses.size() - 1)
				{
					switch (atd.m_attachment_store_op)
					{
					case NGL_STORE_OP_STORE:
					{
						attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
						break;
					}
					case NGL_STORE_OP_DONT_CARE:
					{
						attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
						break;
					}
					}
				}
				else
				{
					attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				}

				r.attachment = (uint32_t)attachment_descriptions.size();

				if (f == NGL_COLOR_ATTACHMENT)
				{
					attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					r.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

					color_attachment_references.push_back(r);
					sp.m_color_attachments_remap.push_back((uint32_t)reference);
				}
				if (f == NGL_DEPTH_ATTACHMENT)
				{
					attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

					depth_attachment_references.push_back(r);
					sp.m_depth_attachments_remap.push_back((uint32_t)reference);
				}
				if (f == NGL_READ_ONLY_DEPTH_ATTACHMENT || f == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
				{
					attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
					r.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

					depth_attachment_references.push_back(r);
					sp.m_depth_attachments_remap.push_back((uint32_t)reference);
				}

				attachment_clear_value.color.float32[0] = t.m_texture_descriptor.m_clear_value[0];
				attachment_clear_value.color.float32[1] = t.m_texture_descriptor.m_clear_value[1];
				attachment_clear_value.color.float32[2] = t.m_texture_descriptor.m_clear_value[2];
				attachment_clear_value.color.float32[3] = t.m_texture_descriptor.m_clear_value[3];
				attachment_clear_value.depthStencil.depth = t.m_texture_descriptor.m_clear_value[0];

				dst_pass.m_renderArea.offset.x = 0;
				dst_pass.m_renderArea.offset.y = 0;
				dst_pass.m_renderArea.extent.width = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
				dst_pass.m_renderArea.extent.height = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

				if (t.m_texture_descriptor.m_format > NGL_UNDEFINED)
				{
					if (!t.m_texture_descriptor.m_is_renderable)
					{
						_logf("not renderable\n");
					}
					else
					{
						attachment_description.format = t.m_format;
						attachment_bind_infos.push_back(t.m_render_target_views[GetSubresourceId(t.m_texture_descriptor, atd.m_attachment.m_level, atd.m_attachment.m_layer, atd.m_attachment.m_face)]);
					}
				}
				else
				{
					assert(0);
				}

				attachment_descriptions.push_back(attachment_description);
				dst_pass.m_attachment_clearvalues.push_back(attachment_clear_value);
			}
		}

		if (!need_clear)
		{
			dst_pass.m_attachment_clearvalues.clear();
		}

		SubpassDescription.flags = 0;
		SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		SubpassDescription.inputAttachmentCount = (uint32_t)input_attachment_references.size();
		SubpassDescription.pInputAttachments = input_attachment_references.data();
		SubpassDescription.colorAttachmentCount = (uint32_t)color_attachment_references.size();
		SubpassDescription.pColorAttachments = color_attachment_references.data();
		SubpassDescription.pResolveAttachments = 0;
		SubpassDescription.pDepthStencilAttachment = depth_attachment_references.data();
		SubpassDescription.preserveAttachmentCount = (uint32_t)preserve_attachment_references.size();
		SubpassDescription.pPreserveAttachments = preserve_attachment_references.data();

		VkResult err;

		VkRenderPassCreateInfo rp_info;
		C(rp_info, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);

		rp_info.attachmentCount = (uint32_t)attachment_descriptions.size();
		rp_info.pAttachments = &attachment_descriptions[0];
		rp_info.subpassCount = 1;
		rp_info.pSubpasses = &SubpassDescription;

		err = vkCreateRenderPass(DEVICE, &rp_info, 0, &dst_pass.m_renderpass);
		LOGVKERROR(err);

		VkFramebufferCreateInfo fb_info;
		C(fb_info, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

		fb_info.attachmentCount = (uint32_t)attachment_bind_infos.size();
		fb_info.pAttachments = attachment_bind_infos.data();
		fb_info.width = dst_pass.m_renderArea.extent.width;
		fb_info.height = dst_pass.m_renderArea.extent.height;
		fb_info.layers = 1;
		fb_info.renderPass = dst_pass.m_renderpass;

		err = vkCreateFramebuffer(DEVICE, &fb_info, 0, &dst_pass.m_framebuffer);
		LOGVKERROR(err);
	}
}


static void CreateSubpasses(std::vector<VK_pass> &passes, NGL_job_descriptor &jd)
{
	std::vector<VkImageView> attachment_bind_infos;
	std::vector<VkAttachmentDescription> attachment_descriptions;

	passes.resize(1);

	VK_pass &dst_pass = passes[0];
	bool need_clear = false;

	for (size_t i = 0; i < jd.m_attachments.size(); i++)
	{
		NGL_attachment_descriptor &atd = jd.m_attachments[i];
		VK_texture &t = VK_instance::This->m_textures[atd.m_attachment.m_idx];

		VkAttachmentDescription attachment_description;
		VkClearValue attachment_clear_value;

		attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment_description.flags = 0;
		attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		switch (atd.m_attachment_load_op)
		{
		case NGL_LOAD_OP_LOAD:
		{
			attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			break;
		}
		case NGL_LOAD_OP_DONT_CARE:
		{
			attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			break;
		}
		case NGL_LOAD_OP_CLEAR:
		{
			attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			need_clear = true;
			break;
		}
		}
		switch (atd.m_attachment_store_op)
		{
		case NGL_STORE_OP_STORE:
		{
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			break;
		}
		case NGL_STORE_OP_DONT_CARE:
		{
			attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		}
		}

		const NGL_resource_state &f0 = jd.m_subpasses.front().m_usages[i];

		switch (f0)
		{
		case NGL_COLOR_ATTACHMENT:
		case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
		{
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		}
		case NGL_DEPTH_ATTACHMENT:
		{
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		}
		case NGL_READ_ONLY_DEPTH_ATTACHMENT:
		case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
		{
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			break;
		}
		default:
			assert(0);
		}

		const NGL_resource_state &f1 = jd.m_subpasses.back().m_usages[i];
		switch (f1)
		{
		case NGL_COLOR_ATTACHMENT:
		case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
		{
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			break;
		}
		case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
		{
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			break;
		}
		case NGL_DEPTH_ATTACHMENT:
		{
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			break;
		}
		case NGL_READ_ONLY_DEPTH_ATTACHMENT:
		case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
		{
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
			break;
		}
		default:
			assert(0);
		}

		attachment_clear_value.color.float32[0] = t.m_texture_descriptor.m_clear_value[0];
		attachment_clear_value.color.float32[1] = t.m_texture_descriptor.m_clear_value[1];
		attachment_clear_value.color.float32[2] = t.m_texture_descriptor.m_clear_value[2];
		attachment_clear_value.color.float32[3] = t.m_texture_descriptor.m_clear_value[3];
		attachment_clear_value.depthStencil.depth = t.m_texture_descriptor.m_clear_value[0];

		dst_pass.m_renderArea.offset.x = 0;
		dst_pass.m_renderArea.offset.y = 0;
		dst_pass.m_renderArea.extent.width = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
		dst_pass.m_renderArea.extent.height = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

		if (t.m_texture_descriptor.m_format > NGL_UNDEFINED)
		{
			if (!t.m_texture_descriptor.m_is_renderable)
			{
				_logf("not renderable\n");
			}
			else
			{
				attachment_description.format = t.m_format;
				attachment_bind_infos.push_back(t.m_render_target_views[GetSubresourceId(t.m_texture_descriptor, atd.m_attachment.m_level, atd.m_attachment.m_layer, atd.m_attachment.m_face)]);
			}
		}
		else
		{
			assert(0);
		}

		attachment_descriptions.push_back(attachment_description);
		dst_pass.m_attachment_clearvalues.push_back(attachment_clear_value);
	}

	if (!need_clear)
	{
		dst_pass.m_attachment_clearvalues.clear();
	}

	std::vector<VkAttachmentReference> input_attachment_references[8];
	std::vector<VkAttachmentReference> color_attachment_references[8];
	std::vector<VkAttachmentReference> depth_attachment_references[8];
	std::vector<uint32_t> preserve_attachment_references[8];

	dst_pass.m_subpasses.resize(jd.m_subpasses.size());

	{
		std::vector<VkSubpassDescription> SubpassDescriptions;
		std::vector<VkSubpassDependency> SubpassDependencies;

		SubpassDescriptions.resize(jd.m_subpasses.size());
		SubpassDependencies.resize(jd.m_subpasses.size() - 1);

		for (size_t sp_idx = 0; sp_idx < jd.m_subpasses.size(); sp_idx++)
		{
			NGL_subpass &src_sp = jd.m_subpasses[sp_idx];
			VK_subpass &dst_sp = dst_pass.m_subpasses[sp_idx];
			VkSubpassDescription &SubpassDescription = SubpassDescriptions[sp_idx];

			VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			VkAccessFlags src_flags = 0;
			VkAccessFlags dst_flags = 0;

			for (size_t j = 0; j < src_sp.m_usages.size(); j++)
			{
				const NGL_resource_state &f = src_sp.m_usages[j];

				VkAccessFlags attachment_src_flags = 0;
				VkAccessFlags attachment_dst_flags = 0;

				if (sp_idx)
				{
					const NGL_resource_state &prev_f = jd.m_subpasses[sp_idx - 1].m_usages[j];

					RM_src(src_stages, attachment_src_flags, prev_f);
					RM_src(dst_stages, attachment_dst_flags, f);

					// Workaround: only gbuffer_depth uses this state. If within a multipass, it is an input attachment, not a shader resource.
					if (sp_idx > 0 && prev_f == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
					{
						attachment_src_flags = (attachment_src_flags & ~VK_ACCESS_SHADER_READ_BIT) | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
					}
				}

				switch (f)
				{
				case NGL_COLOR_ATTACHMENT:
				{
					VkAttachmentReference r;

					r.attachment = (uint32_t)j;
					r.layout = GetImageLayout(f);
					color_attachment_references[sp_idx].push_back(r);
					dst_sp.m_color_attachments_remap.push_back((uint32_t)j);
					break;
				}
				case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
				{
					VkAttachmentReference r;

					r.attachment = (uint32_t)j;
					r.layout = GetImageLayout(f);
					input_attachment_references[sp_idx].push_back(r);
					break;
				}
				case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
				{
					preserve_attachment_references[sp_idx].push_back((uint32_t)j);
					break;
				}
				case NGL_DEPTH_ATTACHMENT:
				{
					VkAttachmentReference r;

					r.attachment = (uint32_t)j;
					r.layout = GetImageLayout(f);
					depth_attachment_references[sp_idx].push_back(r);
					dst_sp.m_depth_attachments_remap.push_back((uint32_t)j);
					break;
				}
				case NGL_READ_ONLY_DEPTH_ATTACHMENT:
				case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
				{
					VkAttachmentReference r;

					r.attachment = (uint32_t)j;
					r.layout = GetImageLayout(f);
					input_attachment_references[sp_idx].push_back(r);
					depth_attachment_references[sp_idx].push_back(r);
					dst_sp.m_depth_attachments_remap.push_back((uint32_t)j);

					// Workaround: only gbuffer_depth uses this state. If within a multipass, it is an input attachment, not a shader resource.
					if (sp_idx > 0 && f == NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE)
					{
						attachment_dst_flags = (attachment_dst_flags & ~VK_ACCESS_SHADER_READ_BIT) | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

						VK_texture &texture = VK_instance::This->m_textures[jd.m_attachments[j].m_attachment.m_idx];
						dst_pass.m_texture_that_was_input_attachment = &texture;
					}
					break;
				}
				default:
					assert(0);
				}

				src_flags |= attachment_src_flags;
				dst_flags |= attachment_dst_flags;
			}

			SubpassDescription.flags = 0;
			SubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			SubpassDescription.inputAttachmentCount = (uint32_t)input_attachment_references[sp_idx].size();
			SubpassDescription.pInputAttachments = input_attachment_references[sp_idx].data();
			SubpassDescription.colorAttachmentCount = (uint32_t)color_attachment_references[sp_idx].size();
			SubpassDescription.pColorAttachments = color_attachment_references[sp_idx].data();
			SubpassDescription.pResolveAttachments = 0;
			SubpassDescription.pDepthStencilAttachment = depth_attachment_references[sp_idx].data();
			SubpassDescription.preserveAttachmentCount = (uint32_t)preserve_attachment_references[sp_idx].size();
			SubpassDescription.pPreserveAttachments = preserve_attachment_references[sp_idx].data();

			if (sp_idx)
			{
				VkSubpassDependency &dependency = SubpassDependencies[sp_idx - 1];

				dependency.srcSubpass = (uint32_t)sp_idx - 1;
				dependency.dstSubpass = (uint32_t)sp_idx;
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				dependency.srcStageMask = src_stages;
				dependency.dstStageMask = dst_stages;
				dependency.srcAccessMask = src_flags;
				dependency.dstAccessMask = dst_flags;
			}
		}

		VkResult err;

		VkRenderPassCreateInfo rp_info;
		C(rp_info, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);

		rp_info.attachmentCount = (uint32_t)attachment_descriptions.size();
		rp_info.pAttachments = &attachment_descriptions[0];
		rp_info.subpassCount = (uint32_t)SubpassDescriptions.size();
		rp_info.pSubpasses = SubpassDescriptions.data();
		rp_info.dependencyCount = (uint32_t)SubpassDependencies.size();
		rp_info.pDependencies = SubpassDependencies.data();

		err = vkCreateRenderPass(DEVICE, &rp_info, 0, &dst_pass.m_renderpass);
		LOGVKERROR(err);

		VkFramebufferCreateInfo fb_info;
		C(fb_info, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

		fb_info.attachmentCount = (uint32_t)attachment_bind_infos.size();
		fb_info.pAttachments = attachment_bind_infos.data();
		fb_info.width = dst_pass.m_renderArea.extent.width;
		fb_info.height = dst_pass.m_renderArea.extent.height;
		fb_info.layers = 1;
		fb_info.renderPass = dst_pass.m_renderpass;

		err = vkCreateFramebuffer(DEVICE, &fb_info, 0, &dst_pass.m_framebuffer);
		LOGVKERROR(err);
	}
}

uint32_t GenJob(NGL_job_descriptor &descriptor)
{
	VK_job *job = new VK_job(VK_instance::This->m_swapchain_length);
	job->m_descriptor = descriptor;

	if (!descriptor.m_load_shader_callback)
	{
		_logf("!!!error: m_load_shader_callback is not set for %s.\n", descriptor.m_subpasses[0].m_name.c_str());
	}

	if (!descriptor.m_is_compute)
	{
		if (descriptor.m_subpasses.size() == 1 && descriptor.m_attachments.size() == 1 && descriptor.m_attachments[0].m_attachment.m_idx == 0)
		{
			job->m_passes.resize(1);

			NGL_attachment_descriptor &atd = descriptor.m_attachments[0];
			VK_texture &t = VK_instance::This->m_textures[atd.m_attachment.m_idx];
			VK_pass &pass = job->m_passes[0];

			pass.m_subpasses.resize(1);

			VK_subpass &sp = pass.m_subpasses[0];
			bool need_clear = false;

			sp.m_name = descriptor.m_subpasses[0].m_name;

			VkAttachmentDescription attachment_description;
			VkAttachmentReference color_attachment_reference;
			VkClearValue attachment_clear_value;

			attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment_description.flags = 0;
			attachment_description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment_description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			switch (atd.m_attachment_load_op)
			{
			case NGL_LOAD_OP_LOAD:
			{
				attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				break;
			}
			case NGL_LOAD_OP_DONT_CARE:
			{
				attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				break;
			}
			case NGL_LOAD_OP_CLEAR:
			{
				attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				need_clear = true;
				break;
			}
			}
			switch (atd.m_attachment_store_op)
			{
			case NGL_STORE_OP_STORE:
			{
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				break;
			}
			case NGL_STORE_OP_DONT_CARE:
			{
				attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				break;
			}
			}
			attachment_description.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment_description.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			attachment_clear_value.color.float32[0] = t.m_texture_descriptor.m_clear_value[0];
			attachment_clear_value.color.float32[1] = t.m_texture_descriptor.m_clear_value[1];
			attachment_clear_value.color.float32[2] = t.m_texture_descriptor.m_clear_value[2];
			attachment_clear_value.color.float32[3] = t.m_texture_descriptor.m_clear_value[3];
			attachment_clear_value.depthStencil.depth = t.m_texture_descriptor.m_clear_value[0];
			pass.m_attachment_clearvalues.push_back(attachment_clear_value);
			if (!need_clear)
			{
				pass.m_attachment_clearvalues.clear();
			}

			attachment_description.format = VK_instance::This->m_surface_color_format;

			color_attachment_reference.attachment = 0;
			color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			sp.m_color_attachments_remap.push_back(0);

			pass.m_renderArea.offset.x = 0;
			pass.m_renderArea.offset.y = 0;
			pass.m_renderArea.extent.width = t.m_texture_descriptor.m_size[0] / (1 << atd.m_attachment.m_level);
			pass.m_renderArea.extent.height = t.m_texture_descriptor.m_size[1] / (1 << atd.m_attachment.m_level);

			VkSubpassDescription subpass =
			{
				0,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				0,
				NULL,
				1,
				&color_attachment_reference,
				NULL,
				0,
				0,
				NULL
			};

			VkResult err;

			VkRenderPassCreateInfo rp_info;
			C(rp_info, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);

			rp_info.attachmentCount = 1;
			rp_info.pAttachments = &attachment_description;
			rp_info.subpassCount = 1;
			rp_info.pSubpasses = &subpass;
			rp_info.dependencyCount = 0;
			rp_info.pDependencies = 0;

			err = vkCreateRenderPass(DEVICE, &rp_info, 0, &pass.m_renderpass);
			LOGVKERROR(err);

			const int count = (int)VK_instance::This->m_swapchain_length;
			for (int sc = 0; sc < count; sc++)
			{
				VkFramebufferCreateInfo fb_info;
				C(fb_info, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);

				fb_info.attachmentCount = 1;
				fb_info.pAttachments = &VK_instance::This->m_swapchain_image_views[sc];
				fb_info.width = pass.m_renderArea.extent.width;
				fb_info.height = pass.m_renderArea.extent.height;
				fb_info.layers = 1;
				fb_info.renderPass = pass.m_renderpass;

				err = vkCreateFramebuffer(DEVICE, &fb_info, 0, &job->m_swapchain_framebuffers[sc]);
				LOGVKERROR(err);
			}

			job->m_onscreen = true;
		}
		else
		{
			if (VK_instance::This->m_propertiesi[NGL_SUBPASS_ENABLED])
			{
				CreateSubpasses(job->m_passes, descriptor);
			}
			else
			{
				CreatePasses(job->m_passes, descriptor);
			}
		}
	}
	else
	{
		job->m_passes.resize(1);
		VK_pass &pass = job->m_passes[0];

		pass.m_subpasses.resize(1);
		VK_subpass &sp = pass.m_subpasses[0];

		sp.m_name = descriptor.m_subpasses[0].m_name;
	}

	job->m_idx = (uint32_t)VK_instance::This->m_jobs.size();

	VK_instance::This->m_jobs.push_back(job);

	return (uint32_t)VK_instance::This->m_jobs.size() - 1;
}


void ViewportScissor(uint32_t job_, int32_t viewport[4], int32_t scissor[4])
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	if (viewport)
	{
		memcpy(job->m_current_state.m_viewport, viewport, sizeof(int32_t) * 4);
	}
	if (scissor)
	{
		memcpy(job->m_current_state.m_scissor, scissor, sizeof(int32_t) * 4);
	}
}


void LineWidth(uint32_t job_, float width)
{
	//VK_job *job = VK_instance::This->m_jobs[job_];

	//glLineWidth(width);
}


void BlendState(uint32_t job_, uint32_t attachment, NGL_blend_func func, NGL_color_channel_mask mask)
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	job->m_current_state.m_blend_state.m_funcs[attachment] = func;
	job->m_current_state.m_blend_state.m_masks[attachment] = mask;
}


void DepthState(uint32_t job_, NGL_depth_func func, bool mask)
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	job->m_current_state.m_depth_state.m_func = func;
	job->m_current_state.m_depth_state.m_mask = mask;
}


void Begin(uint32_t job_, uint32_t idx)
{
	VK_job *job = VK_instance::This->m_jobs[job_];
	VK_cmd_buffer &cb = VK_instance::This->m_cmd_buffers[idx];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	job->cb = cb.cb;
	job->m_ubo_pool = cb.m_ubo_pool;
	job->m_cmd_buffer_idx = idx;
	job->Reset1();

	if (VK_instance::This->m_statistics_enabled)
	{
		ResetQuery(job->cb, job);
	}

	if (job->m_onscreen)
	{
		cb.m_first_onscreen = !VK_instance::This->m_have_acquired_back_buffer;
		job->m_passes[0].m_framebuffer = job->m_swapchain_framebuffers[VK_instance::This->BeginFrame()];

		if (!cb.m_needs_transition_to_present)
		{
			BackBuffer &buf = VK_instance::This->m_back_buffers[VK_instance::This->m_actual_back_buffer_index];
			VK_instance::This->set_image_layout(cb.cb, VK_instance::This->m_swapchain_images[buf.image_index], VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
			cb.m_needs_transition_to_present = true;
		}
	}

	job->m_current_state.m_subpass = 0;

	memset(&job->m_previous_state, ~0, sizeof(NGL_state));

	if (!descriptor.m_is_compute)
	{
		VkRenderPassBeginInfo rp_begin =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			NULL,
			job->m_passes[0].m_renderpass,
			job->m_passes[0].m_framebuffer,
			job->m_passes[0].m_renderArea,
			(uint32_t)job->m_passes[0].m_attachment_clearvalues.size(),
			job->m_passes[0].m_attachment_clearvalues.data()
		};

		vkCmdBeginRenderPass(job->cb, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);
	}
}


void NextSubpass(uint32_t job_)
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	job->m_current_state.m_subpass++;

	if (job->m_passes.size() == 1)
	{
		vkCmdNextSubpass(job->cb, VK_SUBPASS_CONTENTS_INLINE);
	}
	else
	{
		vkCmdEndRenderPass(job->cb);

		{
			std::vector<NGL_texture_subresource_transition> texture_barriers;
			std::vector<NGL_buffer_transition> buffer_barriers;

			NGL_subpass &prev_sp = job->m_descriptor.m_subpasses[job->m_current_state.m_subpass - 1];
			NGL_subpass &cur_sp = job->m_descriptor.m_subpasses[job->m_current_state.m_subpass];

			for (size_t i = 0; i < job->m_descriptor.m_attachments.size(); i++)
			{
				NGL_resource_state prev_state = prev_sp.m_usages[i];
				NGL_resource_state cur_state = cur_sp.m_usages[i];

				if (prev_state != cur_state)
				{
					NGL_texture_subresource_transition t(job->m_descriptor.m_attachments[i].m_attachment, prev_state, cur_state);

					texture_barriers.push_back(t);
				}
			}

			nglBarrier(job->m_cmd_buffer_idx, texture_barriers, buffer_barriers);
		}

		VkRenderPassBeginInfo rp_begin =
		{
			VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			NULL,
			job->m_passes[job->m_current_state.m_subpass].m_renderpass,
			job->m_passes[job->m_current_state.m_subpass].m_framebuffer,
			job->m_passes[job->m_current_state.m_subpass].m_renderArea,
			(uint32_t)job->m_passes[job->m_current_state.m_subpass].m_attachment_clearvalues.size(),
			job->m_passes[job->m_current_state.m_subpass].m_attachment_clearvalues.data()
		};

		vkCmdBeginRenderPass(job->cb, &rp_begin, VK_SUBPASS_CONTENTS_INLINE);

		memset(&job->m_previous_state, ~0, sizeof(NGL_state));
	}
}


void End(uint32_t job_)
{
	VK_job *job = VK_instance::This->m_jobs[job_];
	const NGL_job_descriptor &descriptor = job->m_descriptor;

	if (!descriptor.m_is_compute)
	{
		vkCmdEndRenderPass(job->cb);
	}

	// Workaround: only gbuffer_depth uses this state. If within a multipass, it is an input attachment, not a shader resource.
	if (job->m_passes.size() > 0 && job->m_passes[0].m_subpasses.size() > 1 && job->m_passes[0].m_texture_that_was_input_attachment != nullptr)
	{
		job->m_passes[0].m_texture_that_was_input_attachment->m_was_input_attachment = true;
	}
}


void BeginQuery(uint32_t job_)
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	job_statistics &stat = (*VK_instance::This->m_statistic_vector).jobs[job_];
	stat.m_is_active = true;
	stat.m_sub_pass[job->m_current_state.m_subpass].m_draw_calls.push_back(draw_call_statistics());

	vkCmdBeginQuery(job->cb, job->m_query_pool_occlusion, job->m_draw_call_counter, 0);
	vkCmdBeginQuery(job->cb, job->m_query_pool, job->m_draw_call_counter, 0);
}


void ResetQuery(VkCommandBuffer cb, VK_job *job)
{
	const int pipeline_query_count = 2000;
	const int occlusion_query_count = 2000;

	vkCmdResetQueryPool(cb, job->m_query_pool, 0, pipeline_query_count);
	vkCmdResetQueryPool(cb, job->m_query_pool_occlusion, 0, occlusion_query_count);
}


void EndQuery(uint32_t job_)
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	vkCmdEndQuery(job->cb, job->m_query_pool_occlusion, job->m_draw_call_counter);
	vkCmdEndQuery(job->cb, job->m_query_pool, job->m_draw_call_counter);

	job->m_draw_call_counter++;
}


void BeginStatistic(NGLStatistic& statistic_vector)
{
	if (VK_instance::This->m_propertiesi[NGL_PIPELINE_STATISTICS] == 0)
	{
		return;
	}

	VK_instance::This->m_statistic_vector = &statistic_vector;
	VK_instance::This->m_statistics_enabled = true;

	(*VK_instance::This->m_statistic_vector).jobs.resize(VK_instance::This->m_jobs.size());

	for (size_t i = 0; i < VK_instance::This->m_jobs.size(); i++)
	{
		VK_job *job = (VK_instance::This->m_jobs[i]);

		const NGL_job_descriptor &jd = job->m_descriptor;

		(*VK_instance::This->m_statistic_vector).jobs[i].m_sub_pass.resize(jd.m_subpasses.size());
		(*VK_instance::This->m_statistic_vector).jobs[i].Clear();

		for (size_t j = 0; j < jd.m_subpasses.size(); j++)
		{
			(*VK_instance::This->m_statistic_vector).jobs[i].m_sub_pass[j].m_name = jd.m_subpasses[j].m_name;
		}

		job->m_draw_call_counter = 0;

		const int pipeline_query_count = 2000;
		const int occlusion_query_count = 2000;

		if (!job->m_query_pool)
		{
			// 1. For pipeline statistics
			VkQueryPoolCreateInfo queryPoolInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
			queryPoolInfo.queryCount = pipeline_query_count;
			queryPoolInfo.queryType = VK_QUERY_TYPE_PIPELINE_STATISTICS;
			queryPoolInfo.pipelineStatistics =
				VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT |
				VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
				VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT |
				VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
				VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT |
				VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT;


			VkResult err = vkCreateQueryPool(DEVICE, &queryPoolInfo, 0, &job->m_query_pool);
			LOGVKERROR(err);

			// 2. For occlusion queries
			VkQueryPoolCreateInfo queryPoolInfoOcclusion = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
			queryPoolInfoOcclusion.queryCount = occlusion_query_count;
			queryPoolInfoOcclusion.queryType = VK_QUERY_TYPE_OCCLUSION;
			err = vkCreateQueryPool(DEVICE, &queryPoolInfoOcclusion, 0, &job->m_query_pool_occlusion);
			LOGVKERROR(err);
		}
	}

	VK_instance::This->m_statistic_vector->m_used_textures.clear();
}


void EndStatistic()
{
	if (VK_instance::This->m_propertiesi[NGL_PIPELINE_STATISTICS] == 0)
	{
		return;
	}

	VK_instance::This->m_statistics_enabled = false;
}


void GetStatistic()
{
	if (VK_instance::This->m_propertiesi[NGL_PIPELINE_STATISTICS] == 0)
	{
		return;
	}

	NGLStatistic *ngl_stat = VK_instance::This->m_statistic_vector;

	const int switch_table[NGL_STATISTIC_COUNT] =
	{
		1,
		0,
		2,
		-1, -1, -1, -1,
		5,
		3,
		4,
		-1, -1, -1
	};


	for (size_t h = 0; h < (*VK_instance::This->m_statistic_vector).jobs.size();++h)
	{
		VK_job *job = VK_instance::This->m_jobs[h];
		job_statistics& stat = (*VK_instance::This->m_statistic_vector).jobs[h];
		if (!stat.m_is_active)
		{
			continue;
		}

		uint32_t draw_call_counter = 0;

		for (size_t k = 0; k < stat.m_sub_pass.size(); k++)
		{
			for (size_t j = 0; j < stat.m_sub_pass[k].m_draw_calls.size(); j++)
			{
				draw_call_statistics *draw_stat = &stat.m_sub_pass[k].m_draw_calls[j];

				uint32_t result[6];
				uint32_t resultOcclusion = 0;
				memset(result, 0, sizeof(result));

				VkQueryResultFlags numBits = VK_QUERY_RESULT_WAIT_BIT;// | VK_QUERY_RESULT_64_BIT;

				VkResult err = vkGetQueryPoolResults(DEVICE, job->m_query_pool_occlusion, draw_call_counter, 1, sizeof(uint32_t) * 1, &resultOcclusion, 0, numBits);
				if (err != VK_SUCCESS)
				{
					_logf("error for vkGetQueryPoolResults: %d", err);
				}

				err = vkGetQueryPoolResults(DEVICE, job->m_query_pool, draw_call_counter, 1, sizeof(uint32_t) * 6, result, 0, numBits);
				if (err != VK_SUCCESS)
				{
					_logf("error for vkGetQueryPoolResults: %d", err);
				}

				for (uint32_t i = 0; i < NGL_STATISTIC_COUNT; i++)
				{
					if (!ngl_stat->queries_enabled[i])
					{
						continue;
					}

					if (i == NGL_STATISTIC_SAMPLES_PASSED)
					{
						draw_stat->m_query_results[i] = resultOcclusion;
					}
					else
					{
						// Get the NGL index from the Vulkan index
						const int result_idx = switch_table[i];
						if (result_idx < 0)
						{
							// Not supported query
							draw_stat->m_query_results[i] = 0;
						}
						else
						{
							draw_stat->m_query_results[i] = result[result_idx];
						}
					}
				}

				draw_call_counter++;
			}
		}
	}

	VK_instance::This->m_statistic_vector = nullptr;
}


void Draw(uint32_t job_, NGL_primitive_type primitive_type, uint32_t shader_code, uint32_t num_vbos, uint32_t *vbos, uint32_t ebo, NGL_cull_mode cull_type, const void** parameters)
{
	VK_job *job = VK_instance::This->m_jobs[job_];
	const VK_subpass &current_subpass = job->G();
	uint32_t uniform_update_mask = NGL_GROUP_PER_DRAW;

	job->m_current_state.m_cull_mode = cull_type;
	job->m_current_state.m_primitive_type = primitive_type;
	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;
	for (uint32_t i = 0; i<num_vbos; i++)
	{
		job->m_current_state.m_shader.m_vbo_hash += VK_instance::This->m_vertex_buffers[vbos[i]].m_hash;
	}

	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);

	if (changed_mask & NGL_HEAVY_STATES)
	{
		NGL_renderer *renderer = 0;

		for (size_t j = 0; j < job->m_renderers.size(); j++)
		{
			if (memcmp(&job->m_renderers[j]->m_my_state, &job->m_current_state, sizeof(NGL_state) - sizeof(int32_t) * 8) == 0)
			{
				renderer = job->m_renderers[j];
				break;
			}
		}

		if (!renderer)
		{
			renderer = job->CreateRenderer(job->m_current_state, num_vbos, vbos);
		}

		job->m_active_renderer = renderer;

		vkCmdBindPipeline(job->cb, VK_PIPELINE_BIND_POINT_GRAPHICS, ((VK_renderer *)renderer)->m_pipeline);

		uniform_update_mask = NGL_GROUP_PER_DRAW | NGL_GROUP_PER_RENDERER_CHANGE | NGL_GROUP_MANUAL;
	}

	if (changed_mask & NGL_VIEWPORT_MASK || job->m_previous_state.m_depth_state.m_func == NGL_DEPTH_TO_FAR || job->m_current_state.m_depth_state.m_func == NGL_DEPTH_TO_FAR)
	{
		VkViewport viewport;

		viewport.x = (float)job->m_current_state.m_viewport[0];
		viewport.y = (float)job->m_current_state.m_viewport[1];
		viewport.width = (float)job->m_current_state.m_viewport[2];
		viewport.height = (float)job->m_current_state.m_viewport[3];
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		if (job->m_current_state.m_depth_state.m_func == NGL_DEPTH_TO_FAR)
		{
			viewport.minDepth = 1.0f;
		}

		vkCmdSetViewport(job->cb, 0, 1, &viewport);
	}
	if (changed_mask & NGL_SCISSOR_MASK)
	{
		VkRect2D rect;

		rect.offset.x = job->m_current_state.m_scissor[0];
		rect.offset.y = job->m_current_state.m_scissor[1];
		rect.extent.width = job->m_current_state.m_scissor[2];
		rect.extent.height = job->m_current_state.m_scissor[3];

		vkCmdSetScissor(job->cb, 0, 1, &rect);
	}

	VK_renderer *renderer = (VK_renderer *)job->m_active_renderer;

	std::vector<VkDescriptorSet> &used_DescriptorSets = VK_instance::This->cached_used_DescriptorSets;
	std::vector<VkWriteDescriptorSet> &used_WriteDescriptorSets = VK_instance::This->cached_used_WriteDescriptorSets;

	used_DescriptorSets.clear();
	used_WriteDescriptorSets.clear();

	VkDescriptorSet *descriptor_set_of_this_draw = renderer->m_num_used_uniform_groups_with_descriptor_sets ? renderer->UseNextSet(job->m_cmd_buffer_idx) : 0;

	for (uint32_t g = 0; g < 2; g++)
	{
		VK_uniform_group &group = renderer->m_uniform_groups[g];

		if (uniform_update_mask & (1 << g))
		{
			if (descriptor_set_of_this_draw)
			{
				VK_ubo &UBO = job->GetUBO(g);

				if (group.m_aligned_memory_size)
				{
					UBO.m_minor_offset = 0;
				}

				for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
				{
					if (group.m_ubo_memory_sizes[shader_type])
					{
						group.SetUpUBO(shader_type, UBO);
					}
				}
			}

			for (size_t i = 0; i < renderer->m_used_uniforms[g].size(); i++)
			{
				const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];

				if (uu.m_application_location > -1)
				{
					if (parameters[uu.m_application_location])
					{
						group.BindUniform(job, uu, parameters[uu.m_application_location]);
					}
					else
					{
						_logf("not set uniform: %s in %s - %u\n", uu.m_uniform.m_name.c_str(), current_subpass.m_name.c_str(), renderer->m_my_state.m_shader.m_shader_code);
					}
				}
			}

			if (descriptor_set_of_this_draw)
			{
				VK_ubo &UBO = job->GetUBO(g);

				if (group.m_aligned_memory_size)
				{
					UBO.m_major_offset += group.m_aligned_memory_size;
				}

				if (group.m_descriptor_set.m_WriteDescriptorSet.size())
				{
					used_DescriptorSets.push_back(*descriptor_set_of_this_draw);

					for (size_t i = 0; i < group.m_descriptor_set.m_WriteDescriptorSet.size(); i++)
					{
						group.m_descriptor_set.m_WriteDescriptorSet[i].dstSet = *descriptor_set_of_this_draw;

						used_WriteDescriptorSets.push_back(group.m_descriptor_set.m_WriteDescriptorSet[i]);
					}

					descriptor_set_of_this_draw++;
				}
			}

			for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				if (renderer->m_uniform_groups[g].m_range[shader_type].size)
				{
					vkCmdPushConstants(job->cb, renderer->m_pipeline_layout,
						renderer->m_uniform_groups[g].m_range[shader_type].stageFlags,
						renderer->m_uniform_groups[g].m_range[shader_type].offset,
						renderer->m_uniform_groups[g].m_range[shader_type].size,
						(const void*)renderer->m_uniform_groups[g].m_data[shader_type]);
				}
			}
		}
	}

	if (used_WriteDescriptorSets.size())
	{
		vkUpdateDescriptorSets(DEVICE, (uint32_t)used_WriteDescriptorSets.size(), used_WriteDescriptorSets.data(), 0, 0);
		vkCmdBindDescriptorSets(job->cb, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->m_pipeline_layout, 0, (uint32_t)used_DescriptorSets.size(), used_DescriptorSets.data(), 0, 0);
	}

	if (num_vbos)
	{
		VkDeviceSize vbo_offsets[1] = { 0 };
		VK_vertex_buffer &vb = VK_instance::This->m_vertex_buffers[vbos[0]];
		vkCmdBindVertexBuffers(job->cb, 0, 1, &vb.m_buffer, vbo_offsets);
	}

	VK_index_buffer &ib = VK_instance::This->m_index_buffers[ebo];
	vkCmdBindIndexBuffer(job->cb, ib.m_buffer, 0, ib.m_data_type);

	if (VK_instance::This->m_statistics_enabled)
	{
		BeginQuery(job_);
	}

	vkCmdDrawIndexed(job->cb, ib.m_num_indices, 1, 0, 0, 0);

	if (VK_instance::This->m_statistics_enabled)
	{
		EndQuery(job_);
	}

	job->m_previous_state = job->m_current_state;
}


bool Dispatch(uint32_t job_, uint32_t shader_code, uint32_t x, uint32_t y, uint32_t z, const void** parameters)
{
	VK_job *job = VK_instance::This->m_jobs[job_];
	const VK_subpass &current_subpass = job->G();
	uint32_t uniform_update_mask = NGL_GROUP_PER_DRAW;

	job->m_current_state.m_shader.m_shader_code = shader_code;
	job->m_current_state.m_shader.m_vbo_hash = 0;

	uint32_t changed_mask = NGL_state::ChangedMask(job->m_current_state, job->m_previous_state);

	if (changed_mask & NGL_HEAVY_STATES)
	{
		NGL_renderer *renderer = 0;

		for (size_t j = 0; j < job->m_renderers.size(); j++)
		{
			if (memcmp(&job->m_renderers[j]->m_my_state, &job->m_current_state, sizeof(NGL_state) - sizeof(int32_t) * 8) == 0)
			{
				renderer = job->m_renderers[j];
				break;
			}
		}

		if (!renderer)
		{
			renderer = job->CreateRenderer(job->m_current_state, 0, 0);
		}

		job->m_active_renderer = renderer;

		vkCmdBindPipeline(job->cb, VK_PIPELINE_BIND_POINT_COMPUTE, ((VK_renderer *)renderer)->m_pipeline);

		uniform_update_mask = NGL_GROUP_PER_DRAW | NGL_GROUP_PER_RENDERER_CHANGE | NGL_GROUP_MANUAL;
	}
	VK_renderer *renderer = (VK_renderer *)job->m_active_renderer;

	std::vector<VkDescriptorSet> &used_DescriptorSets = VK_instance::This->cached_used_DescriptorSets;
	std::vector<VkWriteDescriptorSet> &used_WriteDescriptorSets = VK_instance::This->cached_used_WriteDescriptorSets;

	used_DescriptorSets.clear();
	used_WriteDescriptorSets.clear();

	VkDescriptorSet *descriptor_set_of_this_draw = renderer->m_num_used_uniform_groups_with_descriptor_sets ? renderer->UseNextSet(job->m_cmd_buffer_idx) : 0;

	for (uint32_t g = 0; g < 2; g++)
	{
		VK_uniform_group &group = renderer->m_uniform_groups[g];

		if (uniform_update_mask & (1 << g))
		{
			if (descriptor_set_of_this_draw)
			{
				VK_ubo &UBO = job->GetUBO(g);

				if (group.m_aligned_memory_size)
				{
					UBO.m_minor_offset = 0;
				}

				uint32_t shader_type = NGL_COMPUTE_SHADER;
				{
					if (group.m_ubo_memory_sizes[shader_type])
					{
						group.SetUpUBO(shader_type, UBO);
					}
				}
			}

			for (size_t i = 0; i < renderer->m_used_uniforms[g].size(); i++)
			{
				const NGL_used_uniform &uu = renderer->m_used_uniforms[g][i];

				if (uu.m_application_location > -1)
				{
					if (parameters[uu.m_application_location])
					{
						group.BindUniform(job, uu, parameters[uu.m_application_location]);
					}
					else
					{
						_logf("not set uniform: %s in %s - %u\n", uu.m_uniform.m_name.c_str(), current_subpass.m_name.c_str(), renderer->m_my_state.m_shader.m_shader_code);
					}
				}
			}

			if (descriptor_set_of_this_draw)
			{
				VK_ubo &UBO = job->GetUBO(g);

				if (group.m_aligned_memory_size)
				{
					UBO.m_major_offset += group.m_aligned_memory_size;
				}

				if (group.m_descriptor_set.m_WriteDescriptorSet.size())
				{
					used_DescriptorSets.push_back(*descriptor_set_of_this_draw);

					for (size_t i = 0; i < group.m_descriptor_set.m_WriteDescriptorSet.size(); i++)
					{
						group.m_descriptor_set.m_WriteDescriptorSet[i].dstSet = *descriptor_set_of_this_draw;

						used_WriteDescriptorSets.push_back(group.m_descriptor_set.m_WriteDescriptorSet[i]);
					}

					descriptor_set_of_this_draw++;
				}
			}

			for (uint32_t shader_type = NGL_VERTEX_SHADER; shader_type < NGL_NUM_SHADER_TYPES; shader_type++)
			{
				if (renderer->m_uniform_groups[g].m_range[shader_type].size)
				{
					vkCmdPushConstants(job->cb, renderer->m_pipeline_layout,
						renderer->m_uniform_groups[g].m_range[shader_type].stageFlags,
						renderer->m_uniform_groups[g].m_range[shader_type].offset,
						renderer->m_uniform_groups[g].m_range[shader_type].size,
						(const void*)renderer->m_uniform_groups[g].m_data[shader_type]);
				}
			}
		}
	}

	if (used_WriteDescriptorSets.size())
	{
		vkUpdateDescriptorSets(DEVICE, (uint32_t)used_WriteDescriptorSets.size(), used_WriteDescriptorSets.data(), 0, 0);
		vkCmdBindDescriptorSets(job->cb, VK_PIPELINE_BIND_POINT_COMPUTE, renderer->m_pipeline_layout, 0, (uint32_t)used_DescriptorSets.size(), used_DescriptorSets.data(), 0, 0);
	}

	vkCmdDispatch(job->cb, x, y, z);

	job->m_previous_state = job->m_current_state;

	return true;
}


const char* GetString(NGL_backend_property prop)
{
	return VK_instance::This->m_propertiess[prop].c_str();
}

int32_t GetInteger(NGL_backend_property prop)
{
	return VK_instance::This->m_propertiesi[prop];
}


void DeletePipelines(uint32_t job_)
{
	VK_job *job = VK_instance::This->m_jobs[job_];

	job->DeleteRenderers();
}


void CustomAction(uint32_t job_, uint32_t parameter)
{
	if (parameter == NGL_CUSTOM_ACTION_SWAPBUFFERS)
	{
		VK_instance::This->EndFrame();
	}
	if (parameter == 100)
	{
	}
	if (parameter == 101)
	{
	}
	if (parameter == 102)
	{
	}
	if (parameter == 200)
	{
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0] = 1.0f;
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1] = 0.0f;
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2] = 0.0f;
	}
	if (parameter == 201)
	{
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0] = 0.0f;
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1] = 1.0f;
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2] = 0.0f;
	}
	if (parameter == 202)
	{
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[0] = 0.0f;
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[1] = 0.0f;
		VK_instance::This->m_textures[0].m_texture_descriptor.m_clear_value[2] = 1.0f;
	}
}


void SetLineWidth(uint32_t job_, float width)
{
}

#if 0
static void RM(VkPipelineStageFlags &PipelineStageFlags, VkAccessFlags &AccessFlags, VkImageLayout &ImageLayout, const NGL_resource_state &s);
#endif

bool GetTextureContent(uint32_t texture_, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data_)
{
	width = 0;
	height = 0;

	VkPipelineStageFlags src_stages = 0;
	VkAccessFlags a = 0;

	VkImageLayout src_image_layout = GetImageLayout(state);
	if (texture_ == 0)
	{
		// assumes system color attachment is always returned to present state between command buffers
		src_image_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	}

	VkImage src_image = VK_NULL_HANDLE;
	VK_staging_texture staging_tex = { 0 };

	RM_src(src_stages, a, state);

	//rgba or bgra
	bool is_rgba = false;

	{
		if (texture_)
		{
			VK_texture &texture = VK_instance::This->m_textures[texture_];
			src_image = texture.m_image;

			if (!texture.m_texture_descriptor.m_is_transfer_source)
			{
				_logf("GetTextureContent(): Texture is not a transfer source");
				assert(false);
			}

			staging_tex.m_format = texture.m_format;
			staging_tex.m_extent.width = texture.m_texture_descriptor.m_size[0] / (1 << level);
			staging_tex.m_extent.height = texture.m_texture_descriptor.m_size[1] / (1 << level);
		}
		else
		{
			BackBuffer &buf = VK_instance::This->m_back_buffers[VK_instance::This->m_actual_back_buffer_index];
			src_image = VK_instance::This->m_swapchain_images[buf.image_index];

			if (!VK_instance::This->m_context_descriptor.m_system_attachment_is_transfer_source)
			{
				_logf("GetTextureContent(): System attachment is not a transfer source");
				assert(false);
			}

			staging_tex.m_format = VK_instance::This->m_surface_color_format;
			staging_tex.m_extent.width = VK_instance::This->m_context_descriptor.m_display_width;// texture.m_texture_descriptor.m_size[0] / (1 << m);
			staging_tex.m_extent.height = VK_instance::This->m_context_descriptor.m_display_height;//texture.m_texture_descriptor.m_size[1] / (1 << m);
		}

		staging_tex.m_extent.width = staging_tex.m_extent.width == 0 ? 1 : staging_tex.m_extent.width;
		staging_tex.m_extent.height = staging_tex.m_extent.height == 0 ? 1 : staging_tex.m_extent.height;
		staging_tex.m_extent.depth = 1;

		is_rgba = staging_tex.m_format == VK_FORMAT_R8G8B8A8_UNORM;

		if (staging_tex.m_format != VK_FORMAT_R8G8B8A8_UNORM && staging_tex.m_format != VK_FORMAT_B8G8R8A8_UNORM )
		{
			_logf("staging_texture.m_format != VK_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM");
			return false;
		}

		if (format != NGL_R8_G8_B8_A8_UNORM)
		{
			_logf("format != NGL_R8_G8_B8_A8_UNORM");
			return false;
		}

		VK_instance::This->CreateStagingTexture(staging_tex, VK_IMAGE_USAGE_TRANSFER_DST_BIT);
	}
	{
		VkCommandBuffer setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();

		VK_instance::This->set_image_layout(
			setup_cmd, src_image,
			src_image_layout, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1, 0, 1);

		VK_instance::This->set_image_layout(
			setup_cmd, staging_tex.m_image,
			VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1, 0, 1);

		VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);
	}
	{
		VkImageCopy copy_region;

		copy_region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.srcSubresource.mipLevel = (uint32_t)level;
		copy_region.srcSubresource.baseArrayLayer = (uint32_t)layer;
		copy_region.srcSubresource.layerCount = 1;
		copy_region.srcOffset.x = 0;
		copy_region.srcOffset.y = 0;
		copy_region.srcOffset.z = 0;

		copy_region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		copy_region.dstSubresource.mipLevel = 0;
		copy_region.dstSubresource.baseArrayLayer = 0;
		copy_region.dstSubresource.layerCount = 1;
		copy_region.dstOffset.x = 0;
		copy_region.dstOffset.y = 0;
		copy_region.dstOffset.z = 0;

		copy_region.extent = staging_tex.m_extent;

		VkCommandBuffer setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();

		vkCmdCopyImage(setup_cmd, src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, staging_tex.m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

		VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);
	}
	{
		VkCommandBuffer setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();

		VK_instance::This->set_image_layout(
			setup_cmd, src_image,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, src_image_layout,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1, 0, 1);

		VK_instance::This->set_image_layout(
			setup_cmd, staging_tex.m_image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
			VK_IMAGE_ASPECT_COLOR_BIT,
			0, 1, 0, 1);

		VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);
	}

	VkResult err;
	uint8_t *mapped_data = 0;

	err = vkMapMemory(DEVICE, staging_tex.m_mem, 0, VK_WHOLE_SIZE, 0, (void **)&mapped_data);
	LOGVKERROR(err);

	VkMappedMemoryRange range;
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.pNext = NULL;
	range.memory = staging_tex.m_mem;
	range.offset = 0;
	range.size = VK_WHOLE_SIZE;
	err = vkInvalidateMappedMemoryRanges(DEVICE, 1, &range);
	LOGVKERROR(err);

	if (mapped_data)
	{
		uint32_t &thwidth = width;
		uint32_t &thheight = height;
		uint8_t *thpixels = NULL;
		uint32_t src_stride = 4;
		uint32_t dst_stride = 4;
		VkImageSubresource r;
		VkSubresourceLayout layout;

		r.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		r.mipLevel = 0;
		r.arrayLayer = 0;

		vkGetImageSubresourceLayout(DEVICE, staging_tex.m_image, &r, &layout);

		uint8_t *data = mapped_data;

		data += layout.offset;

		float widthf = float(staging_tex.m_extent.width);
		float heightf = float(staging_tex.m_extent.height);

		float aspect = widthf / heightf;

		thwidth = staging_tex.m_extent.width;
		thwidth &= ~0x7; // align down to multiple of 8
		thheight = uint32_t(float(thwidth) / aspect);

		data_.resize(dst_stride * thwidth * thheight);
		thpixels = data_.data();

		uint8_t *dst = thpixels;

		for (uint32_t y = thheight-1; (int32_t)y >= 0; y--)
		{
			for (uint32_t x = 0; x < thwidth; x++)
			{
				//float xf = float(x) / float(thwidth);
				//float yf = float(y) / float(thheight);

				//uint8_t *src = &data[src_stride*uint32_t(xf*widthf) + layout.rowPitch*uint32_t(yf*heightf)];
				uint8_t *src = &data[src_stride*x + layout.rowPitch*y];

				if( is_rgba )
				{
					dst[0] = src[0];
					dst[1] = src[1];
					dst[2] = src[2];
				}
				else
				{
					dst[0] = src[2];
					dst[1] = src[1];
					dst[2] = src[0];
				}

				dst[3] = src[3];

				dst += dst_stride;
			}
		}

		vkDestroyImage(DEVICE, staging_tex.m_image, 0);
		vkFreeMemory(DEVICE, staging_tex.m_mem, 0);
	}

	return true;
}


bool GetVertexBufferContent(uint32_t buffer_id, NGL_resource_state state, std::vector<uint8_t> &data)
{
	VK_vertex_buffer &vb = VK_instance::This->m_vertex_buffers[buffer_id];
	VkResult err = VK_SUCCESS;

	VkBufferUsageFlagBits buffer_type_bit = vb.m_vertex_descriptor.m_unordered_access ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	VK_staging_buffer buffer;
	VkBufferCopy buffer_copy_region = { 0, 0, vb.m_datasize };
	VK_instance::This->CreateStagingBuffer(buffer, vb.m_datasize, buffer_type_bit | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	VkCommandBuffer setup_cmd = VK_instance::This->BeginTemporaryCmdBuffer();
	vkCmdCopyBuffer(setup_cmd, vb.m_buffer, buffer.m_buffer, 1, &buffer_copy_region);
	VK_instance::This->EndTemporaryCmdBuffer(setup_cmd);

	uint8_t *mapped_data = nullptr;
	err = vkMapMemory(DEVICE, buffer.m_mem, 0, vb.m_datasize, 0, (void **)&mapped_data);

	if (err != VK_SUCCESS)
	{
		LOGVKERROR(err);
		return false;
	}

	VkMappedMemoryRange range;
	range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	range.pNext = NULL;
	range.memory = buffer.m_mem;
	range.offset = 0;
	range.size = vb.m_datasize;
	err = vkInvalidateMappedMemoryRanges(DEVICE, 1, &range);
	LOGVKERROR(err);

	data.resize(vb.m_datasize);
	memcpy(&data[0], mapped_data, vb.m_datasize);

	vkUnmapMemory(DEVICE, buffer.m_mem);

	vkDestroyBuffer(DEVICE, buffer.m_buffer, 0);
	vkFreeMemory(DEVICE, buffer.m_mem, 0);

	return true;
}


bool ResizeTextures(uint32_t num_textures, uint32_t *textures, uint32_t size[3])
{
	return false;
}


void BeginCommandBuffer(uint32_t idx)
{
	VkResult err;

	VK_cmd_buffer &cb = VK_instance::This->m_cmd_buffers[idx];

	err = vkWaitForFences(DEVICE, 1, &cb.f, true, UINT64_MAX);
	LOGVKERROR(err);

	err = vkResetFences(DEVICE, 1, &cb.f);
	LOGVKERROR(err);

	if (!VK_instance::This->m_have_coherent_ubo)
	{
		err = vkInvalidateMappedMemoryRanges(DEVICE, 2, cb.m_ubo_ranges);
		LOGVKERROR(err);
	}

	VkCommandBufferInheritanceInfo cmd_buf_hinfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
		NULL,
		VK_NULL_HANDLE,
		0,
		VK_NULL_HANDLE,
		VK_FALSE,
		0,
		0,
	};

	VkCommandBufferBeginInfo CmdBufferBeginInfo =
	{
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		&cmd_buf_hinfo,
	};

	err = vkResetCommandBuffer(cb.cb, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
	LOGVKERROR(err);

	err = vkBeginCommandBuffer(cb.cb, &CmdBufferBeginInfo);
	LOGVKERROR(err);

	cb.m_first_onscreen = false;
	cb.m_ubo_pool[0].m_major_offset = 0;
	cb.m_ubo_pool[1].m_major_offset = 0;

	cb.m_needs_transition_to_present = false;
}


void EndCommandBuffer(uint32_t idx)
{
	VkResult err;

	VK_cmd_buffer &cb = VK_instance::This->m_cmd_buffers[idx];

	if (cb.m_needs_transition_to_present)
	{
		BackBuffer &buf = VK_instance::This->m_back_buffers[VK_instance::This->m_actual_back_buffer_index];
		VK_instance::This->set_image_layout(cb.cb, VK_instance::This->m_swapchain_images[buf.image_index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);
		cb.m_needs_transition_to_present = false;
	}

	err = vkEndCommandBuffer(cb.cb);
	LOGVKERROR(err);
}


void SubmitCommandBuffer(uint32_t idx)
{
	VkResult err;

	VK_cmd_buffer &cb = VK_instance::This->m_cmd_buffers[idx];

	BackBuffer &buf = VK_instance::This->m_back_buffers[VK_instance::This->m_actual_back_buffer_index];

	VkSubmitInfo primary_cmd_submit_info;
	VkPipelineStageFlags primary_cmd_submit_wait_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	C(primary_cmd_submit_info, VK_STRUCTURE_TYPE_SUBMIT_INFO);
	primary_cmd_submit_info.commandBufferCount = 1;
	primary_cmd_submit_info.pCommandBuffers = &cb.cb;
	if (cb.m_first_onscreen)
	{
		primary_cmd_submit_info.waitSemaphoreCount = 1;
		primary_cmd_submit_info.pWaitSemaphores = &buf.acquire_semaphore;
		primary_cmd_submit_info.pWaitDstStageMask = &primary_cmd_submit_wait_stages;
		primary_cmd_submit_info.signalSemaphoreCount = 1;
		primary_cmd_submit_info.pSignalSemaphores = &buf.render_semaphore;
	}

	if (!VK_instance::This->m_have_coherent_ubo)
	{
		err = vkFlushMappedMemoryRanges(DEVICE, 2, cb.m_ubo_ranges);
		LOGVKERROR(err);
	}

	err = vkQueueSubmit(VK_instance::This->m_queue, 1, &primary_cmd_submit_info, cb.f);
	LOGVKERROR(err);
}


void Flush()
{
	//NOP
}


void Finish()
{
	VkResult err;

	for (uint32_t i = 0; i < NUM_MAX_CMD_BUFFERS; i++)
	{
		VK_cmd_buffer &cb = VK_instance::This->m_cmd_buffers[i];

		err = vkWaitForFences(DEVICE, 1, &cb.f, true, UINT64_MAX);
		LOGVKERROR(err);
	}

	err = vkQueueWaitIdle(VK_instance::This->m_queue);
	LOGVKERROR(err);

	err = vkDeviceWaitIdle(DEVICE);
	LOGVKERROR(err);
}

#if 0
static void RM(VkPipelineStageFlags &PipelineStageFlags, VkAccessFlags &AccessFlags, VkImageLayout &ImageLayout, const NGL_resource_state &s)
{
	AccessFlags = 0;

	switch (s)
	{
	case NGL_COLOR_ATTACHMENT:
	case NGL_COLOR_ATTACHMENT_AND_PRESERVED_ATTACHMENT:
	{
		ImageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		AccessFlags = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;
	}
	case NGL_DEPTH_ATTACHMENT:
	{
		ImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		AccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;
	}
	case NGL_READ_ONLY_DEPTH_ATTACHMENT_AND_SHADER_RESOURCE:
	case NGL_READ_ONLY_DEPTH_ATTACHMENT:
	{
		ImageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		AccessFlags = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
		break;
	}
	case NGL_COLOR_ATTACHMENT_AND_INPUT_ATTACHMENT_AND_SHADER_RESOURCE:
	case NGL_SHADER_RESOURCE:
	{
		ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		AccessFlags = VK_ACCESS_SHADER_READ_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}
	case NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE:
	{
		ImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		AccessFlags = VK_ACCESS_SHADER_READ_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}
	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS:
	{
		ImageLayout = VK_IMAGE_LAYOUT_GENERAL;
		AccessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}
	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE:
	{
		ImageLayout = VK_IMAGE_LAYOUT_GENERAL;
		AccessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		break;
	}
	}
}
#endif


static void GetBufferAccessFlags(VkPipelineStageFlags &PipelineStageFlags, VkAccessFlags &AccessFlags, NGL_resource_state state)
{
	switch (state)
	{
	case NGL_SHADER_RESOURCE:
	{
		AccessFlags = VK_ACCESS_SHADER_READ_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}
	case NGL_SHADER_RESOURCE_AND_NON_FRAGMENT_SHADER_RESOURCE:
	{
		AccessFlags = VK_ACCESS_SHADER_READ_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		break;
	}
	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS:
	{
		AccessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		break;
	}
	case NGL_SHADER_RESOURCE_AND_UNORDERED_ACCESS_AND_NON_FRAGMENT_SHADER_RESOURCE:
	{
		AccessFlags = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		PipelineStageFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		break;
	}
	default:
		_logf("Unhandled buffer resource state: %d", state);
		assert(0);
	}
}


void Barrier(uint32_t cmd_buffer, std::vector<NGL_texture_subresource_transition> &texture_barriers, std::vector<NGL_buffer_transition> &buffer_barriers)
{
	NGL_texture_barrier_merger<VK_texture> tbm(texture_barriers, VK_instance::This->m_textures);

	std::vector<VkImageMemoryBarrier> &vk_image_barriers = VK_instance::This->cached_vk_image_barriers;
	std::vector<VkBufferMemoryBarrier> &vk_buffer_barriers = VK_instance::This->cached_vk_buffer_barriers;

	vk_image_barriers.resize(tbm.m_ranges.size());
	vk_buffer_barriers.clear();

	VK_cmd_buffer &cb = VK_instance::This->m_cmd_buffers[cmd_buffer];
	VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dst_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	VkImageMemoryBarrier* image_memory_barrier = vk_image_barriers.data();

	for (std::vector<NGL_texture_subresource_transition_range>::iterator i = tbm.m_ranges.begin(); i != tbm.m_ranges.end(); i++)
	{
		VK_texture &texture = VK_instance::This->m_textures[i->m_texture.m_idx];

		C(*image_memory_barrier, VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER);

		image_memory_barrier->image = texture.m_image;
		image_memory_barrier->subresourceRange.aspectMask = texture.m_aspect;
		image_memory_barrier->subresourceRange.baseMipLevel = i->m_base_level;
		image_memory_barrier->subresourceRange.levelCount = i->m_num_levels;
		image_memory_barrier->subresourceRange.baseArrayLayer = i->m_base_face;
		image_memory_barrier->subresourceRange.layerCount = i->m_num_faces;
		image_memory_barrier->srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		image_memory_barrier->dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		image_memory_barrier->oldLayout = GetImageLayout(i->m_old_state);
		RM_src(src_stages, image_memory_barrier->srcAccessMask, i->m_old_state);

		// Workaround: if m_was_input_attachment is true, the texture is the gbuffer_depth and it came from a multipass. If within a multipass, it is an input attachment, not a shader resource.
		if (texture.m_was_input_attachment)
		{
			image_memory_barrier->srcAccessMask = (image_memory_barrier->srcAccessMask & ~VK_ACCESS_SHADER_READ_BIT) | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
			texture.m_was_input_attachment = false;
		}

		image_memory_barrier->newLayout = GetImageLayout(i->m_new_state);
		RM_src(dst_stages, image_memory_barrier->dstAccessMask, i->m_new_state);

		texture.m_last_layout = image_memory_barrier->newLayout;

		image_memory_barrier++;
	}

	for (size_t i = 0; i < buffer_barriers.size(); i++)
	{
		const NGL_buffer_transition &buffer_transition = buffer_barriers[i];

		const VK_vertex_buffer &buffer = VK_instance::This->m_vertex_buffers[buffer_transition.m_idx];

		VkBufferMemoryBarrier buffer_barrier;

		C(buffer_barrier, VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER);

		buffer_barrier.buffer = buffer.m_buffer;
		GetBufferAccessFlags(src_stages, buffer_barrier.srcAccessMask, buffer_transition.m_old_state);
		GetBufferAccessFlags(dst_stages, buffer_barrier.dstAccessMask, buffer_transition.m_new_state);
		buffer_barrier.offset = 0;
		buffer_barrier.size = VK_WHOLE_SIZE;
		buffer_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		buffer_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vk_buffer_barriers.push_back(buffer_barrier);
	}

	uint32_t image_barrier_count = (uint32_t)vk_image_barriers.size();
	uint32_t buffer_barrier_count = (uint32_t)vk_buffer_barriers.size();

	if (image_barrier_count || buffer_barrier_count)
	{
		VkImageMemoryBarrier *image_barriers_ptr = image_barrier_count ? vk_image_barriers.data() : NULL;
		VkBufferMemoryBarrier *buffer_barriers_ptr = buffer_barrier_count ? vk_buffer_barriers.data() : NULL;

		vkCmdPipelineBarrier(cb.cb, src_stages, dst_stages, 0, 0, NULL, buffer_barrier_count, buffer_barriers_ptr, image_barrier_count, image_barriers_ptr);
	}
}


void DestroyContext()
{
	for (size_t i = 0; i < VK_instance::This->m_jobs.size(); i++)
	{
		VK_job *job = VK_instance::This->m_jobs[i];
		for (size_t j = 0; j < job->m_renderers.size(); j++)
		{
			//VK_renderer *renderer = (VK_renderer *)job->m_renderers[j];
			//printf("%s - %d: %d\n", job->m_descriptor.m_name.c_str(), j, renderer->m_desc_pools.size());
		}
	}

	delete VK_instance::This;
	VK_instance::This = 0;
}


VK_instance *VK_instance::This = 0;


VK_instance::VK_instance()
{
	This = this;

	nglBeginCommandBuffer = BeginCommandBuffer;
	nglEndCommandBuffer = EndCommandBuffer;
	nglSubmitCommandBuffer = SubmitCommandBuffer;
	nglGenJob = GenJob;
	nglBegin = Begin;
	nglNextSubpass = NextSubpass;
	nglEnd = End;
	nglBlendState = BlendState;
	nglDepthState = DepthState;
	nglDraw = Draw;
	nglDispatch = Dispatch;
	nglGenTexture = GenTexture;
	nglGenVertexBuffer = GenVertexBuffer;
	nglGenIndexBuffer = GenIndexBuffer;
	nglViewportScissor = ViewportScissor;
	nglGetString = GetString;
	nglGetInteger = GetInteger;
	nglDeletePipelines = DeletePipelines;
	nglCustomAction = CustomAction;
	nglLineWidth = LineWidth;
	nglGetTextureContent = GetTextureContent;
	nglGetVertexBufferContent = GetVertexBufferContent;
	nglResizeTextures = ResizeTextures;
	nglFlush = Flush;
	nglFinish = Finish;
	nglDestroyContext = DestroyContext;
	nglBarrier = Barrier;
	nglBeginStatistic = BeginStatistic;
	nglEndStatistic = EndStatistic;
	nglGetStatistic = GetStatistic;

	m_swap_chain = VK_NULL_HANDLE;
	m_vsync = false;
	m_actual_back_buffer_index = 0;
	m_swapchain_length = 0;
	m_have_acquired_back_buffer = false;
	m_have_coherent_ubo = false;
	{
		//a nullas attachment invalid, mert a m_format == 0
		VK_texture t;

		m_textures.push_back(t);
	}
	fpCreateDebugReportCallbackEXT = nullptr;
	fpDestroyDebugReportCallbackEXT = nullptr;
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	fpCreateWaylandSurfaceKHR = nullptr;
	fpGetPhysicalDeviceWaylandPresentationSupportKHR = nullptr;
#endif

#ifdef VK_USE_PLATFORM_SCREEN_QNX
	fpCreateScreenSurfaceQNX = nullptr;
	fpGetPhysicalDeviceScreenPresentationSupportQNX = nullptr;
#endif

	m_statistic_vector = nullptr;
	m_statistics_enabled = false;
}


VK_instance::~VK_instance()
{
	Finish();

	for (std::map<VkSamplerCreateInfo, VkSampler, SamplerComparator>::iterator i = m_samplers.begin(); i != m_samplers.end(); i++)
	{
		vkDestroySampler(m_device, i->second, nullptr);
	}

	m_samplers.clear();

	//count from 1 because it reserved
	for (size_t i = 1; i < m_textures.size(); i++)
	{
		vkDestroyImageView(m_device, m_textures[i].m_resource_view_all, NULL);
		vkDestroyImage(m_device, m_textures[i].m_image, NULL);
		vkFreeMemory(m_device, m_textures[i].m_mem, NULL);

		for (size_t j = 0; j < m_textures[i].m_resource_views.size(); ++j)
		{
			vkDestroyImageView(m_device, m_textures[i].m_resource_views[j], nullptr);
		}
		for (size_t j = 0; j < m_textures[i].m_render_target_views.size(); ++j)
		{
			vkDestroyImageView(m_device, m_textures[i].m_render_target_views[j], nullptr);
		}
	}

	for (size_t i = 0; i < m_vertex_buffers.size(); i++)
	{
		vkDestroyBuffer(m_device, m_vertex_buffers[i].m_buffer, nullptr);
		if (m_vertex_buffers[i].m_mem)
		{
			vkFreeMemory(m_device, m_vertex_buffers[i].m_mem, nullptr);
		}
	}
	for (size_t i = 0; i < m_index_buffers.size(); i++)
	{
		vkDestroyBuffer(m_device, m_index_buffers[i].m_buffer, nullptr);
		if (m_index_buffers[i].m_mem)
		{
			vkFreeMemory(m_device, m_index_buffers[i].m_mem, nullptr);
		}
	}

	for (size_t i = 0; i < m_vertex_memory.size(); i++)
	{
		vkFreeMemory(m_device, m_vertex_memory[i].m_memory, nullptr);
	}
	for (size_t i = 0; i < m_vertex_index_memory.size(); i++)
	{
		vkFreeMemory(m_device, m_vertex_index_memory[i].m_memory, nullptr);
	}

	for (size_t i = 0; i < m_swapchain_images.size(); i++)
	{
		vkDestroyImageView(m_device, m_swapchain_image_views[i], NULL);
	}

	for (size_t i = 0; i < m_cmd_buffers.size(); i++)
	{
		VK_cmd_buffer &b = m_cmd_buffers[i];

		vkDestroyFence(m_device, b.f, nullptr);

		vkUnmapMemory(m_device, b.m_ubo_pool[0].m_mem);
		vkDestroyBuffer(m_device, b.m_ubo_pool[0].m_buffer, nullptr);
		vkFreeMemory(m_device, b.m_ubo_pool[0].m_mem, nullptr);

		vkUnmapMemory(m_device, b.m_ubo_pool[1].m_mem);
		vkDestroyBuffer(m_device, b.m_ubo_pool[1].m_buffer, nullptr);
		vkFreeMemory(m_device, b.m_ubo_pool[1].m_mem, nullptr);
	}

	for (size_t i = 0; i < m_jobs.size(); i++)
	{
		VK_job *job = m_jobs[i];

		delete job;
	}

	m_jobs.clear();

	vkDestroyCommandPool(m_device, m_cmd_pool, nullptr);

	vkDestroySwapchainKHR(m_device, m_swap_chain, nullptr);
	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	for (size_t i = 0; i < m_back_buffers.size(); i++)
	{
		BackBuffer &buf = m_back_buffers[i];
		vkDestroyFence(m_device, buf.present_fence, nullptr);
		vkDestroySemaphore(m_device, buf.render_semaphore, nullptr);
		vkDestroySemaphore(m_device, buf.acquire_semaphore, nullptr);
	}


	vkDestroyDevice(m_device, nullptr);
	if (fpDestroyDebugReportCallbackEXT)
	{
		fpDestroyDebugReportCallbackEXT(m_instance, m_debug_msg_callback, nullptr);
	}
	vkDestroyInstance(m_instance, nullptr);
}


void Enable(std::vector<VkExtensionProperties> &supporteds, std::vector<std::string> &names, std::vector<const char*> &enableds, std::vector<const char*> *not_enableds)
{
	for (size_t j = 0; j < names.size(); j++)
	{
		bool found = false;

		for (size_t i = 0; i < supporteds.size(); i++)
		{
			if (names[j] == supporteds[i].extensionName)
			{
				enableds.push_back(names[j].c_str());
				found = true;
				break;
			}
		}

		if (!found)
		{
			if (not_enableds)
			{
				not_enableds->push_back(names[j].c_str());
			}
		}
	}
}


void Enable(std::vector<VkLayerProperties> &supporteds, std::vector<std::string> &names, std::vector<const char*> &enableds, std::vector<const char*> *not_enableds)
{
	for (size_t j = 0; j < names.size(); j++)
	{
		bool found = false;

		for (size_t i = 0; i < supporteds.size(); i++)
		{
			if (names[j] == supporteds[i].layerName)
			{
				enableds.push_back(names[j].c_str());
				found = true;
				break;
			}
		}

		if (!found)
		{
			if (not_enableds)
			{
				not_enableds->push_back(names[j].c_str());
			}
		}
	}
}


void EnableInstanceExtensions(std::vector<std::string> &extension_names, std::vector<const char*> &enabled_extensions, std::vector<const char*> installed_layer)
{
	VkResult err;
	std::vector<VkExtensionProperties> supported_extensions;

	uint32_t extension_count = 0;

	err = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
	LOGVKERROR(err);

	if (extension_count > 0)
	{
		supported_extensions.resize(extension_count);

		err = vkEnumerateInstanceExtensionProperties(NULL, &extension_count, supported_extensions.data());
		LOGVKERROR(err);
	}
	for (size_t i = 0; i < installed_layer.size(); i++)
	{
		uint32_t num_of_props = 0;
		vkEnumerateInstanceExtensionProperties(installed_layer[i], &num_of_props, nullptr);
		if (num_of_props == 0)
		{
			continue;
		}
		VkExtensionProperties *prop = new VkExtensionProperties[num_of_props];
		vkEnumerateInstanceExtensionProperties(installed_layer[i], &num_of_props, prop);

		bool not_added = true;
		for (size_t j = 0; j < num_of_props; j++)
		{
			for (size_t h = 0; h < supported_extensions.size(); h++)
			{
				if (!strcmp(supported_extensions[h].extensionName, prop[j].extensionName))
				{
					not_added = false;
					break;
				}
			}
			if (not_added)
			{
				VkExtensionProperties new_prop;
				memcpy(new_prop.extensionName, prop[j].extensionName, VK_MAX_EXTENSION_NAME_SIZE);
				new_prop.specVersion = prop[j].specVersion;
				supported_extensions.push_back(new_prop);
			}
		}
		delete[] prop;
	}

	Enable(supported_extensions, extension_names, enabled_extensions, 0);
}


void EnableDeviceExtensions(VkPhysicalDevice &gpu, std::vector<std::string> &extension_names, std::vector<const char*> &enabled_extensions)
{
	VkResult err;
	std::vector<VkExtensionProperties> supported_extensions;

	{
		uint32_t extension_count = 0;

		err = vkEnumerateDeviceExtensionProperties(gpu, NULL, &extension_count, NULL);
		LOGVKERROR(err);


		if (extension_count > 0)
		{
			supported_extensions.resize(extension_count);

			err = vkEnumerateDeviceExtensionProperties(gpu, NULL, &extension_count, supported_extensions.data());
			LOGVKERROR(err);
		}
	}

	Enable(supported_extensions, extension_names, enabled_extensions, 0);
}


void EnableDeviceLayers(VkPhysicalDevice &gpu, std::vector<std::string> &layer_names, std::vector<const char*> &enabled_layers)
{
	VkResult err;
	std::vector<VkLayerProperties> supported_layers;

	{
		uint32_t supported_device_count = 0;

		err = vkEnumerateDeviceLayerProperties(gpu, &supported_device_count, NULL);
		LOGVKERROR(err);

		if (supported_device_count > 0)
		{
			supported_layers.resize(supported_device_count);

			err = vkEnumerateDeviceLayerProperties(gpu, &supported_device_count, supported_layers.data());
			LOGVKERROR(err);
		}
	}

	Enable(supported_layers, layer_names, enabled_layers, 0);
}


void EnableInstanceLayers(std::vector<std::string> &layer_names, std::vector<const char*> &enabled_layers)
{
	VkResult err;
	std::vector<VkLayerProperties> supported_layers;

	uint32_t supported_layer_count = 0;

	err = vkEnumerateInstanceLayerProperties(&supported_layer_count, NULL);
	LOGVKERROR(err);

	if (supported_layer_count > 0)
	{
		supported_layers.resize(supported_layer_count);

		err = vkEnumerateInstanceLayerProperties(&supported_layer_count, supported_layers.data());
		LOGVKERROR(err);
	}

	for (uint32_t i = 0; i < supported_layer_count; i++)
	{
		_logf("Installed Layer: %s", supported_layers[i].layerName);
	}
	Enable(supported_layers, layer_names, enabled_layers, 0);
}


VkBool32 VK_instance::can_present(uint32_t queue_family)
{
#if defined VK_USE_PLATFORM_WIN32_KHR
	return vkGetPhysicalDeviceWin32PresentationSupportKHR(m_gpu, queue_family);
#elif defined VK_USE_PLATFORM_ANDROID_KHR
	return true;
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
	return fpGetPhysicalDeviceWaylandPresentationSupportKHR(m_gpu, queue_family,  (wl_display*)m_context_descriptor.m_user_data[0]);
#elif defined VK_USE_PLATFORM_XCB_KHR
	return vkGetPhysicalDeviceXcbPresentationSupportKHR(m_gpu, queue_family,  (xcb_connection_t*)m_context_descriptor.m_user_data[0],  (xcb_visualid_t)(size_t)m_context_descriptor.m_user_data[2]);
#elif defined VK_USE_PLATFORM_SCREEN_QNX
	return vkGetPhysicalDeviceScreenPresentationSupportQNX(m_gpu, queue_family, (struct _screen_window*)m_context_descriptor.m_user_data[1]);
#elif defined VK_USE_KHR_DISPLAY
	return true;
#endif
	return false;
}


void VK_instance::create_surface()
{
	VkResult err = VK_SUCCESS;

#if defined VK_USE_PLATFORM_WIN32_KHR
	VkWin32SurfaceCreateInfoKHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surface_info.hinstance = (HINSTANCE)m_context_descriptor.m_user_data[0];
	surface_info.hwnd = (HWND)m_context_descriptor.m_user_data[1];

	_logf("vkCreateWin32SurfaceKHR: %p %p\n", surface_info.hinstance, surface_info.hwnd);
	err = vkCreateWin32SurfaceKHR(m_instance, &surface_info, nullptr, &m_surface);

#elif defined VK_USE_PLATFORM_ANDROID_KHR
	VkAndroidSurfaceCreateInfoKHR createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.window = (ANativeWindow*)m_context_descriptor.m_user_data[0];

	_logf("vkCreateAndroidSurfaceKHR: %p\n", createInfo.window);
	err = vkCreateAndroidSurfaceKHR(m_instance, &createInfo, NULL, &m_surface);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
    VkWaylandSurfaceCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    createInfo.pNext = NULL;
    createInfo.flags = 0;
    createInfo.display = (wl_display*)m_context_descriptor.m_user_data[0];
    createInfo.surface = (wl_surface*)m_context_descriptor.m_user_data[1];

	_logf("vkCreateWaylandSurfaceKHR: %p, %x\n", createInfo.display, createInfo.surface);
	err = fpCreateWaylandSurfaceKHR(m_instance, &createInfo, NULL, &m_surface);
#elif defined VK_USE_PLATFORM_XCB_KHR
	VkXcbSurfaceCreateInfoKHR surface_info = {};
	surface_info.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surface_info.connection = (xcb_connection_t*)m_context_descriptor.m_user_data[0];
	surface_info.window = (xcb_window_t)(size_t)m_context_descriptor.m_user_data[1];

	_logf("vkCreateXcbSurfaceKHR: %p, %x\n", surface_info.connection, surface_info.window);
	err = vkCreateXcbSurfaceKHR(m_instance, &surface_info, nullptr, &m_surface);
#elif defined VK_USE_KHR_DISPLAY
	struct Candidate
	{
		VkDisplayKHR display;
		VkDisplayModeKHR mode;
		uint32_t plane;
		uint32_t planeStack;
		uint32_t width;
		uint32_t height;
	};

	std::vector<Candidate> candidates;

	{
		// First, find all displays connected to this platform.
		uint32_t displayPropertyCount;
		err = fpGetPhysicalDeviceDisplayPropertiesKHR(m_gpu, &displayPropertyCount, nullptr);

		if (displayPropertyCount < 1)
		{
			_logf("No displays available.\n");
			return;
		}

		std::vector<VkDisplayPropertiesKHR> displayProperties(displayPropertyCount);
		err = fpGetPhysicalDeviceDisplayPropertiesKHR(m_gpu, &displayPropertyCount, displayProperties.data());

		// Find all supported planes.
		uint32_t planeCount;
		err = fpGetPhysicalDeviceDisplayPlanePropertiesKHR(m_gpu, &planeCount, nullptr);

		if (planeCount < 1)
		{
			_logf("No display planes available.\n");
			return;
		}

		std::vector<VkDisplayPlanePropertiesKHR> planeProperties(planeCount);
		err = fpGetPhysicalDeviceDisplayPlanePropertiesKHR(m_gpu, &planeCount, planeProperties.data());

		// Try to find a good combination of display mode, plane and display for use
		// with our application.
		for (uint32_t plane = 0; plane < planeCount; plane++)
		{
			uint32_t supportedCount;
			err = fpGetDisplayPlaneSupportedDisplaysKHR(m_gpu, plane, &supportedCount, nullptr);

			if (supportedCount < 1)
				continue;

			// For a given plane, find all displays which are supported.
			std::vector<VkDisplayKHR> supportedDisplays(supportedCount);
			err = fpGetDisplayPlaneSupportedDisplaysKHR(m_gpu, plane, &supportedCount, supportedDisplays.data());

			for (size_t i = 0; i < supportedDisplays.size(); i++)
			{
				VkDisplayKHR &display = supportedDisplays[i];
				VkDisplayPropertiesKHR *itr = 0;

				for (size_t j = 0; j < displayProperties.size(); j++)
				{
					if (displayProperties[j].display == display)
					{
						itr = &displayProperties[j];
						break;
					}
				}
				// Display should support identity transform.
				if ((itr->supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == 0)
					continue;

				// If the plane is associated with another display already, skip.
				if (planeProperties[plane].currentDisplay != display &&
					planeProperties[plane].currentDisplay != VK_NULL_HANDLE)
					continue;

				// For the display, find all display modes.
				uint32_t modeCount;
				err = fpGetDisplayModePropertiesKHR(m_gpu, display, &modeCount, nullptr);

				if (modeCount < 1)
					continue;

				std::vector<VkDisplayModePropertiesKHR> modes(modeCount);
				err = fpGetDisplayModePropertiesKHR(m_gpu, display, &modeCount, modes.data());

				for (auto mode : modes)
				{
					// Check that the mode we're trying to use supports what we need.
					VkDisplayPlaneCapabilitiesKHR caps;
					err = fpGetDisplayPlaneCapabilitiesKHR(m_gpu, mode.displayMode, plane, &caps);

					// We don't want alpha since we're not going to composite.
					if ((caps.supportedAlpha & VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR) == 0)
						continue;

					// Check that our preferred swapchain fits into the plane.
					if (caps.minSrcExtent.width > m_context_descriptor.m_display_width)
						continue;
					if (caps.minSrcExtent.height > m_context_descriptor.m_display_height)
						continue;
					if (caps.maxSrcExtent.width < m_context_descriptor.m_display_width)
						continue;
					if (caps.maxSrcExtent.height < m_context_descriptor.m_display_height)
						continue;

					if (mode.parameters.visibleRegion.width >= m_context_descriptor.m_display_width &&
						mode.parameters.visibleRegion.height >= m_context_descriptor.m_display_height)
					{
						// We found a candidate.
						candidates.push_back(
						{ display, mode.displayMode, plane, planeProperties[plane].currentStackIndex,
							mode.parameters.visibleRegion.width, mode.parameters.visibleRegion.height });
					}
				}
			}
		}
	}

	if (candidates.empty())
	{
		_logf("Could not find a suitable display mode.\n");
		return;
	}

	// We could go though the list of candidates here to find the optimal match,
	// but we can pick the first one here.
	VkDisplaySurfaceCreateInfoKHR info = { VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR };
	info.displayMode = candidates.front().mode;
	info.planeIndex = candidates.front().plane;
	info.planeStackIndex = candidates.front().planeStack;
	info.transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	info.alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR;
	info.imageExtent.width = candidates.front().width;
	info.imageExtent.height = candidates.front().height;

	_logf("Using display mode: %u x %u.\n", info.imageExtent.width, info.imageExtent.height);

	err = fpCreateDisplayPlaneSurfaceKHR(m_instance, &info, nullptr, &m_surface);
#elif defined VK_USE_PLATFORM_SCREEN_QNX
	VkScreenSurfaceCreateInfoQNX createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SCREEN_SURFACE_CREATE_INFO_QNX;
	createInfo.pNext = NULL;
	createInfo.flags = 0;
	createInfo.context = (struct _screen_context*)m_context_descriptor.m_user_data[0];
	createInfo.window = (struct _screen_window*)m_context_descriptor.m_user_data[1];
	_logf("vkCreateScreenSurfaceQNX: %p, %p\n", createInfo.context, createInfo.window);
	err = fpCreateScreenSurfaceQNX(m_instance, &createInfo, NULL, &m_surface);
#endif
	LOGVKERROR(err);
	VkBool32 supportsPresent = true;

	_logf("vkGetPhysicalDeviceSurfaceSupportKHR\n");
	vkGetPhysicalDeviceSurfaceSupportKHR(m_gpu, m_queue_family, m_surface, &supportsPresent);

	_logf("supportsPresent: %u\n", supportsPresent);

	if (!supportsPresent)
	{
		//assert(0);
	}
}


void VK_instance::Resize(uint32_t width_hint, uint32_t height_hint)
{
	VkResult err;
	VkSurfaceCapabilitiesKHR caps;

	_logf("vkGetPhysicalDeviceSurfaceCapabilitiesKHR\n");
	err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &caps);
	LOGVKERROR(err);
	if (caps.minImageCount > caps.maxImageCount)
	{
		caps.maxImageCount = caps.minImageCount;
	}
	_logf("VkSurfaceCapabilitiesKHR\n");
	_logf("minImageCount: %u\n", caps.minImageCount);
	_logf("maxImageCount: %u\n", caps.maxImageCount);
	_logf("currentExtent: %u, %u\n", caps.currentExtent.width, caps.currentExtent.height);
	_logf("minImageExtent: %u, %u\n", caps.minImageExtent.width, caps.minImageExtent.height);
	_logf("maxImageExtent: %u, %u\n", caps.maxImageExtent.width, caps.maxImageExtent.height);
	_logf("supportedTransforms: %x\n", caps.supportedTransforms);
	_logf("supportedCompositeAlpha: %x\n", caps.supportedCompositeAlpha);
	_logf("supportedUsageFlags: %x\n", caps.supportedUsageFlags);

	VkExtent2D extent = caps.currentExtent;
	// use the hints
	if (extent.width == (uint32_t)-1)
	{
		extent.width = width_hint;
		extent.height = height_hint;
	}
	// clamp width; to protect us from broken hints?
	if (extent.width < caps.minImageExtent.width)
		extent.width = caps.minImageExtent.width;
	else if (extent.width > caps.maxImageExtent.width)
		extent.width = caps.maxImageExtent.width;
	// clamp height
	if (extent.height < caps.minImageExtent.height)
		extent.height = caps.minImageExtent.height;
	else if (extent.height > caps.maxImageExtent.height)
		extent.height = caps.maxImageExtent.height;

	//if (ctx_.extent.width == extent.width && ctx_.extent.height == extent.height)
	//	return;

#if !defined(VK_USE_PLATFORM_SCREEN_QNX)
	uint32_t image_count = NUM_FRAMES;
#else /* VK_USE_PLATFORM_SCREEN_QNX */
	uint32_t image_count = (int)(uintptr_t)m_context_descriptor.m_user_data[3];
#endif /* VK_USE_PLATFORM_SCREEN_QNX */

	if (image_count < caps.minImageCount)
	{
		image_count = caps.minImageCount;
	}
	else if (caps.maxImageCount > 0 && image_count > caps.maxImageCount)
	{
		image_count = caps.maxImageCount;
	}

	_logf("Chosen Image Count: %u\n", image_count); // the value of m_swapchain_length is set later by querying the number of images in the resulting swap chain

	if (image_count <= 1)
	{
		_logf("Error: Image Count <= 1\n");
	}

	//assert(caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
	//assert(caps.supportedTransforms & caps.currentTransform);
	//assert(caps.supportedCompositeAlpha & (VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR | VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR));

	VkCompositeAlphaFlagBitsKHR composite_alpha = (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) ? VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR : VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	_logf("vkGetPhysicalDeviceSurfacePresentModesKHR\n");
	std::vector<VkPresentModeKHR> presentModes;
	{
		uint32_t presentModeCount;

		err = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, NULL);
		LOGVKERROR(err);

		presentModes.resize(presentModeCount);
		_logf("presentModes: %u: ", presentModeCount);
		err = vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, presentModes.data());
		LOGVKERROR(err);
		for (uint32_t i = 0; i < presentModeCount; i++)
		{
			_logf("#%u - %u,", i, presentModes[i]);
		}
		_logf("\n");
	}
	// FIFO is the only mode universally supported
	VkPresentModeKHR mode = VK_PRESENT_MODE_FIFO_KHR;
	for (size_t i = 0; i < presentModes.size(); i++)
	{
		VkPresentModeKHR &m = presentModes[i];
		if ((m_vsync && m == VK_PRESENT_MODE_MAILBOX_KHR) ||
			(!m_vsync && m == VK_PRESENT_MODE_IMMEDIATE_KHR)) {
			mode = m;
			break;
		}
	}
	VkSurfaceTransformFlagBitsKHR preTransform;

	if (caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		preTransform = caps.currentTransform;
	}

	VkImageUsageFlags transfer_src_bit = VK_instance::This->m_context_descriptor.m_system_attachment_is_transfer_source ? VK_IMAGE_USAGE_TRANSFER_SRC_BIT : 0;

	VkSwapchainCreateInfoKHR swapchain_info = {};
	C(swapchain_info, VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
	swapchain_info.surface = m_surface;
	swapchain_info.minImageCount = image_count;
	swapchain_info.imageFormat = m_surface_color_format;
	swapchain_info.imageColorSpace = m_surface_color_space;
	swapchain_info.imageExtent = extent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | transfer_src_bit;
	swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchain_info.preTransform = preTransform;
	swapchain_info.compositeAlpha = composite_alpha;
	swapchain_info.presentMode = mode;
	swapchain_info.clipped = false;
	swapchain_info.oldSwapchain = m_swap_chain;
	swapchain_info.queueFamilyIndexCount = 1;
	swapchain_info.pQueueFamilyIndices = &m_queue_family;

	_logf("vkCreateSwapchainKHR\n");
	_logf("extent: %u, %u\n", extent.width, extent.height);
	_logf("presentMode:%x, preTransform: %x, compositeAlpha:%x\n", mode, preTransform, composite_alpha);
	err = vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swap_chain);
	LOGVKERROR(err);
	//ctx_.extent = extent;

	// destroy the old swapchain
	if (swapchain_info.oldSwapchain != VK_NULL_HANDLE)
	{
		err = vkDeviceWaitIdle(m_device);
		LOGVKERROR(err);
		vkDestroySwapchainKHR(m_device, swapchain_info.oldSwapchain, nullptr);
	}

	//void Hologram::prepare_framebuffers(VkSwapchainKHR swapchain)
	{
		// get swapchain images
		{
			uint32_t swapchainImageCount;

			err = vkGetSwapchainImagesKHR(m_device, m_swap_chain, &swapchainImageCount, NULL);
			LOGVKERROR(err);

			m_swapchain_images.resize(swapchainImageCount);
			m_swapchain_image_views.resize(swapchainImageCount);

			err = vkGetSwapchainImagesKHR(m_device, m_swap_chain, &swapchainImageCount, m_swapchain_images.data());
			LOGVKERROR(err);

			m_swapchain_length = swapchainImageCount;
			m_back_buffers.resize(m_swapchain_length);

			_logf("Effective Swap Chain Length: %u\n", m_swapchain_length);
		}

		VkCommandBuffer setup_cmd = BeginTemporaryCmdBuffer();

		for (uint32_t i = 0; i < m_swapchain_images.size(); i++)
		{
			VkImageViewCreateInfo color_image_view;
			C(color_image_view, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
			color_image_view.pNext = NULL;
			color_image_view.format = m_surface_color_format;
			color_image_view.components.r = VK_COMPONENT_SWIZZLE_R;
			color_image_view.components.g = VK_COMPONENT_SWIZZLE_G;
			color_image_view.components.b = VK_COMPONENT_SWIZZLE_B;
			color_image_view.components.a = VK_COMPONENT_SWIZZLE_A;
			color_image_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			color_image_view.subresourceRange.baseMipLevel = 0;
			color_image_view.subresourceRange.levelCount = 1;
			color_image_view.subresourceRange.baseArrayLayer = 0;
			color_image_view.subresourceRange.layerCount = 1;
			color_image_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
			color_image_view.flags = 0;
			color_image_view.image = m_swapchain_images[i];

			set_image_layout(
				setup_cmd, m_swapchain_images[i],
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1);

			err = vkCreateImageView(m_device, &color_image_view, 0, &m_swapchain_image_views[i]);
			LOGVKERROR(err);
		}

		EndTemporaryCmdBuffer(setup_cmd);
	}
}


void VK_instance::Init2(NGL_api api, uint32_t major_version, uint32_t minor_version, bool enable_validation)
{
	VkResult err;
#ifdef WIN32
	if (m_context_descriptor.m_user_data.size() != 2)
	{
		_logf("Please provide user data for NGL-Vulkan: HINSTANCE, HWND");
		return;
	}
#endif
	m_textures[0].m_texture_descriptor.m_size[0] = m_context_descriptor.m_display_width;
	m_textures[0].m_texture_descriptor.m_size[1] = m_context_descriptor.m_display_height;

	std::vector<std::string> desired_instance_layer_names;
	std::vector<std::string> desired_instance_extension_names;
	std::vector<std::string> device_layer_names;
	std::vector<std::string> device_extension_names;

	std::vector<const char*> enabled_device_layers;
	std::vector<const char*> enabled_device_extensions;

	std::vector<const char*> enabled_instance_layers;
	std::vector<const char*> enabled_instance_extensions;

	desired_instance_extension_names.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

	{
		desired_instance_extension_names.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
#if defined VK_USE_PLATFORM_WIN32_KHR
		desired_instance_extension_names.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_ANDROID_KHR
		desired_instance_extension_names.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_WAYLAND_KHR
		desired_instance_extension_names.push_back(VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME);
		_logf("wayland surface");
#elif defined VK_USE_PLATFORM_XCB_KHR
		desired_instance_extension_names.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
		_logf("xcb surface");
#elif defined VK_USE_KHR_DISPLAY
		desired_instance_extension_names.push_back(VK_KHR_DISPLAY_EXTENSION_NAME);
#elif defined VK_USE_PLATFORM_SCREEN_QNX
		desired_instance_extension_names.push_back(VK_QNX_SCREEN_SURFACE_EXTENSION_NAME);
		_logf("qnx screen surface");
#endif
		device_extension_names.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
	if (enable_validation)
	{
		//KEEP THE ORDER of plugins!
		// Instance layers to enable
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_core_validation");
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_image");
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_monitor");
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_object_tracker");
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_parameter_validation");
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_swapchain");
		desired_instance_layer_names.push_back("VK_LAYER_GOOGLE_threading");
		desired_instance_layer_names.push_back("VK_LAYER_GOOGLE_unique_objects");
		desired_instance_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");

		// Device layers to enable
		device_layer_names.push_back("VK_LAYER_LUNARG_core_validation");
		device_layer_names.push_back("VK_LAYER_LUNARG_image");
		device_layer_names.push_back("VK_LAYER_LUNARG_monitor");
		device_layer_names.push_back("VK_LAYER_LUNARG_object_tracker");
		device_layer_names.push_back("VK_LAYER_LUNARG_parameter_validation");
		device_layer_names.push_back("VK_LAYER_LUNARG_swapchain");
		device_layer_names.push_back("VK_LAYER_GOOGLE_threading");
		device_layer_names.push_back("VK_LAYER_GOOGLE_unique_objects");
		device_layer_names.push_back("VK_LAYER_LUNARG_standard_validation");
	}

	EnableInstanceLayers(desired_instance_layer_names, enabled_instance_layers);
	EnableInstanceExtensions(desired_instance_extension_names, enabled_instance_extensions, enabled_instance_layers);

	_logf("vkCreateInstance\n");
	{
		VkApplicationInfo app_info = {};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Kishonti's NGL";

		app_info.applicationVersion = VK_API_VERSION;
		app_info.engineVersion = VK_API_VERSION;
		app_info.apiVersion = VK_API_VERSION;

		VkInstanceCreateInfo instance_info = {};
		instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_info.pApplicationInfo = &app_info;

		instance_info.enabledLayerCount = (uint32_t)enabled_instance_layers.size();
		instance_info.ppEnabledLayerNames = enabled_instance_layers.data();

		instance_info.enabledExtensionCount = (uint32_t)enabled_instance_extensions.size();
		instance_info.ppEnabledExtensionNames = enabled_instance_extensions.data();

		err = vkCreateInstance(&instance_info, nullptr, &m_instance);
		LOGVKERROR(err);
	}
	_logf("validation_layers_is_enabled %d\n", enable_validation);
	if (enable_validation)
	{
		GET_INSTANCE_PROC_ADDR(m_instance, CreateDebugReportCallbackEXT);
		GET_INSTANCE_PROC_ADDR(m_instance, DestroyDebugReportCallbackEXT);

		_logf("CreateDebugReportCallbackEXT: %p\n", fpCreateDebugReportCallbackEXT);
		_logf("DestroyDebugReportCallbackEXT: %p\n", fpDestroyDebugReportCallbackEXT);

		if (fpCreateDebugReportCallbackEXT && fpDestroyDebugReportCallbackEXT)
		{
			VkDebugReportCallbackCreateInfoEXT info;

			info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
			info.pNext = 0;
			info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT;
			info.pfnCallback = ValidationCallback;
			info.pUserData = 0;

			err = fpCreateDebugReportCallbackEXT(m_instance, &info, 0, &m_debug_msg_callback);
			LOGVKERROR(err);
		}
		else
		{
			_logf("%s is not available", VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
	}

#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	GET_INSTANCE_PROC_ADDR(m_instance, CreateWaylandSurfaceKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceWaylandPresentationSupportKHR);

	if (fpCreateWaylandSurfaceKHR == nullptr)
	{
		void* libvulkan = dlopen("libvulkan.so", RTLD_LAZY);
		if (libvulkan == nullptr)
		{
			_logf("libvulkan.so not found\n");
		}
		else
		{
			fpCreateWaylandSurfaceKHR = reinterpret_cast<PFN_vkCreateWaylandSurfaceKHR>(dlsym(libvulkan, "vkCreateWaylandSurfaceKHR"));
			fpGetPhysicalDeviceWaylandPresentationSupportKHR = reinterpret_cast<PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR>(dlsym(libvulkan, "vkGetPhysicalDeviceWaylandPresentationSupportKHR"));
		}
	}

	_logf("CreateWaylandSurfaceKHR: %p\n", fpCreateWaylandSurfaceKHR);
	_logf("GetPhysicalDeviceWaylandPresentationSupportKHR: %p\n", fpGetPhysicalDeviceWaylandPresentationSupportKHR);
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
	GET_INSTANCE_PROC_ADDR(m_instance, CreateScreenSurfaceQNX);
	GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceScreenPresentationSupportQNX);
	
	if (fpCreateScreenSurfaceQNX == nullptr)
	{
		void* libvulkan = dlopen("libvulkan.so", RTLD_LAZY);
		if (libvulkan == nullptr)
		{
			_logf("libvulkan.so not found\n");
		}
		else
		{
			fpCreateScreenSurfaceQNX = reinterpret_cast<PFN_vkCreateScreenSurfaceQNX>(dlsym(libvulkan, "vkCreateScreenSurfaceQNX"));
			fpGetPhysicalDeviceScreenPresentationSupportQNX = reinterpret_cast<PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX>(dlsym(libvulkan, "vkGetPhysicalDeviceScreenPresentationSupportQNX"));
		}
	}

	_logf("CreateScreenSurfaceQNX: %p\n", fpCreateScreenSurfaceQNX);
	_logf("GetPhysicalDeviceScreenPresentationSupportQNX: %p\n", fpGetPhysicalDeviceScreenPresentationSupportQNX);
#endif

#ifdef VK_USE_KHR_DISPLAY
	GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceDisplayPropertiesKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, GetPhysicalDeviceDisplayPlanePropertiesKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, GetDisplayPlaneSupportedDisplaysKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, GetDisplayModePropertiesKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, CreateDisplayModeKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, GetDisplayPlaneCapabilitiesKHR);
	GET_INSTANCE_PROC_ADDR(m_instance, CreateDisplayPlaneSurfaceKHR);
#endif

	_logf("vkEnumeratePhysicalDevices\n");
	//init_physical_dev();
	{
		// enumerate physical devices

		uint32_t num_gpus;
		std::vector<VkPhysicalDevice> gpus;

		err = vkEnumeratePhysicalDevices(m_instance, &num_gpus, NULL);
		LOGVKERROR(err);

		gpus.resize(num_gpus);

		err = vkEnumeratePhysicalDevices(m_instance, &num_gpus, gpus.data());
		LOGVKERROR(err);

		_logf("Supported physical devices: %d", num_gpus);
		if (num_gpus == 0)
		{
			_logf("vkCreateInstance failed. Supported physical device count is zero!\n");
		}

		std::string selected_device_name;
		uint64_t selected_luid = 0;
		bool has_selected_luid = false;
		{
			const std::string& selected_device = m_context_descriptor.m_selected_device_id;
			size_t delimiter_pos = selected_device.find_last_of(";");
			if (delimiter_pos == std::string::npos || selected_device.size() == 0 || delimiter_pos >= selected_device.size() - 1)
			{
				has_selected_luid = false;
				selected_device_name = selected_device;
			}
			else
			{
				std::stringstream luid_stream;
				luid_stream.str(selected_device.substr(delimiter_pos + 1));
				luid_stream >> selected_luid;
				has_selected_luid = true;
				selected_device_name = selected_device.substr(0, delimiter_pos);
			}
		}

		size_t gpu_index = 0;
		for (gpu_index = 0; gpu_index < gpus.size(); gpu_index++)
		{
			vkGetPhysicalDeviceProperties(gpus[gpu_index], &m_PhysicalDeviceProperties);

			_logf("#%d:\n", gpu_index);
			_logf("\tVendor ID: 0x%x - Device ID: 0x%x, Device Name: %s.\n",
				m_PhysicalDeviceProperties.vendorID,
				m_PhysicalDeviceProperties.deviceID,
				m_PhysicalDeviceProperties.deviceName
				);

			if (selected_device_name.length() == 0 || selected_device_name == m_PhysicalDeviceProperties.deviceName)
			{
				if (has_selected_luid && gpu_index != selected_luid)
				{
					continue;
				}

				m_gpu = gpus[gpu_index];
				break;
			}
		}

		if (gpu_index >= gpus.size())
		{
			throw std::exception();
		}

		EnableDeviceLayers(m_gpu, device_layer_names, enabled_device_layers);
		EnableDeviceExtensions(m_gpu, device_extension_names, enabled_device_extensions);

		vkGetPhysicalDeviceMemoryProperties(m_gpu, &m_PhysicalDeviceMemoryProperties);
		vkGetPhysicalDeviceFeatures(m_gpu, &m_PhysicalDeviceFeatures);
		for (uint32_t f = 0; f < VK_FORMAT_RANGE_SIZE; f++)
		{
			vkGetPhysicalDeviceFormatProperties(m_gpu, (VkFormat)f, &m_format_props[f]);
		}

		{
			std::vector<VkQueueFamilyProperties> QueueFamilyProperties;

			{
				uint32_t queue_property_count = 0;

				vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_property_count, 0);

				QueueFamilyProperties.resize(queue_property_count);

				vkGetPhysicalDeviceQueueFamilyProperties(m_gpu, &queue_property_count, &QueueFamilyProperties[0]);
			}

			int game_queue_family = -1;
			int present_queue_family = -1;
			for (uint32_t i = 0; i < QueueFamilyProperties.size(); i++)
			{
				const VkQueueFamilyProperties &q = QueueFamilyProperties[i];

				// requires only GRAPHICS for game queues
				const VkFlags game_queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT;
				if (game_queue_family < 0 &&
					(q.queueFlags & game_queue_flags) == game_queue_flags)
					game_queue_family = i;

				// present queue must support the surface
				if (present_queue_family < 0 && can_present(i))
					present_queue_family = i;

				if (game_queue_family >= 0 && present_queue_family >= 0)
					break;
			}

			if (game_queue_family == present_queue_family)
			{
				if (game_queue_family > -1)
				{
					m_queue_family = game_queue_family;
				}
				else
				{
					assert(0);
				}
			}
		}
	}
	_logf("vkCreateDevice\n");
	//create_dev()
	{
		VkDeviceCreateInfo dev_info = {};
		dev_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

		float queue_priorities[1] = { 0.0f };
		VkDeviceQueueCreateInfo queue_info[1];

		queue_info[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_info[0].queueFamilyIndex = m_queue_family;
		queue_info[0].queueCount = 1;
		queue_info[0].pQueuePriorities = queue_priorities;
		queue_info[0].pNext = nullptr;
		queue_info[0].flags = 0;

		dev_info.queueCreateInfoCount = 1;
		dev_info.pQueueCreateInfos = queue_info;
		dev_info.enabledLayerCount = (uint32_t)enabled_device_layers.size();
		dev_info.ppEnabledLayerNames = enabled_device_layers.data();
		dev_info.enabledExtensionCount = (uint32_t)enabled_device_extensions.size();
		dev_info.ppEnabledExtensionNames = enabled_device_extensions.data();
		dev_info.pNext = nullptr;

		// enable all features
		VkPhysicalDeviceFeatures features;
		vkGetPhysicalDeviceFeatures(m_gpu, &features);
		dev_info.pEnabledFeatures = &features;

		_logf("%s %d", "robustBufferAccess", features.robustBufferAccess);
		_logf("%s %d", "fullDrawIndexUint32", features.fullDrawIndexUint32);
		_logf("%s %d", "imageCubeArray", features.imageCubeArray);
		_logf("%s %d", "independentBlend", features.independentBlend);
		_logf("%s %d", "geometryShader", features.geometryShader);
		_logf("%s %d", "tessellationShader", features.tessellationShader);
		_logf("%s %d", "sampleRateShading", features.sampleRateShading);
		_logf("%s %d", "dualSrcBlend", features.dualSrcBlend);
		_logf("%s %d", "logicOp", features.logicOp);
		_logf("%s %d", "multiDrawIndirect", features.multiDrawIndirect);
		_logf("%s %d", "drawIndirectFirstInstance", features.drawIndirectFirstInstance);
		_logf("%s %d", "depthClamp", features.depthClamp);
		_logf("%s %d", "depthBiasClamp", features.depthBiasClamp);
		_logf("%s %d", "fillModeNonSolid", features.fillModeNonSolid);
		_logf("%s %d", "depthBounds", features.depthBounds);
		_logf("%s %d", "wideLines", features.wideLines);
		_logf("%s %d", "largePoints", features.largePoints);
		_logf("%s %d", "multiViewport", features.multiViewport);
		_logf("%s %d", "samplerAnisotropy", features.samplerAnisotropy);
		_logf("%s %d", "textureCompressionETC2", features.textureCompressionETC2);
		_logf("%s %d", "textureCompressionASTC_LDR", features.textureCompressionASTC_LDR);
		_logf("%s %d", "textureCompressionBC", features.textureCompressionBC);
		_logf("%s %d", "occlusionQueryPrecise", features.occlusionQueryPrecise);
		_logf("%s %d", "pipelineStatisticsQuery", features.pipelineStatisticsQuery);
		_logf("%s %d", "vertexPipelineStoresAndAtomics", features.vertexPipelineStoresAndAtomics);
		_logf("%s %d", "fragmentStoresAndAtomics", features.fragmentStoresAndAtomics);
		_logf("%s %d", "shaderTessellationAndGeometryPointSize", features.shaderTessellationAndGeometryPointSize);
		_logf("%s %d", "shaderImageGatherExtended", features.shaderImageGatherExtended);
		_logf("%s %d", "shaderStorageImageExtendedFormats", features.shaderStorageImageExtendedFormats);
		_logf("%s %d", "shaderStorageImageMultisample", features.shaderStorageImageMultisample);
		_logf("%s %d", "shaderStorageImageReadWithoutFormat", features.shaderStorageImageReadWithoutFormat);
		_logf("%s %d", "shaderStorageImageWriteWithoutFormat", features.shaderStorageImageWriteWithoutFormat);
		_logf("%s %d", "shaderUniformBufferArrayDynamicIndexing", features.shaderUniformBufferArrayDynamicIndexing);
		_logf("%s %d", "shaderSampledImageArrayDynamicIndexing", features.shaderSampledImageArrayDynamicIndexing);
		_logf("%s %d", "shaderStorageBufferArrayDynamicIndexing", features.shaderStorageBufferArrayDynamicIndexing);
		_logf("%s %d", "shaderStorageImageArrayDynamicIndexing", features.shaderStorageImageArrayDynamicIndexing);
		_logf("%s %d", "shaderClipDistance", features.shaderClipDistance);
		_logf("%s %d", "shaderCullDistance", features.shaderCullDistance);
		_logf("%s %d", "shaderFloat64", features.shaderFloat64);
		_logf("%s %d", "shaderInt64", features.shaderInt64);
		_logf("%s %d", "shaderInt16", features.shaderInt16);
		_logf("%s %d", "shaderResourceResidency", features.shaderResourceResidency);
		_logf("%s %d", "shaderResourceMinLod", features.shaderResourceMinLod);
		_logf("%s %d", "sparseBinding", features.sparseBinding);
		_logf("%s %d", "sparseResidencyBuffer", features.sparseResidencyBuffer);
		_logf("%s %d", "sparseResidencyImage2D", features.sparseResidencyImage2D);
		_logf("%s %d", "sparseResidencyImage3D", features.sparseResidencyImage3D);
		_logf("%s %d", "sparseResidency2Samples", features.sparseResidency2Samples);
		_logf("%s %d", "sparseResidency4Samples", features.sparseResidency4Samples);
		_logf("%s %d", "sparseResidency8Samples", features.sparseResidency8Samples);
		_logf("%s %d", "sparseResidency16Samples", features.sparseResidency16Samples);
		_logf("%s %d", "sparseResidencyAliased", features.sparseResidencyAliased);
		_logf("%s %d", "variableMultisampleRate", features.variableMultisampleRate);
		_logf("%s %d", "inheritedQueries", features.inheritedQueries);

		features.robustBufferAccess = 0;

		err = vkCreateDevice(m_gpu, &dev_info, 0, &m_device);
		LOGVKERROR(err);
	}

	vkGetDeviceQueue(m_device, m_queue_family, 0, &m_queue);

	create_surface();

	_logf("vkGetPhysicalDeviceSurfaceFormatsKHR\n");
	{
		VkResult err;
		std::vector<VkSurfaceFormatKHR> surfFormats;
		uint32_t formatCount;

		err = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, NULL);

		LOGVKERROR(err);
		surfFormats.resize(formatCount);

		err = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &formatCount, surfFormats.data());
		LOGVKERROR(err);

		_logf("surfFormats: %u\n", formatCount);

		for (uint32_t i = 0; i < formatCount; i++)
		{
			_logf("#%u - color_format: %d, color_space: %d\n", i, surfFormats[i].format, surfFormats[i].colorSpace);
		}

		m_surface_color_format = VK_FORMAT_R8G8B8A8_UNORM;
		m_surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;

		if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED)
		{
			m_surface_color_format = VK_FORMAT_B8G8R8A8_UNORM;
			m_surface_color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		}
		else
		{
			size_t selected_format = 0;

#if !defined(VK_USE_PLATFORM_SCREEN_QNX)
			VkFormat supported_formats[] = {
				VK_FORMAT_R8G8B8A8_UNORM,
				VK_FORMAT_B8G8R8A8_UNORM,
				VK_FORMAT_R8G8B8_UNORM,
				VK_FORMAT_B8G8R8_UNORM,
#if defined(VK_USE_PLATFORM_SCREEN_QNX)
				VK_FORMAT_A2R10G10B10_UNORM_PACK32,
				VK_FORMAT_A2B10G10R10_UNORM_PACK32,
				VK_FORMAT_R5G6B5_UNORM_PACK16,
#endif /* VK_USE_PLATFORM_SCREEN_QNX */
			};
#else /* !VK_USE_PLATFORM_SCREEN_QNX */
            VkFormat supported_formats[1] = { VK_FORMAT_UNDEFINED };

            std::string fmt((const char*)m_context_descriptor.m_user_data[2]);

            do
			{
                if (fmt.compare("rgb565") == 0)
				{
                    supported_formats[0] = VK_FORMAT_R5G6B5_UNORM_PACK16;
                    break;
                }
                if (fmt.compare("rgba8888") == 0)
				{
                    supported_formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
                    break;
                }
                if (fmt.compare("bgra8888") == 0)
				{
                    supported_formats[0] = VK_FORMAT_R8G8B8A8_UNORM;
                    break;
                }
                if (fmt.compare("rgba1010102") == 0)
				{
                    supported_formats[0] = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
                    break;
                }
                if (fmt.compare("bgra1010102") == 0)
				{
                    supported_formats[0] = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
                    break;
                }

            } while (0);
#endif /* !VK_USE_PLATFORM_SCREEN_QNX */
			const int num_supported_formats = sizeof(supported_formats) / sizeof(supported_formats[0]);
			bool format_found = false;
			for (int j = 0; j < num_supported_formats; j++)
			{
				for (size_t i = 0; i < formatCount; i++)
				{
					if (surfFormats[i].format == supported_formats[j])
					{
						selected_format = i;
						format_found = true;
						break;
					}
				}

				if (format_found)
				{
					break;
				}
			}

			if (format_found)
			{
				m_surface_color_format = surfFormats[selected_format].format;
				m_surface_color_space = surfFormats[selected_format].colorSpace;

				_logf("selected format: #%u\n", selected_format);
			}
			else
			{
				_logf("Error: none of the available surface formats are supported\n");

				_logf("Supported formats:\n");
				for (int i = 0; i < num_supported_formats; i++)
				{
					_logf("%d\n", i, supported_formats[i]);
				}
			}
		}
	}
	{
		const VkCommandPoolCreateInfo cmd_pool_info = {
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			NULL,
			VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
			m_queue_family
		};
		err = vkCreateCommandPool(m_device, &cmd_pool_info, NULL, &m_cmd_pool);
		LOGVKERROR(err);
	}

	Resize(m_textures[0].m_texture_descriptor.m_size[0], m_textures[0].m_texture_descriptor.m_size[1]);

	{
		VkSemaphoreCreateInfo sem_info = {};
		sem_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		// BackBuffer is used to track which swapchain image and its associated
		// sync primitives are busy.  Having more BackBuffer's than swapchain
		// images may allows us to replace CPU wait on present_fence by GPU wait
		// on acquire_semaphore.

		const int count = (int)m_back_buffers.size();

		if (count <= 0)
		{
			_logf("Error: no backbuffers were created!");
		}

		for (int i = 0; i < count; i++)
		{
			BackBuffer &buf = m_back_buffers[i];
			err = vkCreateSemaphore(m_device, &sem_info, 0, &buf.acquire_semaphore);
			LOGVKERROR(err);
			err = vkCreateSemaphore(m_device, &sem_info, 0, &buf.render_semaphore);
			LOGVKERROR(err);
			err = vkCreateFence(m_device, &fence_info, 0, &buf.present_fence);
			LOGVKERROR(err);
		}
	}

	m_vertex_memory.resize(1);
	m_vertex_index_memory.resize(1);
	CreateMemory(m_vertex_memory.back(), 1024 * 1024 * 22, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	CreateMemory(m_vertex_index_memory.back(), 1024 * 1024 * 3, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

	m_cmd_buffers.resize(NUM_MAX_CMD_BUFFERS);

	{
		VkResult err;
		VkCommandBufferAllocateInfo i;
		std::vector<VkCommandBuffer> b(m_cmd_buffers.size());

		i.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		i.pNext = 0;
		i.commandPool = m_cmd_pool;
		i.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		i.commandBufferCount = (uint32_t)m_cmd_buffers.size();

		err = vkAllocateCommandBuffers(DEVICE, &i, b.data());
		LOGVKERROR(err);

		for (size_t i = 0; i < m_cmd_buffers.size(); i++)
		{
			VK_cmd_buffer &cmd_buffer = m_cmd_buffers[i];

			cmd_buffer.cb = b[i];
			{
				VkFenceCreateInfo fence_info = {};
				fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

				err = vkCreateFence(DEVICE, &fence_info, 0, &cmd_buffer.f);
				LOGVKERROR(err);
			}
			{
				VK_ubo &ubo = cmd_buffer.m_ubo_pool[0];
				ubo.m_datasize = UBO_POOL0_SIZE;
				CreateUBO(ubo);
			}
			{
				VK_ubo &ubo = cmd_buffer.m_ubo_pool[1];
				ubo.m_datasize = UBO_POOL1_SIZE;
				CreateUBO(ubo);
			}
			{
				C(cmd_buffer.m_ubo_ranges[0], VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);
				C(cmd_buffer.m_ubo_ranges[1], VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE);

				cmd_buffer.m_ubo_ranges[0].memory = cmd_buffer.m_ubo_pool[0].m_mem;
				cmd_buffer.m_ubo_ranges[0].size = cmd_buffer.m_ubo_pool[0].m_datasize;
				cmd_buffer.m_ubo_ranges[1].memory = cmd_buffer.m_ubo_pool[1].m_mem;
				cmd_buffer.m_ubo_ranges[1].size = cmd_buffer.m_ubo_pool[1].m_datasize;
			}
		}
	}

	cached_used_DescriptorSets.reserve(1000);
	cached_used_WriteDescriptorSets.reserve(1000);
	cached_vk_buffer_barriers.reserve(1000);
	cached_vk_image_barriers.reserve(1000);
}


void VK_instance::PostInit(NGL_api api, uint32_t major_version, uint32_t minor_version, bool enable_validation)
{
	const char *vendor = "";
	const char *renderer = "";
	const char *version = "";

	m_propertiess[NGL_VENDOR] = vendor ? vendor : "null";
	m_propertiess[NGL_RENDERER] = renderer ? renderer : "null";
	m_propertiess[NGL_VERSION] = version ? version : "null";
	for (size_t i = 0; i < NGL_NUM_PROPERTIES; i++)
	{
		m_propertiesi[i] = 0;
	}
	m_propertiesi[NGL_API] = api;
	m_propertiesi[NGL_MAJOR_VERSION] = major_version;
	m_propertiesi[NGL_MINOR_VERSION] = minor_version;
	m_propertiesi[NGL_RASTERIZATION_CONTROL_MODE] = NGL_ORIGIN_UPPER_LEFT;
	m_propertiesi[NGL_DEPTH_MODE] = NGL_ZERO_TO_ONE;
	m_propertiesi[NGL_NEED_SWAPBUFFERS] = 0;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT5] = 0;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_ETC2] = 0;

	m_propertiesi[NGL_TEXTURE_MAX_ANISOTROPY] = (int32_t)m_PhysicalDeviceProperties.limits.maxSamplerAnisotropy;
	m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D] = (int32_t)m_PhysicalDeviceProperties.limits.maxImageDimension2D;
	m_propertiesi[NGL_TEXTURE_MAX_SIZE_CUBE] = (int32_t)m_PhysicalDeviceProperties.limits.maxImageDimensionCube;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_ETC2] = (int32_t)m_PhysicalDeviceFeatures.textureCompressionETC2;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT1] = (int32_t)m_PhysicalDeviceFeatures.textureCompressionBC;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT5] = (int32_t)m_PhysicalDeviceFeatures.textureCompressionBC;
	m_propertiesi[NGL_TEXTURE_COMPRESSION_ASTC] = (int32_t)m_PhysicalDeviceFeatures.textureCompressionASTC_LDR;
	m_propertiesi[NGL_TESSELLATION] = (int32_t)m_PhysicalDeviceFeatures.tessellationShader;
	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_X] = (int32_t)m_PhysicalDeviceProperties.limits.maxComputeWorkGroupSize[0];
	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Y] = (int32_t)m_PhysicalDeviceProperties.limits.maxComputeWorkGroupSize[1];
	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_SIZE_Z] = (int32_t)m_PhysicalDeviceProperties.limits.maxComputeWorkGroupSize[2];
	m_propertiesi[NGL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS] = (int32_t)m_PhysicalDeviceProperties.limits.maxComputeWorkGroupInvocations;
	m_propertiesi[NGL_MAX_COMPUTE_SHARED_MEMORY_SIZE] = (int32_t)m_PhysicalDeviceProperties.limits.maxComputeSharedMemorySize;

	m_D24_format = VK_instance::This->m_format_props[VK_FORMAT_X8_D24_UNORM_PACK32].optimalTilingFeatures ? VK_FORMAT_X8_D24_UNORM_PACK32 : VK_FORMAT_D32_SFLOAT;
	m_propertiesi[NGL_D16_LINEAR_SHADOW_FILTER] = 1;
	m_propertiesi[NGL_D24_LINEAR_SHADOW_FILTER] = 1;
	m_propertiesi[NGL_PIPELINE_MAX_PUSH_CONSTANT_SIZE] = (int32_t)m_PhysicalDeviceProperties.limits.maxPushConstantsSize;
	{
		switch (m_surface_color_format)
		{
		case VK_FORMAT_R8G8B8_UNORM:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_rgb";
			break;
		}
		case VK_FORMAT_R8G8B8_SRGB:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_srgb";
			break;
		}
		case VK_FORMAT_B8G8R8_UNORM:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_bgr";
			break;
		}
		case VK_FORMAT_B8G8R8_SRGB:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_sbgr";
			break;
		}
		case VK_FORMAT_R8G8B8A8_UNORM:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 8888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_rgba";
			break;
		}
		case VK_FORMAT_R8G8B8A8_SRGB:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 8888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_srgba";
			break;
		}
		case VK_FORMAT_B8G8R8A8_UNORM:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 8888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_bgra";
			break;
		}
		case VK_FORMAT_B8G8R8A8_SRGB:
		{
			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = 8888;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = "unorm_sbgra";
			break;
		}
		default:
			std::stringstream s;
			s << m_surface_color_format;

			m_propertiesi[NGL_SWAPCHAIN_COLOR_FORMAT] = -1;
			m_propertiess[NGL_SWAPCHAIN_COLOR_FORMAT] = s.str();
		}
	}
	{
		char tmp[512];
		sprintf(tmp, "0x%0x", m_PhysicalDeviceProperties.vendorID);
		m_propertiess[NGL_VENDOR] = tmp;
	}
	{
		char tmp[512];
		sprintf(tmp, "0x%0x - %s", m_PhysicalDeviceProperties.deviceID, m_PhysicalDeviceProperties.deviceName);
		m_propertiess[NGL_RENDERER] = tmp;
	}
	{
		char tmp[512];
		sprintf(tmp, "0x%0x 0x%0x", m_PhysicalDeviceProperties.apiVersion, m_PhysicalDeviceProperties.driverVersion);
		m_propertiess[NGL_VERSION] = tmp;
	}

	m_propertiesi[NGL_SUBPASS_ENABLED] = 1;
	m_propertiesi[NGL_PIPELINE_STATISTICS] = 1;
	_logf("NGL_PIPELINE_STATISTICS: %d\n", m_propertiesi[NGL_PIPELINE_STATISTICS]);

	_logf("VK_RENDERER: %s\n", m_propertiess[NGL_RENDERER].c_str());
	_logf("VK_VERSION: %s\n", m_propertiess[NGL_VERSION].c_str());
	_logf("VK max. aniso: %d\n", m_propertiesi[NGL_TEXTURE_MAX_ANISOTROPY]);
	_logf("VK max. 2D texture size: %d\n", m_propertiesi[NGL_TEXTURE_MAX_SIZE_2D]);
	_logf("VK max. cube texture size: %d\n", m_propertiesi[NGL_TEXTURE_MAX_SIZE_CUBE]);
	_logf("VK ETC2 support: %d\n", m_propertiesi[NGL_TEXTURE_COMPRESSION_ETC2]);
	_logf("VK DXT support: %d\n", m_propertiesi[NGL_TEXTURE_COMPRESSION_DXT5]);
	_logf("VK ASTC support: %d\n", m_propertiesi[NGL_TEXTURE_COMPRESSION_ASTC]);
	_logf("VK maxMemoryAllocationCount: %u\n", m_PhysicalDeviceProperties.limits.maxMemoryAllocationCount);
	_logf("VK maxPushConstantsSize: %u\n", m_PhysicalDeviceProperties.limits.maxPushConstantsSize);
	_logf("VK minUniformBufferOffsetAlignment: %llu\n", m_PhysicalDeviceProperties.limits.minUniformBufferOffsetAlignment);
	_logf("VK minStorageBufferOffsetAlignment: %llu\n", m_PhysicalDeviceProperties.limits.minStorageBufferOffsetAlignment);
	_logf("VK maxComputeWorkGroupSize: %u, %u, %u\n", m_PhysicalDeviceProperties.limits.maxComputeWorkGroupSize[0], m_PhysicalDeviceProperties.limits.maxComputeWorkGroupSize[1], m_PhysicalDeviceProperties.limits.maxComputeWorkGroupSize[2]);
	_logf("VK maxComputeWorkGroupInvocations: %u\n", m_PhysicalDeviceProperties.limits.maxComputeWorkGroupInvocations);
	_logf("VK maxComputeSharedMemorySize: %u\n", m_PhysicalDeviceProperties.limits.maxComputeSharedMemorySize);
	_logf("VK coherent ubo: %u\n", m_have_coherent_ubo);
	_logf("VK D16 linear filter: %u\n", m_propertiesi[NGL_D16_LINEAR_SHADOW_FILTER]);
	_logf("VK D24 linear filter: %u\n", m_propertiesi[NGL_D24_LINEAR_SHADOW_FILTER]);
}
}


void nglCreateContextVulkan(NGL_context_descriptor& descriptor)
{
#ifdef ANDROID
    int r = InitVulkan();
    if (r == 0)
    {
        _logf("Unable to load vulkan.so!");
        return;
    }
#endif

	new VULKAN::VK_instance;

	VULKAN::VK_instance::This->m_context_descriptor = descriptor;

	VULKAN::VK_instance::This->Init2(descriptor.m_api, descriptor.m_major_version, descriptor.m_minor_version, descriptor.m_enable_validation);
	VULKAN::VK_instance::This->PostInit(descriptor.m_api, descriptor.m_major_version, descriptor.m_minor_version, descriptor.m_enable_validation);
}

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef UINT64_MAX
#define UINT64_MAX  (__UINT64_C(18446744073709551615))
#endif
#ifndef UINT32_MAX
#define UINT32_MAX (0xffffffff)
#endif

#define NUM_FRAMES 2
#define NUM_MAX_CMD_BUFFERS 9
#define UBO_POOL0_SIZE 1024 * 1024
#define UBO_POOL1_SIZE 128 * 1024

template<typename T> void C(T &t, VkStructureType type)
{
	memset(&t, 0, sizeof(T));
	t.sType = type;
}


inline void SetupImageMemoryBarrier(VkPipelineStageFlags &src_stages, VkPipelineStageFlags &dest_stages, VkImageMemoryBarrier &image_memory_barrier)
{
	VkImageLayout &old_image_layout = image_memory_barrier.oldLayout;
	VkImageLayout &new_image_layout = image_memory_barrier.newLayout;

	if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) 
	{
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		dest_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
		if (old_image_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src_stages = VK_PIPELINE_STAGE_HOST_BIT;
		}
		else
		{
			assert(0);
		}
	}
	else if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) 
	{
		image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dest_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		if (old_image_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			src_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src_stages = VK_PIPELINE_STAGE_HOST_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		else
		{
			assert(0);
		}
	}
	else if (new_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) 
	{
		image_memory_barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dest_stages = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
		if (old_image_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src_stages = VK_PIPELINE_STAGE_HOST_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_GENERAL)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else
		{
			assert(0);
		}
	}
	else if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) 
	{
		image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
		dest_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		if (old_image_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			src_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_GENERAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
		else
		{
			assert(0);
		}
	}
	else if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) 
	{
		image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		dest_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
		if (old_image_layout == VK_IMAGE_LAYOUT_GENERAL)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src_stages = VK_PIPELINE_STAGE_HOST_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		}
		else
		{
			assert(0);
		}
	}
	else if (new_image_layout == VK_IMAGE_LAYOUT_GENERAL)
	{
		if (old_image_layout == VK_IMAGE_LAYOUT_UNDEFINED)
		{
			image_memory_barrier.srcAccessMask = 0;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			src_stages = VK_PIPELINE_STAGE_HOST_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		else
		{
			assert(0);
		}
	}
	else if (new_image_layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
	{
		image_memory_barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		if (old_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			image_memory_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			image_memory_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			src_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dest_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		}
		else if (old_image_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		{
			image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		}
	}
	else
	{
		assert(0);
	}
}


inline bool LogVkError(VkResult err, const char* fn_name, int line)
{
	if (err)
	{
		_logf("glVkError: %d line: %d func: %s", err, line, fn_name);

		return true;
	}
	return false;
}
#define LOGVKERROR(err) LogVkError(err, __FUNCTION__, __LINE__)

struct VK_job;

struct VK_staging_texture
{
	VkImage m_image;
	VkDeviceMemory m_mem;
	VkFormat m_format;
	VkExtent3D m_extent;
};

struct VK_texture_transition
{
	VkImageLayout _old;
	VkImageLayout _new;
};


struct VK_texture : public NGL_texture
{
	VkFormat m_format;
	VkImageType m_image_type;
	VkImageViewType m_image_view_type;

	//HACK
	VkImageLayout m_last_layout;

	bool m_was_input_attachment; // Workaround: only set for gbuffer_depth

	VkDeviceMemory m_mem;
	VkImage m_image;
	VkImageView m_resource_view_all;
	std::vector<VkImageView> m_resource_views;
	std::vector<VkImageView> m_render_target_views;
	//0 - standard, 1 - shadow
	VkSampler m_samplers[2];

	VkImageAspectFlags m_aspect;
	VK_texture()
	{
		m_samplers[0] = VK_NULL_HANDLE;
		m_samplers[1] = VK_NULL_HANDLE;
		m_was_input_attachment = false;
	}
};


struct VK_staging_buffer
{
	VkBuffer m_buffer;
	VkDeviceMemory m_mem;
};


struct VK_vertex_buffer : public NGL_vertex_buffer
{
	VkDeviceMemory m_mem;
	VkBuffer m_buffer;
	VK_vertex_buffer() : m_mem(0)
	{
	}
};


struct VK_ubo
{
	VkDeviceMemory m_mem;
	VkBuffer m_buffer;
	uint8_t* m_mapped_ptr;
	size_t m_datasize;
	size_t m_major_offset;
	size_t m_minor_offset;

	VK_ubo()
	{
		m_datasize = 0;
		m_major_offset = 0;
		m_minor_offset = 0;
		m_mapped_ptr = 0;
	}
};


struct VK_index_buffer : public NGL_index_buffer
{
	VkDeviceMemory m_mem;
	VkBuffer m_buffer;
	VkIndexType m_data_type;
	VK_index_buffer() : m_mem(0)
	{
	}
};


struct VkDescriptorInfo
{
	VkDescriptorBufferInfo BufferInfo;
	VkDescriptorImageInfo ImageInfo;
	VkBufferView TexelBufferView;
};


struct VK_descriptor_set
{
	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	VkDescriptorSetLayout m_set_layout;
	std::vector<VkWriteDescriptorSet> m_WriteDescriptorSet;
	std::vector<VkDescriptorInfo> m_DescriptorInfos;
	size_t where_is_the_ubo[NGL_NUM_SHADER_TYPES];

	VK_descriptor_set() : m_set_layout(0)
	{
	}
	void AddUniform(_shader_reflection::Block &mb);
	void CreateWriteDescriptorSets();
};


struct VK_descriptor_pool
{
	VkDescriptorPool m_desc_pool;
	std::vector<VkDescriptorSet> m_desc_sets;
};


struct VK_uniform_group
{
	uint32_t m_aligned_memory_size;
	uint32_t m_ubo_memory_sizes[NGL_NUM_SHADER_TYPES];
	uint8_t *m_ubo_ptrs[NGL_NUM_SHADER_TYPES];

	VK_descriptor_set m_descriptor_set;

	VkPushConstantRange m_range[NGL_NUM_SHADER_TYPES];
	uint8_t m_data[NGL_NUM_SHADER_TYPES][256];

	void SetUpUBO(uint32_t shader_type, VK_ubo &ubo);
	void BindUniform(VK_job *job, const NGL_used_uniform &uu, const void *ptr);
};


struct VK_renderer : public NGL_renderer
{
	struct _GL_layout
	{
		uint32_t loc;
		uint32_t size;
		uint32_t type;
		uint32_t normalized;
		uint32_t offset;
	};

	struct _used_vertex_buffer
	{
		uint32_t m_buffer_idx;
		std::vector<_GL_layout> m_GL_layouts;
	};


	VK_uniform_group m_uniform_groups[3];
	uint32_t m_num_used_uniform_groups_with_descriptor_sets;
	uint32_t m_num_used_desc_pools[NUM_MAX_CMD_BUFFERS];
	std::vector<VK_descriptor_pool> m_desc_pools[NUM_MAX_CMD_BUFFERS];

	VkRenderPass m_renderpass;
	VkPipelineLayout m_pipeline_layout;
	VkPipeline m_pipeline;
	VkPipelineCache m_pipeline_cache;
	std::vector<_used_vertex_buffer> m_used_vbs;
	uint32_t m_used_texture_slots;
	uint32_t m_used_image_slots;

	void Alloc(uint32_t fr, uint32_t num);
	VkDescriptorSet* UseNextSet(uint32_t fr);

	VK_renderer()
	{
		for (int i = 0; i < NUM_MAX_CMD_BUFFERS; i++)
		{
			m_num_used_desc_pools[i] = 0;
		}
	}
	~VK_renderer();
	static uint32_t CompileShader(uint32_t shader_type, const char *str);
	void LinkShader();
	bool GetActiveAttribs2(std::vector<VkVertexInputBindingDescription> &VertexBindingDescriptions, std::vector<VkVertexInputAttributeDescription> &VertexAttributeDescriptions, _shader_reflection &reflection, uint32_t num_vbos, uint32_t *vbos);
	void GetActiveResources3(_shader_reflection &reflection, std::vector<NGL_shader_uniform> &application_uniforms);
};


struct VK_subpass
{
	std::string m_name;
	std::vector<uint32_t> m_color_attachments_remap;
	std::vector<uint32_t> m_depth_attachments_remap;
};


struct VK_pass
{
	VkRenderPass m_renderpass;
	VkFramebuffer m_framebuffer;
	VkRect2D m_renderArea;
	std::vector<VkClearValue> m_attachment_clearvalues;
	std::vector<VK_subpass> m_subpasses;

	VK_texture *m_texture_that_was_input_attachment; // Workaround: set to gbuffer_depth if this is a multipass

	VK_pass() : m_renderpass(VK_NULL_HANDLE), m_framebuffer(VK_NULL_HANDLE)
	{
		m_texture_that_was_input_attachment = nullptr;
	}
};


struct VK_job : public NGL_job
{
	VK_ubo *m_ubo_pool;
	VkCommandBuffer cb;
	std::vector<VkFramebuffer> m_swapchain_framebuffers;
	std::vector<VK_pass> m_passes;

	// Pipeline statistics
	VkQueryPool m_query_pool;
	VkQueryPool m_query_pool_occlusion;
	uint32_t m_draw_call_counter;

	VK_job(uint32_t swapchain_length)
	{
		m_swapchain_framebuffers.resize(swapchain_length);
		for (uint32_t i = 0; i < swapchain_length; i++)
		{
			m_swapchain_framebuffers[i] = VK_NULL_HANDLE;
		}
		m_draw_call_counter = 0;
		m_query_pool = 0;
		m_query_pool_occlusion = 0;
	}
	~VK_job();
	NGL_renderer* CreateRenderer(NGL_state &sh, uint32_t num_vbos, uint32_t *vbos);
	void Reset1();
	VK_ubo& GetUBO(uint32_t group)
	{
		return m_ubo_pool[group];
	}
	VK_subpass& G()
	{
		if (m_passes.size() > 1)
		{
			return m_passes[m_current_state.m_subpass].m_subpasses[0];
		}
		else
		{
			return m_passes[0].m_subpasses[m_current_state.m_subpass];
		}
	}
};


struct BackBuffer
{
	uint32_t image_index;
	VkSemaphore acquire_semaphore;
	VkSemaphore render_semaphore;

	// signaled when this struct is ready for reuse
	VkFence present_fence;

	BackBuffer() : image_index(0)
	{
	}
};


struct VK_cmd_buffer
{
	bool m_first_onscreen;
	bool m_needs_transition_to_present;
	//for each uniform group
	VK_ubo m_ubo_pool[2];
	VkMappedMemoryRange m_ubo_ranges[2];
	VkFence f;
	VkCommandBuffer cb;
};


struct SamplerComparator
{
	bool operator()(const VkSamplerCreateInfo& A, const VkSamplerCreateInfo& B) const
	{
		return memcmp(&A, &B, sizeof(VkSamplerCreateInfo)) > 0;
	}
}; 


struct VK_instance : public NGL_instance
{
	std::vector<VK_vertex_buffer> m_vertex_buffers;
	std::vector<VK_index_buffer> m_index_buffers;
	std::vector<VK_texture> m_textures;
	std::vector<VK_job*> m_jobs;

	VK_instance();
	~VK_instance();
	static VK_instance *This;

	void Init2(NGL_api api, uint32_t major_version, uint32_t minor_version, bool enable_validation);
	void PostInit(NGL_api api, uint32_t major_version, uint32_t minor_version, bool enable_validation);

	PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallbackEXT;
	PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallbackEXT;
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
	PFN_vkCreateWaylandSurfaceKHR fpCreateWaylandSurfaceKHR;
	PFN_vkGetPhysicalDeviceWaylandPresentationSupportKHR fpGetPhysicalDeviceWaylandPresentationSupportKHR;
#endif
#ifdef VK_USE_PLATFORM_SCREEN_QNX
	PFN_vkCreateScreenSurfaceQNX fpCreateScreenSurfaceQNX;
	PFN_vkGetPhysicalDeviceScreenPresentationSupportQNX fpGetPhysicalDeviceScreenPresentationSupportQNX;
#endif
	VkDebugReportCallbackEXT m_debug_msg_callback;
	PFN_vkGetPhysicalDeviceDisplayPropertiesKHR fpGetPhysicalDeviceDisplayPropertiesKHR;
	PFN_vkGetPhysicalDeviceDisplayPlanePropertiesKHR fpGetPhysicalDeviceDisplayPlanePropertiesKHR;
	PFN_vkGetDisplayPlaneSupportedDisplaysKHR fpGetDisplayPlaneSupportedDisplaysKHR;
	PFN_vkGetDisplayModePropertiesKHR fpGetDisplayModePropertiesKHR;
	PFN_vkCreateDisplayModeKHR fpCreateDisplayModeKHR;
	PFN_vkGetDisplayPlaneCapabilitiesKHR fpGetDisplayPlaneCapabilitiesKHR;
	PFN_vkCreateDisplayPlaneSurfaceKHR fpCreateDisplayPlaneSurfaceKHR;
	std::string m_app_name;
	VkDevice m_device;
	VkInstance m_instance;
	VkCommandPool m_cmd_pool;
	std::vector<VK_cmd_buffer> m_cmd_buffers;

	VkPhysicalDevice m_gpu;
	VkPhysicalDeviceProperties m_PhysicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties m_PhysicalDeviceMemoryProperties;
	VkPhysicalDeviceFeatures m_PhysicalDeviceFeatures;
	VkFormatProperties m_format_props[VK_FORMAT_RANGE_SIZE];
	bool m_have_coherent_ubo;
	VkFormat m_D24_format;

	VkColorSpaceKHR m_surface_color_space;
	VkFormat m_surface_color_format;
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swap_chain;
	std::vector<VkImage> m_swapchain_images;
	std::vector<VkImageView> m_swapchain_image_views;
	std::vector<BackBuffer> m_back_buffers;
	uint32_t m_actual_back_buffer_index;
	uint32_t m_swapchain_length;
	bool m_vsync;
	bool m_have_acquired_back_buffer;

	VkQueue m_queue;
	uint32_t m_queue_family;

	void create_surface();
	VkBool32 can_present(uint32_t queue_family);

	void Resize(uint32_t width_hint, uint32_t height_hint);
		
	std::vector<NGL_memory<VkDeviceMemory> > m_vertex_memory;
	std::vector<NGL_memory<VkDeviceMemory> > m_vertex_index_memory;
	std::map<VkSamplerCreateInfo, VkSampler, SamplerComparator> m_samplers;

	NGLStatistic* m_statistic_vector;
	bool m_statistics_enabled;
	
	std::vector<VkDescriptorSet> cached_used_DescriptorSets;
	std::vector<VkWriteDescriptorSet> cached_used_WriteDescriptorSets;
	std::vector<VkImageMemoryBarrier> cached_vk_image_barriers;
	std::vector<VkBufferMemoryBarrier> cached_vk_buffer_barriers;

	uint32_t BeginFrame()
	{
		BackBuffer &buf = m_back_buffers[m_actual_back_buffer_index];

		if (m_have_acquired_back_buffer)
		{
			return buf.image_index;
		}
			
		VkResult err;

		//bevarjuk az elozoleg ram kiadott presentet
		err = vkWaitForFences(m_device, 1, &buf.present_fence, true, UINT64_MAX);
		LOGVKERROR(err);
		// reset the fence
		err = vkResetFences(m_device, 1, &buf.present_fence);
		LOGVKERROR(err);
		
		//beallitjuk hogy az acquire_semaphore jelezze hogy kesz az acquire
		err = vkAcquireNextImageKHR(m_device, m_swap_chain, UINT64_MAX, buf.acquire_semaphore, VK_NULL_HANDLE, &buf.image_index);
		LOGVKERROR(err);

		m_have_acquired_back_buffer = true;

		return buf.image_index;
	}

	void EndFrame()
	{
		VkResult err;

		BackBuffer &buf = m_back_buffers[m_actual_back_buffer_index];

		VkPresentInfoKHR present_info;
		C(present_info, VK_STRUCTURE_TYPE_PRESENT_INFO_KHR);
		present_info.waitSemaphoreCount = 1;
		present_info.pWaitSemaphores = &buf.render_semaphore;
		present_info.swapchainCount = 1;
		present_info.pSwapchains = &m_swap_chain;
		present_info.pImageIndices = &buf.image_index;

		//itt megvarjuk az utolso rajzolas veget a render_semaphore-ral
		err = vkQueuePresentKHR(m_queue, &present_info);
		LOGVKERROR(err);

		////jelezzuk hogy elindult a presentem
		err = vkQueueSubmit(m_queue, 0, nullptr, buf.present_fence);
		LOGVKERROR(err);

		m_actual_back_buffer_index++;
		m_actual_back_buffer_index %= m_swapchain_length;

		m_have_acquired_back_buffer = false;
	}

	void CreateMemory(NGL_memory<VkDeviceMemory> &mem, uint32_t size, VkBufferUsageFlags usage)
	{
		VkMemoryRequirements mem_reqs;
		VkResult err;
		VkBuffer buffer;

		VkBufferCreateInfo buf_info =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			0,
			0,
			size,
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0
		};

		err = vkCreateBuffer(m_device, &buf_info, 0, &buffer);
		LOGVKERROR(err);

		vkGetBufferMemoryRequirements(m_device, buffer, &mem_reqs);

		vkDestroyBuffer(DEVICE, buffer, 0);

		VkMemoryAllocateInfo alloc_info =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			NULL,
			mem_reqs.size,
			0,
		};

		memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_info.memoryTypeIndex);

		mem.m_size = (size_t) alloc_info.allocationSize;

		err = vkAllocateMemory(m_device, &alloc_info, 0, &mem.m_memory);
		LOGVKERROR(err);
	}

	void CreateSSBO(VK_vertex_buffer &vb)
	{
		VkResult err;

		VkBufferCreateInfo buf_info =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			0,
			0,
			vb.m_datasize,
			VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0
		};

		err = vkCreateBuffer(m_device, &buf_info, 0, &vb.m_buffer);
		LOGVKERROR(err);

		VkMemoryRequirements mem_reqs;

		vkGetBufferMemoryRequirements(m_device, vb.m_buffer, &mem_reqs);

		VkMemoryAllocateInfo alloc_info =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			NULL,
			mem_reqs.size,
			0,
		};

		err = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_info.memoryTypeIndex);
		LOGVKERROR(err);

		err = vkAllocateMemory(m_device, &alloc_info, 0, &vb.m_mem);
		LOGVKERROR(err);

		err = vkBindBufferMemory(m_device, vb.m_buffer, vb.m_mem, 0);
		LOGVKERROR(err);
	}
	
	void CreateUBO(VK_ubo &ubo)
	{
		VkResult err;

		VkBufferCreateInfo buf_info =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			0,
			0,
			ubo.m_datasize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0
		};

		err = vkCreateBuffer(m_device, &buf_info, 0, &ubo.m_buffer);
		LOGVKERROR(err);

		VkMemoryRequirements mem_reqs;

		vkGetBufferMemoryRequirements(m_device, ubo.m_buffer, &mem_reqs);

		VkMemoryAllocateInfo alloc_info =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			NULL,
			mem_reqs.size,
			0,
		};

		err = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &alloc_info.memoryTypeIndex);
		
		if (err == VK_ERROR_FORMAT_NOT_SUPPORTED)
		{
			err = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex);
			m_have_coherent_ubo = false;
		}
		else
		{
			m_have_coherent_ubo = true;
		}
		LOGVKERROR(err);

		err = vkAllocateMemory(m_device, &alloc_info, 0, &ubo.m_mem);
		LOGVKERROR(err);

		err = vkBindBufferMemory(m_device, ubo.m_buffer, ubo.m_mem, 0);
		LOGVKERROR(err);

		err = vkMapMemory(DEVICE, ubo.m_mem, 0, ubo.m_datasize, 0, (void**)&ubo.m_mapped_ptr);
		LOGVKERROR(err);
	}


	void CreateVBO(VK_vertex_buffer &vb, void *data)
	{
		VkResult err;

		VkBufferCreateInfo buf_info =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			0,
			0,
			vb.m_datasize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0
		};

		err = vkCreateBuffer(m_device, &buf_info, 0, &vb.m_buffer);
		LOGVKERROR(err);

		VkMemoryRequirements mem_reqs;

		vkGetBufferMemoryRequirements(m_device, vb.m_buffer, &mem_reqs);

		if (!m_vertex_memory.back().TestAllocate((size_t)mem_reqs.size, (size_t)mem_reqs.alignment))
		{
			NGL_memory<VkDeviceMemory> mem;

			CreateMemory(mem, 1024 * 1024, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

			m_vertex_memory.push_back(mem);
		}

		vb.m_memory_pool_offset = m_vertex_memory.back().Allocate((size_t)mem_reqs.size, (size_t)mem_reqs.alignment);

		err = vkBindBufferMemory(DEVICE, vb.m_buffer, m_vertex_memory.back().m_memory, vb.m_memory_pool_offset);
		LOGVKERROR(err);

		if (data)
		{
			CopyBuffer(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vb.m_buffer, vb.m_datasize, data);
		}
	}

	void CreateIBO(VK_index_buffer &ib, void *data)
	{
		VkResult err;

		VkBufferCreateInfo buf_info =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			0,
			0,
			ib.m_datasize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0
		};

		err = vkCreateBuffer(m_device, &buf_info, 0, &ib.m_buffer);
		LOGVKERROR(err);

		VkMemoryRequirements mem_reqs;

		vkGetBufferMemoryRequirements(m_device, ib.m_buffer, &mem_reqs);

		if (!m_vertex_index_memory.back().TestAllocate((size_t)mem_reqs.size, (size_t)mem_reqs.alignment))
		{
			NGL_memory<VkDeviceMemory> mem;

			CreateMemory(mem, 1024 * 1024, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);

			m_vertex_index_memory.push_back(mem);
		}

		ib.m_memory_pool_offset = m_vertex_index_memory.back().Allocate((size_t)mem_reqs.size, (size_t)mem_reqs.alignment);

		err = vkBindBufferMemory(DEVICE, ib.m_buffer, m_vertex_index_memory.back().m_memory, ib.m_memory_pool_offset);
		LOGVKERROR(err);

		if (data)
		{
			CopyBuffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT, ib.m_buffer, ib.m_datasize, data);
		}
	}

	void CreateStagingTexture(VK_staging_texture &tex, VkImageUsageFlags usage)
	{
		VkResult err = VK_SUCCESS;
		VkMemoryRequirements mem_reqs = { 0 };

		VkImageCreateInfo image_create_info =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			NULL,
			0,
			VK_IMAGE_TYPE_2D,
			tex.m_format,
			tex.m_extent,
			1,
			1,
			VK_SAMPLE_COUNT_1_BIT,
			VK_IMAGE_TILING_LINEAR,
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0,
			VK_IMAGE_LAYOUT_PREINITIALIZED
		};

		err = vkCreateImage(m_device, &image_create_info, 0, &tex.m_image);
		LOGVKERROR(err);

		vkGetImageMemoryRequirements(m_device, tex.m_image, &mem_reqs);

		VkMemoryAllocateInfo alloc_info =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			NULL,
			mem_reqs.size,
			0,
		};

		err = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex);
		LOGVKERROR(err);

		err = vkAllocateMemory(m_device, &alloc_info, 0, &tex.m_mem);
		LOGVKERROR(err);

		err = vkBindImageMemory(m_device, tex.m_image, tex.m_mem, 0);
		LOGVKERROR(err);
	}


	void CreateImage(VK_texture &tex, VkImageType type, VkFormat format, VkExtent3D extent, uint32_t num_mipmaps, uint32_t num_arrays, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags create_flags)
	{
		VkResult err;

		VkImageCreateInfo image_create_info =
		{
			VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			NULL,
			create_flags,
			type,
			format,
			extent,
			num_mipmaps,
			num_arrays,
			VK_SAMPLE_COUNT_1_BIT,
			tiling,
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0,
			VK_IMAGE_LAYOUT_PREINITIALIZED
		};

		err = vkCreateImage(m_device, &image_create_info, 0, &tex.m_image);
		LOGVKERROR(err);

		VkMemoryRequirements mem_reqs;

		vkGetImageMemoryRequirements(m_device, tex.m_image, &mem_reqs);

		VkMemoryAllocateInfo alloc_info =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			NULL,
			mem_reqs.size,
			0,
		};

		err = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &alloc_info.memoryTypeIndex);
		LOGVKERROR(err);

		err = vkAllocateMemory(m_device, &alloc_info, 0, &tex.m_mem);
		LOGVKERROR(err);

		err = vkBindImageMemory(m_device, tex.m_image, tex.m_mem, 0);
		LOGVKERROR(err);
	}

	void CreateStagingBuffer(VK_staging_buffer &buf, uint32_t datasize, VkBufferUsageFlags usage)
	{
		VkResult err;
		VkMemoryRequirements mem_reqs;

		VkBufferCreateInfo buf_info =
		{
			VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
			0,
			0,
			datasize,
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			0
		};

		err = vkCreateBuffer(m_device, &buf_info, 0, &buf.m_buffer);
		LOGVKERROR(err);

		vkGetBufferMemoryRequirements(m_device, buf.m_buffer, &mem_reqs);

		VkMemoryAllocateInfo alloc_info =
		{
			VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			NULL,
			mem_reqs.size,
			0,
		};

		err = memory_type_from_properties(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &alloc_info.memoryTypeIndex);
		LOGVKERROR(err);

		err = vkAllocateMemory(m_device, &alloc_info, 0, &buf.m_mem);
		LOGVKERROR(err);

		err = vkBindBufferMemory(m_device, buf.m_buffer, buf.m_mem, 0);
		LOGVKERROR(err);
	}

	void CopyBuffer(VkBufferUsageFlags usage, VkBuffer &dst_buf, uint32_t datasize, void *data)
	{
		VkResult err;
		VK_staging_buffer buf;

		CreateStagingBuffer(buf, datasize, usage);

		{
			uint8_t *mapped_data;

			err = vkMapMemory(m_device, buf.m_mem, 0, datasize, 0, (void **)&mapped_data);
			LOGVKERROR(err);
			memcpy(mapped_data, data, datasize);

			VkMappedMemoryRange range;
			range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
			range.pNext = NULL;
			range.memory = buf.m_mem;
			range.offset = 0;
			range.size = datasize;
			err = vkFlushMappedMemoryRanges(DEVICE, 1, &range);
			LOGVKERROR(err);

			vkUnmapMemory(m_device, buf.m_mem);
		}
		
		VkBufferCopy r =
		{
			0, 0, datasize
		};

		VkCommandBuffer setup_cmd = BeginTemporaryCmdBuffer();

		vkCmdCopyBuffer(setup_cmd, buf.m_buffer, dst_buf, 1, &r);

		EndTemporaryCmdBuffer(setup_cmd);

		vkDestroyBuffer(m_device, buf.m_buffer, 0);
		vkFreeMemory(m_device, buf.m_mem, 0);
	}
	
	VkResult memory_type_from_properties(uint32_t typeBits, VkFlags properties, uint32_t *typeIndex)
	{
		// Search memtypes to find first index with those properties
		for (uint32_t i = 0; i < 32; i++)
		{
			if ((typeBits & 1) == 1) 
			{
				// Type is available, does it match user properties?
				if ((m_PhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) 
				{
					*typeIndex = i;
					return VK_SUCCESS;
				}
			}
			typeBits >>= 1;
		}
		// No memory types matched, return failure
		return VK_ERROR_FORMAT_NOT_SUPPORTED;
	}

	void set_image_layout(VkCommandBuffer cmd_buffer, VkImage image, VkImageLayout old_image_layout, VkImageLayout new_image_layout, VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount, uint32_t baseArrayLayer, uint32_t layerCount)
	{
		VkImageSubresourceRange subresourceRange;

		subresourceRange.aspectMask = aspectMask;
		subresourceRange.baseArrayLayer = baseArrayLayer;
		subresourceRange.baseMipLevel = baseMipLevel;
		subresourceRange.layerCount = layerCount;
		subresourceRange.levelCount = levelCount;
		
		VkImageMemoryBarrier image_memory_barrier =
		{
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			0,
			0,
			old_image_layout,
			new_image_layout,
			m_queue_family,
			m_queue_family,
			image,
			subresourceRange
		};

		VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		SetupImageMemoryBarrier(src_stages, dest_stages, image_memory_barrier);

		VkImageMemoryBarrier *pmemory_barrier = &image_memory_barrier;

		vkCmdPipelineBarrier(cmd_buffer, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1, pmemory_barrier);
	}

	VkCommandBuffer BeginTemporaryCmdBuffer( bool can_be_reused = false)
	{
		VkResult err = VK_SUCCESS;
		VkCommandBuffer cmd_buffer = 0;

		const VkCommandBufferAllocateInfo cmd =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			m_cmd_pool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};

		err = vkAllocateCommandBuffers(m_device, &cmd, &cmd_buffer);
		LOGVKERROR(err);

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
		VkCommandBufferBeginInfo cmd_buf_info =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			NULL,
			(VkCommandBufferUsageFlags)(can_be_reused ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT),
			&cmd_buf_hinfo,
		};
			
		err = vkBeginCommandBuffer(cmd_buffer, &cmd_buf_info);
		LOGVKERROR(err);		
		return cmd_buffer;
	}

	void EndTemporaryCmdBuffer(VkCommandBuffer cmd_buffer, bool do_submit = true)
	{
		VkResult err;

		err = vkEndCommandBuffer(cmd_buffer);
		LOGVKERROR(err);

		if (do_submit)
		{
			const VkCommandBuffer cmd_bufs[] =
			{
				cmd_buffer
			};
			VkFence nullFence = { VK_NULL_HANDLE };

			VkSubmitInfo submit_info =
			{
				VK_STRUCTURE_TYPE_SUBMIT_INFO,
				NULL,
				0,
				NULL,
				0,
				1,
				cmd_bufs,
				0,
				NULL
			};

			err = vkQueueSubmit(m_queue, 1, &submit_info, nullFence);
			LOGVKERROR(err);

			err = vkQueueWaitIdle(m_queue);
			LOGVKERROR(err);

			vkFreeCommandBuffers(m_device, m_cmd_pool, 1, cmd_bufs);
		}
	}

	VkSampler GetSampler(VkSamplerCreateInfo &sampler_info)
	{
		VkSampler result;

		std::map<VkSamplerCreateInfo, VkSampler, SamplerComparator>::iterator S = m_samplers.find(sampler_info);

		if (S == VK_instance::This->m_samplers.end())
		{
			VkResult err = vkCreateSampler(m_device, &sampler_info, 0, &result);
			LOGVKERROR(err);
			
			m_samplers[sampler_info] = result;
		}
		else
		{
			result = S->second;
		}

		return result;
	}
};


inline uint32_t u_align(uint32_t val, uint32_t alignment)
{
	return (val + alignment - 1) & ~(alignment - 1);
}

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ngl.h"
#include "ngl_internal.h"
#include "ngl_metal_adapter_interface.h"

#include <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#include <TargetConditionals.h>

#include <map>
#include <cstdint>


#define NGL_METAL_NUM_UNIFORM_GROUPS 3


// upload to private memory is buggy
#define UPLOAD_TEXTURES_TO_PRIVATE_MEMORY 0


enum Metal_job_status
{
	NGL_JOB_STARTED,
	NGL_JOB_ENDED
};


enum Metal_command_buffer_status
{
	NGL_COMMAND_BUFFER_STARTED,
	NGL_COMMAND_BUFFER_ENDED,
	NGL_COMMAND_BUFFER_SUBMITTED
};


struct Metal_vertex_buffer : public NGL_vertex_buffer
{
    id <MTLBuffer> buffer;
    
	Metal_vertex_buffer();
	~Metal_vertex_buffer();
};


struct Metal_index_buffer : public NGL_index_buffer
{
    id <MTLBuffer> buffer;
    uint32_t m_num_indices;
    MTLIndexType m_Metal_data_type;
    
	Metal_index_buffer();
	~Metal_index_buffer();
};


struct Metal_texture : public NGL_texture
{
    id<MTLTexture> texture;
    MTLPixelFormat pixel_format;
    
    id<MTLSamplerState> sampler;
	
	Metal_texture();
	~Metal_texture();
};


struct Metal_instance;


struct Metal_renderer : public NGL_renderer
{
    //mivel itt a shader, ide kellenek majd a used textures is
    id <MTLRenderPipelineState> m_render_pipeline;
	id <MTLComputePipelineState> m_compute_pipeline;
    bool m_has_ubo[NGL_NUM_SHADER_TYPES][NGL_METAL_NUM_UNIFORM_GROUPS];
	uint32_t m_work_group_size[3];
	uint32_t threads_per_workgroup;
    
    Metal_renderer();
    
    static id<MTLFunction> CompileShader( id<MTLDevice> device, const char *source, const char *entry_point);
    void GetUsedRenderResources(MTLRenderPipelineDescriptor *pipelineDescriptor, uint32_t num_vbos, uint32_t *vbos, const NGL_shader_source_descriptor &ssd);
	void GetUsedComputeResources(id<MTLDevice> device, id<MTLFunction> function);
    void GetUsedArguments(NSArray *arguments, uint32_t shader_type);
};

    
//meg kell tudni osztani fbokat es azok texturait
struct Metal_job : public NGL_job
{
    Metal_job();
    ~Metal_job();
	
	void InitRenderPassDescriptor(uint32_t idx);
	void BeginRenderPass(uint32_t idx);
    
	MTLViewport viewport;
	std::vector<std::vector<NGL_attachment_descriptor*>> m_perpass_attachments;
	std::vector<MTLRenderPassDescriptor*> passDescriptors;
    id<MTLRenderCommandEncoder> renderCommandEncoder;
	id<MTLComputeCommandEncoder> computeCommandEncoder;
	id<MTLCommandBuffer> m_commandBuffer;
	Metal_job_status m_status;
	
	uint32_t m_current_subpass;
	std::vector<bool> m_has_depth_attachment;
	std::vector<int> m_backbuffer_attachment_ids;

	NGL_renderer* CreateRenderer(NGL_state &st, uint32_t num_vbos, uint32_t *vbos);
	
	std::map<MTLCompareFunction, id<MTLDepthStencilState> > m_depth_stencil_states[2];
};


struct Metal_instance : public NGL_instance
{
	const static uint32_t MAX_COMMAND_BUFFER_COUNT = 256;
	
    static Metal_instance *This;
    
    Metal_instance();
    ~Metal_instance();
    
    void Init(NGL_metal_adapter_interface* metal_adapter, uint32_t width, uint32_t height);
        
    std::vector<Metal_vertex_buffer> m_vertex_buffers;
    std::vector<Metal_index_buffer> m_index_buffers;
    std::vector<Metal_texture> m_textures;
    std::vector<Metal_job*> m_jobs;
	
	id<MTLCommandBuffer> m_command_buffers[MAX_COMMAND_BUFFER_COUNT];
	Metal_command_buffer_status m_command_buffer_status[MAX_COMMAND_BUFFER_COUNT];
    
    id <MTLDevice> device;
    id<MTLCommandQueue> commandQueue;

    NGL_metal_adapter_interface *backbuffer_provider;
    uint32_t system_depth;
	
	bool m_use_subpass;
};


bool mtl_GenTexture(uint32_t &buffer, NGL_texture_descriptor &texture_layout, std::vector<std::vector<uint8_t> > *datas);
bool mtl_GenVertexBuffer(uint32_t &buffer, NGL_vertex_descriptor &vertex_layout, uint32_t num, void *data);
bool mtl_GenIndexBuffer(uint32_t &buffer, NGL_format format, uint32_t num, void *data);
bool mtl_GetTextureContent(uint32_t texture_, uint32_t level, uint32_t layer, uint32_t face, NGL_format format, NGL_resource_state state, uint32_t &width, uint32_t &height, std::vector<uint8_t> &data);
bool mtl_GetVertexBufferContent(uint32_t buffer_id, NGL_resource_state state, std::vector<uint8_t> &data);

MTLLoadAction NGLLoadToMTLLoad(NGL_attachment_load_op lo);
MTLStoreAction NGLStoreToMTLStore(NGL_attachment_store_op so);


/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "low_level_tests/tess_base.h"

#include "kcl_io.h"
#include "kcl_camera2.h"
#include "platform.h"

#include "graphics/metalgraphicscontext.h"

#include "metal/mtl_pipeline.h"
#include "metal/mtl_pipeline_builder.h"
#include "metal/fbo_metal.h"
#include "metal/mtl_dynamic_data_buffer.h"


#define ITERATIONS 4


struct TessUniforms
{
	KCL::Matrix4x4 mvp;
	KCL::Matrix4x4 mv;
	KCL::Matrix4x4 model;
	
	KCL::Vector4D cam_near_far_pid_vpscale;
	KCL::Vector4D frustum_planes[6];
	
	KCL::Vector2D view_port_size;
	float time;
	float itercount;
	
	KCL::Vector3D view_pos;
	float _pad1;
};


class TessTest_Metal : public TessBase
{
public:
	TessTest_Metal(const GlobalTestEnvironment * const gte);
	virtual ~TessTest_Metal();
	
	
private:
	id <MTLBuffer> m_vbo;
	id <MTLBuffer> m_ebo;
	
	MetalRender::Pipeline *m_shader_geom;
	MetalRender::Pipeline *m_shader;
	
	MetalRender::Pipeline *m_shader_tc;
	
	virtual float getScore () const { return m_score; }
	virtual const char* getUom() const { return "frames"; }
	virtual bool isWarmup() const { return false; }
	virtual KCL::uint32 indexCount() const { return 0; }
	
	virtual KCL::KCL_Status init ();
	virtual bool render ();
	
	virtual void FreeResources();
	
	void RenderMesh(id <MTLRenderCommandEncoder> render_encoder, id <MTLComputeCommandEncoder> compute_encoder,
					const Mesh &mesh, const MetalRender::Pipeline *shader, TessUniforms &tu);
	
	KCL::KCL_Status LoadShaders();
	void SetBaseDescriptor(MTLPipeLineBuilder &pb);
	
	MTLRenderPassDescriptor *m_render_pass_desc;
	id <MTLTexture> m_depth_texture;
	id <MTLDepthStencilState> m_depth_state;
	
	MetalRender::DynamicDataBufferPool* m_dynamic_data_buffer_pool;
	MetalRender::DynamicDataBuffer* m_dynamic_data_buffer;
	
	id <MTLDevice> m_device;
	id <MTLCommandQueue> m_command_queue;
};


TessTest_Metal::TessTest_Metal(const GlobalTestEnvironment * const gte) : TessBase(gte), m_vbo( 0), m_ebo( 0)
{
	MetalGraphicsContext* context = (MetalGraphicsContext*)gte->GetGraphicsContext() ;
	m_device = context->getDevice();
	m_command_queue = context->getMainCommandQueue();
}


TessTest_Metal::~TessTest_Metal()
{
	FreeResources();
}


KCL::KCL_Status TessTest_Metal::init ()
{
	KCL::KCL_Status s = TessBase::init();
	
	m_dynamic_data_buffer_pool = new MetalRender::DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
	m_dynamic_data_buffer = m_dynamic_data_buffer_pool->GetNewBuffer(16*1024*1024) ;

	MTLPipeLineBuilder::SetScene(KCL::SV_INVALID, nullptr);
	KCL::KCL_Status error = LoadShaders();
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	{
		std::vector<KCL::Vector4D> v4;
		for (size_t i = 0; i < m_vertices.size(); i++)
		{
			KCL::Vector4D t(m_vertices[i]);
			v4.push_back(t);
		}
		
		m_vbo = [m_device newBufferWithBytes:v4[0].v length:sizeof(KCL::Vector4D) * v4.size() options:STORAGE_MODE_MANAGED_OR_SHARED];
	}
	m_ebo = [m_device newBufferWithBytes:&m_indices[0] length:sizeof(KCL::uint16) * m_indices.size() options:STORAGE_MODE_MANAGED_OR_SHARED];

	// create depth texture
	{
		MTLTextureDescriptor *depth_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatDepth32Float
																								  width:m_window_width
																								 height:m_window_height
																							  mipmapped:false];
		depth_tex_desc.storageMode = MTLStorageModePrivate;
		depth_tex_desc.usage = MTLTextureUsageRenderTarget;
		
		m_depth_texture = [m_device newTextureWithDescriptor:depth_tex_desc];
	}
	
	// create render pass descriptor
	{
		m_render_pass_desc = [MTLRenderPassDescriptor renderPassDescriptor];
	
		m_render_pass_desc.colorAttachments[0].loadAction = MTLLoadActionClear;
		m_render_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore;
		m_render_pass_desc.colorAttachments[0].clearColor = MTLClearColorMake(0.0f,0.0f,0.0f,1.0f);
	
		m_render_pass_desc.depthAttachment.texture = m_depth_texture;
		m_render_pass_desc.depthAttachment.loadAction = MTLLoadActionClear;
		m_render_pass_desc.depthAttachment.storeAction = MTLStoreActionDontCare;
		m_render_pass_desc.depthAttachment.clearDepth = 1.0f;
	}
	
	// create depth state
	{
		MTLDepthStencilDescriptor *depth_state_desc = [[MTLDepthStencilDescriptor alloc] init];
	
		depth_state_desc.depthWriteEnabled = YES;
		depth_state_desc.depthCompareFunction = MTLCompareFunctionLess;
	
		m_depth_state = [m_device newDepthStencilStateWithDescriptor:depth_state_desc];
	}

	return s;
}


void TessTest_Metal::SetBaseDescriptor(MTLPipeLineBuilder &sb)
{
	MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];

	desc.maxTessellationFactor = 64;
	desc.tessellationFactorScaleEnabled = false;
	desc.tessellationFactorFormat = MTLTessellationFactorFormatHalf;
	desc.tessellationControlPointIndexType = MTLTessellationControlPointIndexTypeUInt16;
	
	desc.tessellationFactorStepFunction = MTLTessellationFactorStepFunctionPerPatch;
	desc.tessellationOutputWindingOrder = MTLWindingCounterClockwise;
	desc.tessellationPartitionMode = MTLTessellationPartitionModeFractionalOdd;

	sb.SetBaseDescriptor(desc);
}


KCL::KCL_Status TessTest_Metal::LoadShaders()
{
	MTLPipeLineBuilder sb;
	sb.AddShaderDir("shaders_mtl/lowlevel4/");
	KCL::KCL_Status error;
	
	// Compile the tessellation kernel
	sb.ForceHighp(true);
	m_shader_tc = sb.ShaderFile("bezier_tess_tc.shader").Build(error);
	
	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	// Compile first shader with no geometry shader
	sb.AddDefine("USE_TESSELLATION");
	sb.SetType(MetalRender::kShaderTypeSingleBGRA8);
	sb.HasDepth(true);
	SetBaseDescriptor(sb);
	sb.ForceHighp(true);

	m_shader = sb.ShaderFile("bezier_tess.shader").Build(error);

	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	// Compile foreground shader for geometry shading
	sb.AddDefine("USE_GEOMSHADER");
	sb.AddDefine("USE_TESSELLATION");
	sb.SetType(MetalRender::kShaderTypeSingleBGRA8);
	sb.HasDepth(true);
	SetBaseDescriptor(sb);
	sb.ForceHighp(true);

	m_shader_geom = sb.ShaderFile( "bezier_tess.shader").Build(error);

	if (error != KCL::KCL_TESTERROR_NOERROR)
	{
		return error;
	}

	return KCL::KCL_TESTERROR_NOERROR;
}


bool TessTest_Metal::render ()
{
	m_render_pass_desc.colorAttachments[0].texture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
	
	id <MTLCommandBuffer> compute_command_buffer = [m_command_queue commandBuffer];
	id <MTLCommandBuffer> render_command_buffer = [m_command_queue commandBuffer];
	
	id <MTLComputeCommandEncoder> compute_encoder = [compute_command_buffer computeCommandEncoder];
	id <MTLRenderCommandEncoder> render_encoder = [render_command_buffer renderCommandEncoderWithDescriptor:m_render_pass_desc];
	
	[render_encoder setCullMode:MTLCullModeBack];
	[render_encoder setDepthStencilState:m_depth_state];
	
	m_dynamic_data_buffer_pool->InitFrame();
	
	TessUniforms tu;
	
	tu.itercount = ITERATIONS;

	m_shader_tc->SetAsCompute(compute_encoder);
	m_shader->Set(render_encoder);

	tu.time = m_time / 1000.0;
	for (int i = 0; i < 6; i++)
	{
		tu.frustum_planes[i] = m_camera->GetCullPlane(i);
	}
	tu.view_pos = m_camera->GetEye();

	//render close mesh first
	m_mesh1.mv = m_mesh1.model * m_camera->GetView();
	m_mesh1.mvp = m_mesh1.model * m_camera->GetViewProjection();

	m_mesh2.mv = m_mesh2.model * m_camera->GetView();
	m_mesh2.mvp = m_mesh2.model * m_camera->GetViewProjection();

	m_mesh3.mv = m_mesh3.model * m_camera->GetView();
	m_mesh3.mvp = m_mesh3.model * m_camera->GetViewProjection();

	const KCL::uint32 vp_count_x = 4;
	const KCL::uint32 vp_count_y = 4;

	const KCL::uint32 vp_size_x = getViewportWidth() / vp_count_x;
	const KCL::uint32 vp_size_y = getViewportHeight() / vp_count_y;

	float tess_vp_scale = sqrtf(float(vp_size_x) / 1920.0 * float(vp_size_y) / 1080.0);
	tu.cam_near_far_pid_vpscale = KCL::Vector4D(m_camera->GetNear(), m_camera->GetFar(), 0.0, tess_vp_scale);
	tu.view_port_size = KCL::Vector2D(vp_size_x, vp_size_y);

	for (KCL::uint32 x = 0; x < vp_count_x; x++)
	{
		for (KCL::uint32 y = 0; y < vp_count_y; y++)
		{
			MTLViewport vp = { (double)vp_size_x * x, (double)vp_size_y * y, (double)vp_size_x, (double)vp_size_y, 0.5, 1.0 };
			[render_encoder setViewport:vp];
			
			RenderMesh(render_encoder, compute_encoder, m_mesh1, m_shader, tu);
			RenderMesh(render_encoder, compute_encoder, m_mesh2, m_shader, tu);
			RenderMesh(render_encoder, compute_encoder, m_mesh3, m_shader, tu);
		}
	}

	// Change to geometry shader for foreground
	m_shader_geom->Set(render_encoder);

	{
		MTLViewport vp = { 0.0, 0.0, (double)getViewportWidth(), (double)getViewportHeight(), 0.0, 0.5 };
		[render_encoder setViewport:vp];
	}

	tess_vp_scale = sqrtf(float(getViewportWidth()) / 1920.0 * float(getViewportHeight()) / 1080.0);
	tu.cam_near_far_pid_vpscale = KCL::Vector4D(m_camera->GetNear(), m_camera->GetFar(), 0.0, tess_vp_scale);
	tu.view_port_size = KCL::Vector2D(getViewportWidth(), getViewportHeight());

	tu.time = m_time / 1000.0;
	for (int i = 0; i < 6; i++)
	{
		tu.frustum_planes[i] = m_camera->GetCullPlane(i);
	}
	tu.view_pos = m_camera->GetEye();

	RenderMesh(render_encoder, compute_encoder, m_mesh1, m_shader_geom, tu);
	RenderMesh(render_encoder, compute_encoder, m_mesh3, m_shader_geom, tu);
	
	[compute_encoder endEncoding];
	[compute_command_buffer commit];
	
	[render_encoder endEncoding];
	
	unsigned char slot = m_dynamic_data_buffer_pool->GetCurrentSlot();
	[render_command_buffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
	 {
		 m_dynamic_data_buffer_pool->MarkSlotUnused(slot);
	 }];
	
	[render_command_buffer commit];

	return true;
}


void TessTest_Metal::RenderMesh(id <MTLRenderCommandEncoder> render_encoder, id <MTLComputeCommandEncoder> compute_encoder,
								const Mesh &mesh, const MetalRender::Pipeline *shader, TessUniforms &tu)
{
	tu.mvp = mesh.mvp;
	tu.mv = mesh.mv;
	tu.model = mesh.model;
	
	// MTL_TESS_TODO calc required data appropirate
	uint32_t patch_param_offset = m_dynamic_data_buffer->ReserveAndGetOffset(128*1024);
	uint32_t tess_factor_offset = m_dynamic_data_buffer->ReserveAndGetOffset(128*1024);
	
	// MTL_TESS_TODO any other way to calc patch count
	assert(m_vertices.size() % 16 == 0);
	uint32_t patch_count = m_vertices.size()/16;
	
	// compute encoding
	{
		[compute_encoder setBytes:&tu length:sizeof(TessUniforms) atIndex:0];
		[compute_encoder setBuffer:m_vbo offset:0 atIndex:1];
		[compute_encoder setBuffer:m_dynamic_data_buffer->GetCurrentBuffer() offset:patch_param_offset atIndex:2];
		[compute_encoder setBuffer:m_dynamic_data_buffer->GetCurrentBuffer() offset:tess_factor_offset atIndex:3];
		
		// MTL_TESS_TODO
		MTLSize threadsPerGroup = { 1, 1, 1 };
		MTLSize numThreadgroups = { patch_count, 1, 1 };
		
		[compute_encoder dispatchThreadgroups:numThreadgroups
						threadsPerThreadgroup:threadsPerGroup];
	}
	
	// render encoding
	{
		[render_encoder setVertexBytes:&tu length:sizeof(TessUniforms) atIndex:0];
		[render_encoder setFragmentBytes:&tu length:sizeof(TessUniforms) atIndex:0];
		
		[render_encoder setVertexBuffer:m_dynamic_data_buffer->GetCurrentBuffer() offset:patch_param_offset atIndex:1];
		
		[render_encoder setTessellationFactorBuffer:m_dynamic_data_buffer->GetCurrentBuffer() offset:tess_factor_offset instanceStride:0];
		
		[render_encoder drawIndexedPatches:16
								patchStart:0
								patchCount:patch_count
						  patchIndexBuffer:nil
			        patchIndexBufferOffset:0
			       controlPointIndexBuffer:m_ebo
	         controlPointIndexBufferOffset:0
		                     instanceCount:1
							  baseInstance:0];
	}
}


void TessTest_Metal::FreeResources()
{
	MetalRender::Pipeline::ClearCashes();

	releaseObj(m_vbo);
	releaseObj(m_ebo);
}


GLB::TestBase *CreateTessTestMetal(const GlobalTestEnvironment* const gte)
{
	return new TessTest_Metal(gte);
}

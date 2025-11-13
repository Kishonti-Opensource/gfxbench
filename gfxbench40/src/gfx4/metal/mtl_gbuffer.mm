/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_gbuffer.h"
#include "mtl_scene_40.h"
#include "metal/mtl_pipeline_builder.h"
#include "platform.h"

using namespace GFXB4;

#define USE_HIZ_GEN_MIPMAPS 1

GBuffer::GBuffer() :
	m_device(MetalRender::GetContext()->getDevice())
{
    m_albedo_texture = nil;
    m_depth_texture = nil;
    m_velocity_texture = nil;
    m_normal_texture = nil;
    m_params_texture = nil;
    m_transparent_accum_texture = nil;
	m_depth_hiz_texture = nil;

	m_gbuffer_pass = nil;
	m_transparent_accum_pass = nil;

    m_viewport_width = 0;
    m_viewport_height = 0;
    m_hiz_depth_levels = 0;
}

GBuffer::~GBuffer()
{
	releaseObj(m_albedo_texture);
	releaseObj(m_depth_texture);
	releaseObj(m_velocity_texture);
	releaseObj(m_normal_texture);
	releaseObj(m_params_texture);
	releaseObj(m_transparent_accum_texture);

	releaseObj(m_gbuffer_pass);
	releaseObj(m_transparent_accum_pass);
}


bool GBuffer::Init(KCL::uint32 width, KCL::uint32 height)
{
    m_viewport_width = width;
    m_viewport_height = height;

	// Create the textures
	MTLTextureDescriptor * desc = [[MTLTextureDescriptor alloc] init];
	desc.width = m_viewport_width;
	desc.height = m_viewport_height;
	desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
	desc.storageMode = MTLStorageModePrivate;
#endif

	desc.pixelFormat = MTLPixelFormatDepth32Float;
	m_depth_texture = [m_device newTextureWithDescriptor:desc];

	desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
	m_albedo_texture = [m_device newTextureWithDescriptor:desc];
	m_params_texture =  [m_device newTextureWithDescriptor:desc];
	m_transparent_accum_texture = [m_device newTextureWithDescriptor:desc];

	desc.pixelFormat = VELOCITY_BUFFER_RGBA8 ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGB10A2Unorm;
	m_velocity_texture = [m_device newTextureWithDescriptor:desc];

	desc.pixelFormat = NORMAL_BUFFER_RGBA8 ? MTLPixelFormatRGBA8Unorm : MTLPixelFormatRGB10A2Unorm;
	m_normal_texture = [m_device newTextureWithDescriptor:desc];

	releaseObj(desc);


	// Create the render passes
	m_gbuffer_pass = [[MTLRenderPassDescriptor alloc] init];

	m_gbuffer_pass.colorAttachments[0].texture = m_albedo_texture;
	m_gbuffer_pass.colorAttachments[0].loadAction = MTLLoadActionClear;
	m_gbuffer_pass.colorAttachments[0].clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
	m_gbuffer_pass.colorAttachments[0].storeAction = MTLStoreActionStore;

	m_gbuffer_pass.colorAttachments[1].texture = m_velocity_texture;
	m_gbuffer_pass.colorAttachments[1].loadAction = MTLLoadActionClear;
#if VELOCITY_BUFFER_RGBA8
	m_gbuffer_pass.colorAttachments[1].clearColor = MTLClearColorMake(0.499f, 0.499f, 0.499f, 0.499f);
#else
	m_gbuffer_pass.colorAttachments[1].clearColor = MTLClearColorMake(0.5f, 0.5f, 0.5f, 0.5f);
#endif
	m_gbuffer_pass.colorAttachments[1].storeAction = MTLStoreActionStore;

	m_gbuffer_pass.colorAttachments[2].texture = m_normal_texture;
	m_gbuffer_pass.colorAttachments[2].loadAction = MTLLoadActionClear;
	m_gbuffer_pass.colorAttachments[2].clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
	m_gbuffer_pass.colorAttachments[2].storeAction = MTLStoreActionStore;

	m_gbuffer_pass.colorAttachments[3].texture = m_params_texture;
	m_gbuffer_pass.colorAttachments[3].loadAction = MTLLoadActionClear;
	m_gbuffer_pass.colorAttachments[3].clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
	m_gbuffer_pass.colorAttachments[3].storeAction = MTLStoreActionStore;

	m_gbuffer_pass.depthAttachment.texture = m_depth_texture;
	m_gbuffer_pass.depthAttachment.loadAction = MTLLoadActionClear;
	m_gbuffer_pass.depthAttachment.clearDepth = 1.0f;
	m_gbuffer_pass.depthAttachment.storeAction = MTLStoreActionStore;


	m_transparent_accum_pass = [[MTLRenderPassDescriptor alloc] init];

	m_transparent_accum_pass.colorAttachments[0].texture = m_transparent_accum_texture;
	m_transparent_accum_pass.colorAttachments[0].loadAction = MTLLoadActionClear;
	m_transparent_accum_pass.colorAttachments[0].clearColor = MTLClearColorMake(0.0f, 0.0f, 0.0f, 0.0f);
	m_transparent_accum_pass.colorAttachments[0].storeAction = MTLStoreActionStore;

	m_transparent_accum_pass.depthAttachment.texture = m_depth_texture;
	m_transparent_accum_pass.depthAttachment.loadAction = MTLLoadActionLoad;
	m_transparent_accum_pass.depthAttachment.storeAction = MTLStoreActionStore;

	// Hi-Z depth texture
	KCL::uint32 max_texture_levels = KCL::uint32(KCL::texture_levels(m_viewport_width, m_viewport_height));
	m_hiz_depth_levels = max_texture_levels < DEPTH_HIZ_LEVELS ? max_texture_levels : DEPTH_HIZ_LEVELS;

	MTLTextureDescriptor * hiz_desc = [[MTLTextureDescriptor alloc] init];

	hiz_desc.textureType = MTLTextureType2D;
	hiz_desc.pixelFormat = MTLPixelFormatRGBA8Unorm;
	hiz_desc.width = m_viewport_width;
	hiz_desc.height = m_viewport_height;
	hiz_desc.mipmapLevelCount = m_hiz_depth_levels;
	hiz_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_IPHONE
	hiz_desc.storageMode = MTLStorageModePrivate;
#endif

	m_depth_hiz_texture = [m_device newTextureWithDescriptor:hiz_desc];
	releaseObj(hiz_desc);

	// Set up the samplers
	MTLSamplerDescriptor * sampler_desc = [[MTLSamplerDescriptor alloc] init];

	sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.rAddressMode = MTLSamplerAddressModeClampToEdge;

	sampler_desc.minFilter = MTLSamplerMinMagFilterNearest;
	sampler_desc.magFilter = MTLSamplerMinMagFilterNearest;
	sampler_desc.mipFilter = MTLSamplerMipFilterNotMipmapped;

	m_nearest_sampler = [m_device newSamplerStateWithDescriptor:sampler_desc];

	sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
	sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
	sampler_desc.mipFilter = MTLSamplerMipFilterLinear;

	m_linear_mipmap_sampler = [m_device newSamplerStateWithDescriptor:sampler_desc];

    return InitShaders();
}

bool GBuffer::InitShaders()
{
    KCL::KCL_Status status;
	MTLPipeLineBuilder sb;

	sb.SetVertexLayout(MetalRender::QuadBuffer::GetVertexLayout());

    INFO("Enable depth hiz: %d", HIZ_DEPTH_ENABLED);

    m_linearize_shader = sb.ShaderFile("linearize_depth.shader").Build(status);
    if ( status != KCL::KCL_TESTERROR_NOERROR)
    {
        return false;
    }

#if !USE_HIZ_GEN_MIPMAPS
    m_downsample_shader = sb.ShaderFile("downsample_depth.shader").Build( status);
    if ( status != KCL::KCL_TESTERROR_NOERROR)
    {
        return false;
    }

    GLB::OpenGLStateManager::GlUseProgram(m_downsample_shader->m_p);
    glUniform1i(m_downsample_shader->m_uniform_locations[GLB::uniforms::texture_unit0], 0);
#endif

	return true;
}

void GBuffer::DownsampleDepth(id<MTLCommandBuffer> command_buffer, const KCL::Camera2 *camera, MetalRender::QuadBuffer * quad_buffer)
{
    if (!HIZ_DEPTH_ENABLED)
    {
        return;
    }

	MTLRenderPassDescriptor * desc = [[MTLRenderPassDescriptor alloc] init];

	desc.colorAttachments[0].texture = m_depth_hiz_texture;
	desc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
	desc.colorAttachments[0].storeAction = MTLStoreActionStore;

	id <MTLRenderCommandEncoder> render_encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
	render_encoder.label = @"Downsample Depth";
	releaseObj(desc);

    KCL::Vector4D depth_linearize_factors;
    depth_linearize_factors.x = camera->GetNear() - camera->GetFar();
    depth_linearize_factors.z = camera->GetNear();
    depth_linearize_factors.w = camera->GetFar();

	m_linearize_shader->Set(render_encoder);
	[render_encoder setFragmentTexture:m_depth_texture atIndex:DEPTH_UNIT0_SLOT];
	[render_encoder setFragmentSamplerState:m_nearest_sampler atIndex:DEPTH_UNIT0_SLOT];
	[render_encoder setFragmentBytes:&depth_linearize_factors length:sizeof(KCL::Vector4D) atIndex:0];

	quad_buffer->Draw(render_encoder);

	[render_encoder endEncoding];

#if USE_HIZ_GEN_MIPMAPS
	id <MTLBlitCommandEncoder> blit_encoder = [command_buffer blitCommandEncoder];
	blit_encoder.label = @"Generate Downsampled Depth Mipmaps";

	[blit_encoder generateMipmapsForTexture:m_depth_hiz_texture];
	[blit_encoder endEncoding];
#else
	#error "USE_HIZ_GEN_MIPMAPS = 0 not implemented."
#endif
}

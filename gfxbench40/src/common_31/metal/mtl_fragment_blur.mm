/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_fragment_blur.h"
#include "gauss_blur_helper.h"

#include "platform.h"
#include "mtl_pipeline_builder.h"

#include "gauss_blur_fs.h"

using namespace MetalRender;

FragmentBlur::FragmentBlur(id <MTLDevice> device)
{
    m_device = device ;
    
	m_width = 0;
    m_height = 0;
    m_lod_levels = 0;
    m_has_lod = false;

	m_sampler = 0;

	m_blur_shader_h = NULL;
	m_blur_shader_v = NULL;
    
    m_in_tex = nil;
    m_temp_tex = nil;
    m_out_tex = nil;
    
    m_sampler = nil;
}

FragmentBlur::~FragmentBlur()
{
    delete m_quadBuffer ;
    releaseObj(m_quadBufferVertexLayout) ;
    
    releaseObj(m_in_tex) ;
    releaseObj(m_temp_tex) ;
    releaseObj(m_out_tex) ;
    
    releaseObj(m_sampler) ;
    
    releaseObj(m_device) ;
}

void FragmentBlur::Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, MTLPixelFormat texture_format, KCL::uint32 lod_levels)
{
    m_width = width;
    m_height = height;
    m_lod_levels = lod_levels;
    m_has_lod = m_lod_levels > 1;
    
    m_quadBuffer = new MetalRender::QuadBuffer(QuadBuffer::kBlitQuadLandscape) ;
    m_quadBufferVertexLayout = MetalRender::QuadBuffer::GetVertexLayout() ;

    blur_kernel_size = KCL::Max(blur_kernel_size, 2u);
    
	// Generate the shader constants
    std::vector<float> gauss_weights = COMMON31::GaussBlurHelper::GetGaussWeights(blur_kernel_size, true);
    packed_weights = COMMON31::GaussBlurHelper::CalcPackedWeights(gauss_weights);

    std::string shader_file;
    if (m_has_lod)
    {   
        horizontal_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(1, blur_kernel_size, gauss_weights);
        vertical_packed_offsets = horizontal_packed_offsets;

         // Calc step values for gauss	   
	    stepuv.resize(lod_levels);

        KCL::uint32 k = 1;
	    for (KCL::uint32 i = 0; i < lod_levels; i++)
	    {
		    stepuv[i].x = 1.0 / float(std::max(m_width / k,  1u));
		    stepuv[i].y = 1.0 / float(std::max(m_height / k, 1u));
		    k *= 2;
	    }
        shader_file = "gauss_blur_fs_lod.shader";
    }
    else
    {
        vertical_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(m_height, blur_kernel_size, gauss_weights);
        horizontal_packed_offsets = COMMON31::GaussBlurHelper::CalcPackedOffsets(m_width, blur_kernel_size, gauss_weights);
        shader_file = "gauss_blur_fs.shader";
    }

    // Compile the vertical shader
    KCL::KCL_Status error;
	MTLPipeLineBuilder sb;
    sb.ShaderFile(shader_file.c_str());
    sb.AddDefine("VERTICAL");        
    sb.AddDefineInt("KS", blur_kernel_size + 1);
    sb.SetVertexLayout(m_quadBufferVertexLayout);
	if (m_has_lod)
	{
		sb.AddDefineInt("LOD_LEVEL_COUNT", lod_levels) ;
	}
    m_blur_shader_v = sb.Build(error);

    // Compile the horizontal shader
    sb.ShaderFile(shader_file.c_str());
    sb.AddDefine("HORIZONTAL");        
    sb.AddDefineInt("KS", blur_kernel_size + 1);
    sb.SetVertexLayout(m_quadBufferVertexLayout);
	if (m_has_lod)
	{
		sb.AddDefineInt("LOD_LEVEL_COUNT", lod_levels) ;
	}
    m_blur_shader_h = sb.Build(error);

    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];
    samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
    samplerDesc.mipFilter = m_has_lod ? MTLSamplerMipFilterLinear : MTLSamplerMipFilterNotMipmapped;
    
    samplerDesc.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
    samplerDesc.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
    samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;

    m_sampler = [m_device newSamplerStateWithDescriptor:samplerDesc];
    
	// Setup the textures
    MTLTextureDescriptor *tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:texture_format
                                                       width:width
                                                      height:height
                                                   mipmapped:m_has_lod];
    if (m_has_lod)
    {
        tex_desc.mipmapLevelCount = m_lod_levels;
    }
    
    tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
#if !TARGET_OS_EMBEDDED
    tex_desc.storageMode = MTLStorageModePrivate;
#endif
    
    m_temp_tex = [m_device newTextureWithDescriptor:tex_desc];
    m_out_tex  = [m_device newTextureWithDescriptor:tex_desc];
}


void FragmentBlur::Execute(id <MTLCommandBuffer> commandBuffer)
{
    // Execute the vertical pass
    for (KCL::uint32 i = 0; i < m_lod_levels; i++)
    {
        MTLRenderPassDescriptor* verticalPassDescriptor = [[MTLRenderPassDescriptor alloc] init];

        verticalPassDescriptor.colorAttachments[0].texture = m_temp_tex ;
        verticalPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare ;
        verticalPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore ;
        verticalPassDescriptor.colorAttachments[0].level = i ;
        
        id <MTLRenderCommandEncoder> render_encoder = [commandBuffer renderCommandEncoderWithDescriptor:verticalPassDescriptor] ;
		render_encoder.label = @"Fragment Blur V";
        releaseObj(verticalPassDescriptor) ;
        
        m_blur_shader_v->Set(render_encoder) ;
        
        // Setup the vertical shader
        [render_encoder setFragmentBytes:&vertical_packed_offsets[0] length:vertical_packed_offsets.size()*sizeof(float) atIndex:FB_GAUSS_OFFSETS_BFR_SLOT];
        [render_encoder setFragmentBytes:&packed_weights[0]          length:packed_weights.size()*sizeof(float)          atIndex:FB_GAUSS_WEIGHTS_BFR_SLOT];
        
        if (m_has_lod)
        {
            [render_encoder setFragmentBytes:&i length:sizeof(KCL::uint32) atIndex:FB_GAUSS_LOD_LEVEL_BFR_SLOT] ;
            [render_encoder setFragmentBytes:stepuv[0].v length:stepuv.size()*sizeof(KCL::Vector2D) atIndex:FB_INV_RESOLUTION_BFR_SLOT] ;
        }
        
        [render_encoder setFragmentTexture:m_in_tex       atIndex:FB_IN_TEXTURE_SLOT];
        [render_encoder setFragmentSamplerState:m_sampler atIndex:FB_SAMPLER_SLOT];
        
        m_quadBuffer->Draw(render_encoder) ;
        
        [render_encoder endEncoding] ;
        releaseObj(render_encoder) ;
    }


    // Execute the horizontal pass
    for (KCL::uint32 i = 0; i < m_lod_levels; i++)
    {
        MTLRenderPassDescriptor* horizontalPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
        
        horizontalPassDescriptor.colorAttachments[0].texture = m_out_tex ;
        horizontalPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare ;
        horizontalPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore ;
        horizontalPassDescriptor.colorAttachments[0].level = i ;
        
        id <MTLRenderCommandEncoder> render_encoder = [commandBuffer renderCommandEncoderWithDescriptor:horizontalPassDescriptor] ;
		render_encoder.label = @"Fragment Blur H";		
        releaseObj(horizontalPassDescriptor) ;
        
        m_blur_shader_h->Set(render_encoder) ;
        
        // Setup the horizontal shader
        [render_encoder setFragmentBytes:&horizontal_packed_offsets[0] length:horizontal_packed_offsets.size()*sizeof(float) atIndex:FB_GAUSS_OFFSETS_BFR_SLOT];
        [render_encoder setFragmentBytes:&packed_weights[0]            length:packed_weights.size()*sizeof(float)            atIndex:FB_GAUSS_WEIGHTS_BFR_SLOT];
        
        if (m_has_lod)
        {
            [render_encoder setFragmentBytes:&i length:sizeof(KCL::uint32) atIndex:FB_GAUSS_LOD_LEVEL_BFR_SLOT] ;
            [render_encoder setFragmentBytes:stepuv[0].v length:stepuv.size()*sizeof(KCL::Vector2D) atIndex:FB_INV_RESOLUTION_BFR_SLOT] ;
        }
        
        [render_encoder setFragmentTexture:m_temp_tex     atIndex:FB_IN_TEXTURE_SLOT];
        [render_encoder setFragmentSamplerState:m_sampler atIndex:FB_SAMPLER_SLOT];

        m_quadBuffer->Draw(render_encoder) ;
        
        [render_encoder endEncoding] ;
        releaseObj(render_encoder) ;
    }
}


void FragmentBlur::SetInputTexture(id <MTLTexture> in_tex)
{
    m_in_tex = in_tex;
}


id <MTLTexture> FragmentBlur::GetOutputTexture() const
{
    return m_out_tex;
}


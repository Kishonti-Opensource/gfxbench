/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_compute_hdr40.h"

#include "metal/mtl_pipeline_builder.h"

typedef KCL::Vector4D hfloat4;

#include "bright_pass40.h"
#include "hdr_consts40.h"

#include "ng/log.h"

const KCL::uint32 ComputeHDR40::BLUR_TEXTURE_COUNT ;

ComputeHDR40::ComputeHDR40(id <MTLDevice> device)
{
    m_device = device ;

	m_bright_pass = NULL;
    m_fragment_blur = NULL;
    m_reduction = NULL;

    m_input_texture = 0;    
    m_bloom_texture_type = MTLPixelFormatInvalid;
	m_compute_bright_pass = false;
}

ComputeHDR40::~ComputeHDR40()
{
    releaseObj(m_input_sampler) ;
    releaseObj(m_bloom_sampler) ;
    releaseObj(m_bloom_texture) ;
    
    delete m_fragment_blur;
    delete m_reduction;
    
    releaseObj(m_device) ;
}

void ComputeHDR40::Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type, bool use_compute_bright_pass, KRL_Scene* scene)
{
    m_width = width;
    m_height = height;

    m_bloom_texture_type = bloom_texture_gl_type;
        
	KCL::uint32 blur_kernel_size = 9; // blur kernel size for FullHD
    
    KCL::uint32 m_dim = KCL::Max(m_width, m_height);
	blur_kernel_size = blur_kernel_size * m_dim / 1920; // resize blur kernel for actual resolution

    m_fragment_blur = new MetalRender::FragmentBlur(m_device);
    m_fragment_blur->Init(width / DOWNSAMPLE, height / DOWNSAMPLE, blur_kernel_size, bloom_texture_gl_type, 4);

    // Init reduction pass
    m_reduction = new MetalRender::ComputeReduction40(m_device);
    m_reduction->Init(m_width, m_height, scene);
    
    // Init bright pass
	m_compute_bright_pass = use_compute_bright_pass;
	if (use_compute_bright_pass)
	{
		m_bright_pass = new ComputeBrightPass();
	}
	else
	{
		m_bright_pass = new FragmentBrightPass();
	}
	m_bright_pass->Init(width / DOWNSAMPLE, height / DOWNSAMPLE, bloom_texture_gl_type);
	
	SetupBloomTexture();
}

void ComputeHDR40::SetInputTexture(id <MTLTexture> in_texture)
{
    m_input_texture = in_texture; 
    m_reduction->SetInputTexture(m_input_texture);
}

id <MTLTexture> ComputeHDR40::GetBloomTexture() const
{
    return m_fragment_blur->GetOutputTexture();
}

id <MTLSamplerState> ComputeHDR40::GetBloomSampler() const
{
    return m_bloom_sampler;
}

id <MTLBuffer> ComputeHDR40::GetLuminanceBuffer() const
{
    return m_reduction->GetLuminanceBuffer();
}

MetalRender::ComputeReduction40 * ComputeHDR40::GetComputeReduction() const
{
	return m_reduction;
}

void ComputeHDR40::Execute(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame)
{    
    m_reduction->Execute(command_buffer, ubo_frame);
	
	// Execute the bright pass
	m_bright_pass->Execute(command_buffer, m_input_texture,m_input_sampler,m_bloom_texture, GetLuminanceBuffer(), ubo_frame);
	
	// Downsample the bright texture
	id<MTLBlitCommandEncoder> blitEncoder = [command_buffer blitCommandEncoder];
	blitEncoder.label = @"Generate Bright Pass Mipmaps";
	
	[blitEncoder generateMipmapsForTexture:m_bloom_texture];
	[blitEncoder endEncoding];
	
	// Blur the bright texture
	m_fragment_blur->SetInputTexture(m_bloom_texture);
	m_fragment_blur->Execute(command_buffer);
}

void ComputeHDR40::SetupBloomTexture()
{    
    // Create a linear sampler for input
    MTLSamplerDescriptor *inputSamplerDesc = [[MTLSamplerDescriptor alloc] init];
    inputSamplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    inputSamplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
    inputSamplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
    
    inputSamplerDesc.sAddressMode = MTLSamplerAddressModeRepeat;
    inputSamplerDesc.tAddressMode = MTLSamplerAddressModeRepeat;
    inputSamplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    m_input_sampler = [m_device newSamplerStateWithDescriptor:inputSamplerDesc];

    // Create the bright texture
    unsigned int width = m_width / DOWNSAMPLE;
	unsigned int height = m_height / DOWNSAMPLE;
    
    MTLTextureDescriptor *bright_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:m_bloom_texture_type
                                                                                               width:width
                                                                                              height:height
                                                                                           mipmapped:true];
	bright_tex_desc.usage = (m_compute_bright_pass?MTLTextureUsageShaderWrite:MTLTextureUsageRenderTarget) | MTLTextureUsageShaderRead;
    
    bright_tex_desc.mipmapLevelCount = BLUR_TEXTURE_COUNT ;
    
    m_bloom_texture = [m_device newTextureWithDescriptor:bright_tex_desc];

    
    // Create a sampler to help bloom sampling
    MTLSamplerDescriptor *bloomSamplerDesc = [[MTLSamplerDescriptor alloc] init];
    bloomSamplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
    bloomSamplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
    bloomSamplerDesc.mipFilter = MTLSamplerMipFilterLinear;
    
    bloomSamplerDesc.sAddressMode = MTLSamplerAddressModeMirrorRepeat;
    bloomSamplerDesc.tAddressMode = MTLSamplerAddressModeMirrorRepeat;
    bloomSamplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
    
    m_bloom_sampler = [m_device newSamplerStateWithDescriptor:bloomSamplerDesc];
}


ComputeBrightPass::ComputeBrightPass()
{
	m_work_group_size_x = 0;
	m_work_group_size_y = 0;
	m_dispatch_count_x = 0;
	m_dispatch_count_y = 0;
	
	m_bloom_texture_gl_type = 0;
}

ComputeBrightPass::~ComputeBrightPass()
{
}

void ComputeBrightPass::Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type)
{
	NGLOG_INFO("ComputeBrightPass - Init: %sx%s", width, height);
	
	m_bloom_texture_gl_type = bloom_texture_gl_type;

	KCL::KCL_Status error;
	MTLPipeLineBuilder sb;
	sb.ShaderFile("bright_pass40.shader") ;
	sb.AddDefineInt("WORK_GROUP_SIZE_X", m_work_group_size_x);
	sb.AddDefineInt("WORK_GROUP_SIZE_Y", m_work_group_size_y);
	sb.AddDefineVec2("STEP_UV", KCL::Vector2D(1.0f / float(width) / float(ComputeHDR40::DOWNSAMPLE), 1.0f / float(height) / float(ComputeHDR40::DOWNSAMPLE)));
	sb.ForceHighp(true);
	m_bright_pass = sb.Build(error);
	
	SetWorkGroupSize();

	NGLOG_INFO("ComputeBrightPass - Work groups: %sx%s", m_work_group_size_x, m_work_group_size_y);
	
	m_dispatch_count_x = (width + m_work_group_size_x - 1) / m_work_group_size_x;
	m_dispatch_count_y = (height + m_work_group_size_y - 1) / m_work_group_size_y;
}

void ComputeBrightPass::SetWorkGroupSize()
{
	uint32_t wgsizes[] = { 32, 16, 8 };
	
	int i = 0;
	for (; i < 3; i++)
	{
		if (m_bright_pass->IsThreadCountOk("bright_pass", wgsizes[i]*wgsizes[i]))
		{
			m_work_group_size_x =  m_work_group_size_y = wgsizes[i];
			return;
		}
	}
	
	NGLOG_ERROR("ComputeBrightPass - No suitable WG configuration!");
	m_work_group_size_x = 1;
	m_work_group_size_y = 1;
}

void ComputeBrightPass::Execute(id <MTLCommandBuffer> command_buffer, id <MTLTexture> m_input_texture, id <MTLSamplerState> m_input_sampler,
								id <MTLTexture> m_output_texture, id<MTLBuffer> luminance_buffer, const UBOFrame &ubo_frame)
{
	id <MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder] ;
	compute_encoder.label = @"Compute Bright Pass";
	
	m_bright_pass->SetAsCompute(compute_encoder) ;
	
	HDRConsts hdr_consts ;
	hdr_consts.ABCD = ubo_frame.ABCD ;
	hdr_consts.EFW_tau = ubo_frame.EFW_tau ;
	hdr_consts.exposure_bloomthreshold_minmax_lum = ubo_frame.exposure_bloomthreshold_pad2 ;
	
	[compute_encoder setBytes:&hdr_consts length:sizeof(hdr_consts) atIndex:BRIGHT_PASS_HDRCONTS_BFR_SLOT] ;
	
	
	// Bind the input texture and the avg luminance
	[compute_encoder setTexture:m_input_texture atIndex:BRIGHT_PASS_INPUT_TEXTURE_SLOT] ;
	[compute_encoder setSamplerState:m_input_sampler atIndex:BRIGHT_PASS_INPUT_SAMPLER_SLOT] ;
	[compute_encoder setBuffer:luminance_buffer offset:0 atIndex:BRIGHT_PASS_LUMINANCE_BFR_SLOT] ;
	
	// Bind the output as image
	[compute_encoder setTexture:m_output_texture atIndex:BRIGHT_PASS_OUTPUT_TEXTURE_SLOT] ;
	
	// Execute the bright pass
	MTLSize threadsPerGroup = { m_work_group_size_x, m_work_group_size_y, 1 };
	MTLSize numThreadgroups = { m_dispatch_count_x, m_dispatch_count_y, 1};
	
	[compute_encoder dispatchThreadgroups:numThreadgroups
					threadsPerThreadgroup:threadsPerGroup] ;
	
	[compute_encoder endEncoding] ;
}



FragmentBrightPass::FragmentBrightPass()
{
	m_bright_pass_desc = [[MTLRenderPassDescriptor alloc] init];
	m_bright_pass_desc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
	m_bright_pass_desc.colorAttachments[0].storeAction = MTLStoreActionStore;
	
	m_quad_buffer = nullptr;
}

FragmentBrightPass::~FragmentBrightPass()
{
	releaseObj(m_bright_pass_desc);
	releaseObj(m_pipeline);
	
	delete m_quad_buffer;
}

void FragmentBrightPass::Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type)
{
	NGLOG_INFO("FragmentBrightPass - Init: %sx%s", width, height);
	
	m_quad_buffer = new MetalRender::QuadBuffer(MetalRender::QuadBuffer::kBlitQuadLandscape);
	
	KCL::KCL_Status error;
	MTLPipeLineBuilder sb;
	sb.SetVertexLayout(MetalRender::QuadBuffer::GetVertexLayout());
	m_pipeline = sb.ShaderFile("bright_pass40_fs.shader").Build(error);
}

void FragmentBrightPass::Execute(id <MTLCommandBuffer> command_buffer, id <MTLTexture> m_input_texture, id <MTLSamplerState> m_input_sampler,
								 id <MTLTexture> m_output_texture, id<MTLBuffer> luminance_buffer, const UBOFrame &ubo_frame)
{
	m_bright_pass_desc.colorAttachments[0].texture = m_output_texture;
	
	id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:m_bright_pass_desc];
	
	m_pipeline->Set(encoder);
	
	HDRConsts hdr_consts ;
	hdr_consts.ABCD = ubo_frame.ABCD ;
	hdr_consts.EFW_tau = ubo_frame.EFW_tau ;
	hdr_consts.exposure_bloomthreshold_minmax_lum = ubo_frame.exposure_bloomthreshold_pad2 ;
	
	[encoder setFragmentBytes:&hdr_consts length:sizeof(hdr_consts) atIndex:BRIGHT_PASS_HDRCONTS_BFR_SLOT] ;
	
	[encoder setFragmentTexture:m_input_texture atIndex:BRIGHT_PASS_INPUT_TEXTURE_SLOT];
	[encoder setFragmentSamplerState:m_input_sampler atIndex:BRIGHT_PASS_INPUT_SAMPLER_SLOT];
	[encoder setFragmentBuffer:luminance_buffer offset:0 atIndex:BRIGHT_PASS_LUMINANCE_BFR_SLOT] ;
	
	m_quad_buffer->Draw(encoder);
	
	[encoder endEncoding];
}



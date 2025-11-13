/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_compute_hdr31.h"

#include "metal/mtl_pipeline_builder.h"

typedef KCL::Vector4D hfloat4;

#include "bright_pass.h"
#include "hdr_consts.h"

const KCL::uint32 ComputeHDR31::BLUR_TEXTURE_COUNT ;

ComputeHDR31::ComputeHDR31(id <MTLDevice> device)
{
    m_device = device ;
    
	m_bright_pass_work_group_size = 8;    
    m_bright_pass_dispatch_count_x = 0;
    m_bright_pass_dispatch_count_y = 0;

    m_fragment_blur = NULL;
    m_reduction = NULL;

    m_input_texture = 0;    
    m_bright_texture_type = MTLPixelFormatInvalid;
}

ComputeHDR31::~ComputeHDR31()
{
    releaseObj(m_input_sampler) ;
    releaseObj(m_bloom_sampler) ;
    releaseObj(m_bright_texture) ;
    
    delete m_fragment_blur;
    delete m_reduction;
    
    releaseObj(m_device) ;
}

void ComputeHDR31::Init(KCL::uint32 width, KCL::uint32 height, MTLPixelFormat bloom_texture_gl_type, KRL_Scene* scene)
{
    m_width = width;
    m_height = height;

    m_bright_texture_type = bloom_texture_gl_type;
    m_bright_pass_dispatch_count_x = ((m_width / DOWNSAMPLE) + m_bright_pass_work_group_size - 1) / m_bright_pass_work_group_size;
    m_bright_pass_dispatch_count_y = ((m_height / DOWNSAMPLE) + m_bright_pass_work_group_size - 1) / m_bright_pass_work_group_size;
        
	KCL::uint32 blur_kernel_size = 9; // blur kernel size for FullHD
    
    KCL::uint32 m_dim = KCL::Max(m_width, m_height);
	blur_kernel_size = blur_kernel_size * m_dim / 1920; // resize blur kernel for actual resolution

    m_fragment_blur = new MetalRender::FragmentBlur(m_device);
    m_fragment_blur->Init(width / DOWNSAMPLE, height / DOWNSAMPLE, blur_kernel_size, bloom_texture_gl_type, 4);

    // Init reduction pass
    m_reduction = new MetalRender::ComputeReduction(m_device);
    m_reduction->Init(m_width, m_height, scene);
    
    // Init bright pass
	InitBrightPass();
}

void ComputeHDR31::SetInputTexture(id <MTLTexture> in_texture)
{
    m_input_texture = in_texture; 
    m_reduction->SetInputTexture(m_input_texture);
}

id <MTLTexture> ComputeHDR31::GetBloomTexture() const
{
    return m_fragment_blur->GetOutputTexture();
}

id <MTLSamplerState> ComputeHDR31::GetBloomSampler() const
{
    return m_bloom_sampler;
}

id <MTLBuffer> ComputeHDR31::GetLuminanceBuffer() const
{
    return m_reduction->GetLuminanceBuffer();
}

void ComputeHDR31::Execute(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame)
{    
    m_reduction->Execute(command_buffer, ubo_frame);
	BrightPass(command_buffer, ubo_frame);
}

void ComputeHDR31::InitBrightPass()
{
    CompileShader();
    SetupBrightTexture();
}

void ComputeHDR31::CompileShader()
{
	KCL::KCL_Status error;
	MTLPipeLineBuilder sb;
    sb.ShaderFile("bright_pass.shader") ;
	sb.AddDefineInt("WORK_GROUP_SIZE",m_bright_pass_work_group_size);
    sb.AddDefineVec2("STEP_UV", KCL::Vector2D(1.0f / m_width, 1.0f / m_height));
    sb.ForceHighp(true);
	m_bright_pass = sb.Build(error);
    m_bright_pass->IsThreadCountOk("bright_pass", m_bright_pass_work_group_size);
}

void ComputeHDR31::SetupBrightTexture()
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
    
    MTLTextureDescriptor *bright_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:m_bright_texture_type
                                                                                               width:width
                                                                                              height:height
                                                                                           mipmapped:true];
    bright_tex_desc.usage = MTLTextureUsageShaderWrite | MTLTextureUsageShaderRead;
    
    bright_tex_desc.mipmapLevelCount = BLUR_TEXTURE_COUNT ;
    
    m_bright_texture = [m_device newTextureWithDescriptor:bright_tex_desc];

    
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

void ComputeHDR31::BrightPass(id <MTLCommandBuffer> command_buffer, const UBOFrame &ubo_frame)
{
    id <MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder] ;

    m_bright_pass->SetAsCompute(compute_encoder) ;
    
    HDRConsts hdr_consts ;
    hdr_consts.ABCD = ubo_frame.ABCD ;
    hdr_consts.EFW_tau = ubo_frame.EFW_tau ;
    hdr_consts.exposure_bloomthreshold_minmax_lum = ubo_frame.exposure_bloomthreshold_pad2 ;
    
    [compute_encoder setBytes:&hdr_consts length:sizeof(hdr_consts) atIndex:BRIGHT_PASS_HDRCONTS_BFR_SLOT] ;
    
    
    // Bind the input texture and the avg luminance
    [compute_encoder setTexture:m_input_texture atIndex:BRIGHT_PASS_INPUT_TEXTURE_SLOT] ;
    [compute_encoder setSamplerState:m_input_sampler atIndex:BRIGHT_PASS_INPUT_SAMPLER_SLOT] ;
    [compute_encoder setBuffer:GetLuminanceBuffer() offset:0 atIndex:BRIGHT_PASS_LUMINANCE_BFR_SLOT] ;

    // Bind the output as image
    [compute_encoder setTexture:m_bright_texture atIndex:BRIGHT_PASS_OUTPUT_TEXTURE_SLOT] ;
    
    // Execute the bright pass   
    MTLSize threadsPerGroup = { m_bright_pass_work_group_size, m_bright_pass_work_group_size, 1 };
    MTLSize numThreadgroups = { m_bright_pass_dispatch_count_x, m_bright_pass_dispatch_count_y, 1};
    
    [compute_encoder dispatchThreadgroups:numThreadgroups
                    threadsPerThreadgroup:threadsPerGroup] ;
    
    [compute_encoder endEncoding] ;

    // Downsample the bright texture
    id<MTLBlitCommandEncoder> blitEncoder = [command_buffer blitCommandEncoder];
    [blitEncoder generateMipmapsForTexture:m_bright_texture];
    [blitEncoder endEncoding];

    // Blur the bright texture
    m_fragment_blur->SetInputTexture(m_bright_texture);
    m_fragment_blur->Execute(command_buffer);
}


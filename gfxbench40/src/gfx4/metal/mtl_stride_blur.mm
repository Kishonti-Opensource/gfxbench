/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "platform.h"
#include "mtl_stride_blur.h"
#include "metal/mtl_pipeline_builder.h"
#include "mtl_scene_40_support.h"

#include <ng/log.h>

#include <sstream>
#include <iomanip> // std::setprecision

using namespace MetalRender;

StrideBlur::StrideBlur()
{
	m_shader_h_pipeline = nullptr;
	m_shader_v_pipeline = nullptr;

    m_nearest_sampler = nil;
	m_linear_sampler = nil;

	m_temp_texture = nil;
	m_color_texture = nil;
}

StrideBlur::~StrideBlur()
{
	releaseObj(m_nearest_sampler);
	releaseObj(m_linear_sampler);
	releaseObj(m_temp_texture);
	releaseObj(m_color_texture);
}

void StrideBlur::Init(id<MTLDevice> device, KCL::uint32 width, KCL::uint32 height, KCL::uint32 stride, KCL::uint32 kernel_size)
{
	kernel_size = KCL::Max(kernel_size, 2u);

	NGLOG_INFO("Stride blur kernel: %s", kernel_size);

	MTLSamplerDescriptor * sampler_desc = [[MTLSamplerDescriptor alloc] init];

	sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.minFilter = MTLSamplerMinMagFilterNearest;
	sampler_desc.magFilter = MTLSamplerMinMagFilterNearest;
	sampler_desc.mipFilter = MTLSamplerMipFilterNotMipmapped;

	m_nearest_sampler = [device newSamplerStateWithDescriptor:sampler_desc];

	sampler_desc.sAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.tAddressMode = MTLSamplerAddressModeClampToEdge;
	sampler_desc.minFilter = MTLSamplerMinMagFilterLinear;
	sampler_desc.magFilter = MTLSamplerMinMagFilterLinear;
	sampler_desc.mipFilter = MTLSamplerMipFilterNotMipmapped;

	m_linear_sampler = [device newSamplerStateWithDescriptor:sampler_desc];

	m_temp_texture = GFXB4::CreateRenderTarget(device, width, height, MTLPixelFormatRGBA8Unorm);
	m_color_texture = GFXB4::CreateRenderTarget(device, width, height, MTLPixelFormatRG8Unorm);

	KCL::KCL_Status status;
	MTLPipeLineBuilder sb;

	sb.AddDefineInt("KS", kernel_size);
	sb.AddDefine("GAUSS_WEIGHTS " + GetGaussWeightsString(kernel_size));
	sb.AddDefine("OFFSETS " + GetOffsetsString(kernel_size, stride, width, false));
	sb.AddDefine("PASS_DEPTH");

	m_shader_h_pipeline =
	sb.ShaderFile("pp_stride_blur.shader")
	.SetVertexLayout(QuadBuffer::GetVertexLayout())
	.SetTypeByPixelFormat(MTLPixelFormatRGBA8Unorm)
	.Build(status);

	sb.AddDefineInt("KS", kernel_size);
	sb.AddDefine("GAUSS_WEIGHTS " + GetGaussWeightsString(kernel_size));
	sb.AddDefine("OFFSETS " + GetOffsetsString(kernel_size, stride, height, true));

	m_shader_v_pipeline =
	sb.ShaderFile("pp_stride_blur.shader")
	.SetVertexLayout(QuadBuffer::GetVertexLayout())
	.SetTypeByPixelFormat(MTLPixelFormatRG8Unorm)
	.Build(status);
}

void StrideBlur::Render(id<MTLCommandBuffer> command_buffer, MetalRender::QuadBuffer * quad_buffer, KCL::Camera2 * camera, id<MTLTexture> input_texture, id<MTLTexture> depth_texture)
{
	// Horizontal pass
	{
		MTLRenderPassDescriptor * desc = [[MTLRenderPassDescriptor alloc] init];

		desc.colorAttachments[0].texture = m_temp_texture;
		desc.colorAttachments[0].loadAction = MTLLoadActionClear;
		desc.colorAttachments[0].storeAction = MTLStoreActionStore;

		id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
		encoder.label = @"Stride Blur H";
		releaseObj(desc);

		m_shader_h_pipeline->Set(encoder);
		[encoder setFragmentTexture:input_texture atIndex:0];
		[encoder setFragmentSamplerState:m_linear_sampler atIndex:0];
		quad_buffer->Draw(encoder);

		[encoder endEncoding];
	}

	// Vertical pass
	{
		MTLRenderPassDescriptor * desc = [[MTLRenderPassDescriptor alloc] init];

		desc.colorAttachments[0].texture = m_color_texture;
		desc.colorAttachments[0].loadAction = MTLLoadActionClear;
		desc.colorAttachments[0].storeAction = MTLStoreActionStore;

		id <MTLRenderCommandEncoder> encoder = [command_buffer renderCommandEncoderWithDescriptor:desc];
		encoder.label = @"Stride Blur V";
		releaseObj(desc);

		m_shader_v_pipeline->Set(encoder);
		[encoder setFragmentTexture:m_temp_texture atIndex:0];
		[encoder setFragmentSamplerState:m_nearest_sampler atIndex:0];
		quad_buffer->Draw(encoder);

		[encoder endEncoding];
	}
}

std::vector<float> StrideBlur::GetGaussWeights(unsigned int kernel_size)
{
	std::vector<float> gauss_weights ;

	double sigma = (1.0*kernel_size)/3.0 ;

	float sum_weights = 1.0f;
	for (int i = -1*kernel_size; i <= (int)kernel_size ;++i)
    {
        if (i != 0)
        {
            float w = exp(-0.5*i*i / (sigma*sigma));
            gauss_weights.push_back(w);
            sum_weights += w;
        }
	}

	// Normalize weights
    /*
	for (KCL::uint32 i = 0; i < gauss_weights.size(); i++)
	{
		gauss_weights[i] /= sum_weights;
	}
    */

	return gauss_weights ;
}

std::string StrideBlur::GetGaussWeightsString(unsigned int kernel_size)
{
	std::stringstream sstream ;

	std::vector<float> gauss_weights = GetGaussWeights(kernel_size) ;
	std::vector<float>::iterator it = gauss_weights.begin() ;

	sstream<<std::fixed<<std::setprecision(10)<<*it ;
	it++ ;

	for(  ; it != gauss_weights.end() ; it++)
	{
		sstream<<", "<<*it ;
	}

	return sstream.str() ;
}

std::string StrideBlur::GetOffsetsString(KCL::uint32 kernel_size, float stride, KCL::uint32 width, bool y)
{
	std::stringstream sstream;

	sstream<<std::fixed<<std::setprecision(10) ;

	for (KCL::uint32 i = 0; i < kernel_size * 2 + 1; i++)
	{
        if (i == kernel_size)
        {
            continue;
        }

		float offset = (float(i) - float(kernel_size)) * stride / width;
		sstream << "hfloat2(";
		if (y)
		{
			sstream << "0.0, " << offset << ")";
		}
		else
		{
			sstream << offset << ", 0.0)";
		}

		if (i != kernel_size * 2)
		{
			sstream << ", ";
		}
	}
	return sstream.str();
}

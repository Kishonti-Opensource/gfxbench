/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef FRAGMENT_BLUR_H
#define FRAGMENT_BLUR_H

#include "kcl_base.h"
#include "mtl_pipeline.h"
#include "mtl_quadBuffer.h"


namespace MetalRender
{
class FragmentBlur
{
public:
    FragmentBlur(id <MTLDevice> device);
	virtual ~FragmentBlur();

    void Init(KCL::uint32 width, KCL::uint32 height, KCL::uint32 blur_kernel_size, MTLPixelFormat texture_format, KCL::uint32 lod_levels);
	void Execute(id <MTLCommandBuffer> commandBuffer);		
    
    void SetInputTexture( id <MTLTexture> in_tex);
    id <MTLTexture> GetOutputTexture() const;    

private:
	KCL::uint32 m_width, m_height;
    KCL::uint32 m_lod_levels;
    bool m_has_lod;

    MetalRender::Pipeline *m_blur_shader_h;
    MetalRender::Pipeline *m_blur_shader_v;
    
    id <MTLTexture> m_in_tex ;
    id <MTLTexture> m_temp_tex ;
    id <MTLTexture> m_out_tex ;
    
    id <MTLSamplerState> m_sampler ;
    
    std::vector<float> packed_weights;
    std::vector<float> vertical_packed_offsets;
    std::vector<float> horizontal_packed_offsets;
    std::vector<KCL::Vector2D> stepuv;

    MetalRender::QuadBuffer* m_quadBuffer;
    MTLVertexDescriptor* m_quadBufferVertexLayout;
    
    id <MTLDevice> m_device ;
};
}

#endif
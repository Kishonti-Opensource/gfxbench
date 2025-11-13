/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_scene_30_imp.h"


KRL_Scene* MetalRender::CreateMTLScene30(const GlobalTestEnvironment* const gte)
{
    return new MTL_Scene_30_Imp(gte);
}


MTL_Scene_30_Imp::MTL_Scene_30_Imp(const GlobalTestEnvironment* const gte) : MTL_Scene_30(gte)
{
    
}


MTL_Scene_30_Imp::~MTL_Scene_30_Imp()
{
    
}


void MTL_Scene_30_Imp::DoLightingPass(id <MTLRenderCommandEncoder> render_encoder)
{
    m_framebuffer->SetRenderedTexturesForLightPass(render_encoder);
    
    [render_encoder setDepthStencilState:m_lightDepthState];
    [render_encoder setCullMode:MTLCullModeFront];
    
    for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
    {
        KCL::Light* llight = m_visible_lights[i];
        
        RenderLight(llight, render_encoder,i != 0);
    }
}


void MTL_Scene_30_Imp::RunLightningEffectPass1(id <MTLCommandBuffer> commandBuffer)
{
    
}


void MTL_Scene_30_Imp::RunLightningEffectPass2(id <MTLCommandBuffer> commandBuffer)
{
    
}


void MTL_Scene_30_Imp::RenderLightningEffect(id <MTLRenderCommandEncoder> render_encoder)
{
    
}


void MTL_Scene_30_Imp::RunPostProcess(id<MTLCommandBuffer> commandBuffer)
{
    //
    //  Generate mipmaps for DoF effect
    //
    m_framebuffer->GenerateMipmapsForMainBuffer(commandBuffer);
    
    
    //
    //  Render Bloom Layers
    //
    id <MTLRenderCommandEncoder> renderEncoder =
    m_framebuffer->SetBloomBufferAsTargetAndGetEncoder(commandBuffer, 0, (MetalRender::Framebuffer::BloomDirection)0);
    
    [renderEncoder setDepthStencilState:m_blitDepthState];
    
    m_framebuffer->SetRenderedTexturesForSubFilter(renderEncoder);
    
    m_subFilter_pipeline->Set(renderEncoder);
    
    m_quadBuffer->Draw(renderEncoder);
    
    [renderEncoder endEncoding];
    
    for(uint32_t bloomLevel = 0; bloomLevel < m_framebuffer->GetNumBloomLevels(); bloomLevel++)
    {
        for(uint32_t bloomDir = 0; bloomDir < MetalRender::Framebuffer::NUM_BLOOM_DIRECTIONS; bloomDir++)
        {
            if(bloomLevel == 0 && bloomDir == 0)
            {
                continue;
            }
            
            renderEncoder =
            m_framebuffer->SetBloomBufferAsTargetAndGetEncoder(commandBuffer, bloomLevel,
                                                               (MetalRender::Framebuffer::BloomDirection)bloomDir);
            
            [renderEncoder setDepthStencilState:m_blitDepthState];
            
            float blurConst[2];
            if((MetalRender::Framebuffer::BloomDirection)bloomDir == MetalRender::Framebuffer::HORIZONTAL)
            {
                blurConst[0] = 1.0f/m_framebuffer->GetBloomBufferWidth(bloomLevel);
                blurConst[1] = 0.0;
            }
            else
            {
                blurConst[0] = 0.0;
                blurConst[1] = 1.0f/m_framebuffer->GetBloomBufferHeight(bloomLevel);
            }
            
            m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, COLOR_BLUR_OFFSET_INDEX, blurConst, sizeof(blurConst));
            
            m_blurFilter_pipeline->Set(renderEncoder);
            
            uint32_t sourceLevel = bloomLevel;
            MetalRender::Framebuffer::BloomDirection sourceDir =
            (bloomDir == MetalRender::Framebuffer::HORIZONTAL) ? MetalRender::Framebuffer::VERTICAL : MetalRender::Framebuffer::HORIZONTAL;
            
            if(bloomDir == 0)
            {
                sourceLevel--;
            }
            
            m_framebuffer->SetBloomBufferAsTexture(renderEncoder, sourceLevel, sourceDir, 0);
            
            m_quadBuffer->Draw(renderEncoder);
            
            [renderEncoder endEncoding];
        }
    }
}


void MTL_Scene_30_Imp::RenderPostProcess(id<MTLCommandBuffer> commandBuffer)
{
    id <MTLRenderCommandEncoder> renderEncoder = m_framebuffer->SetFinalBufferAsTargetAndGetEncoder(commandBuffer);
    
    WriteAndSetFrameConsts<true, true>(m_active_camera, renderEncoder);
    
    float depthOfFieldConstants[2] = { m_active_camera->GetFov(), m_camera_focus_distance };
    
    m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, DEPTH_OF_FIELD_CONST_INDEX, depthOfFieldConstants, sizeof(depthOfFieldConstants));
    
    m_framebuffer->SetRenderedTexturesForDepthOfFieldFilter(renderEncoder);
    
    [renderEncoder setDepthStencilState:m_blitDepthState];
    
    m_depthOfField_pipeline->Set(renderEncoder);
    
    m_finalBlitQuadBuffer->Draw(renderEncoder);
    
    [renderEncoder endEncoding];
}


KCL::KCL_Status MTL_Scene_30_Imp::reloadShaders()
{
    KCL::KCL_Status result = MTL_Scene_30::reloadShaders();
    
    if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
    
    std::set< std::string> defines;
    
    MetalRender::Pipeline::GFXPipelineDescriptor depthOfField_pipeline_desc(MetalRender::Pipeline::DISABLED,MetalRender::kShaderTypeSingleBGRA8,
                                                                            m_quadBufferVertexLayout, false, m_force_highp);
    
    m_depthOfField_pipeline = MetalRender::Pipeline::CreatePipeline("pp.vs", "pp.fs", &defines, depthOfField_pipeline_desc, result);

    if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
    
    return result;
}




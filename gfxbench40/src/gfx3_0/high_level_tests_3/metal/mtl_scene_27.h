/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_27_H
#define MTL_SCENE_27_H

#include "mtl_scene_base.h"
#include "fbo.h"
#include "mtl_quadBuffer.h"
#include "mtl_dynamic_data_buffer.h"
#include "mtl_framebuffer_27.h"


namespace MetalRender
{

class MTL_Scene_27 : public MTL_Scene_Base
{
public:
    MTL_Scene_27(const GlobalTestEnvironment* const gte) ;
    virtual ~MTL_Scene_27() ;
    
    void Render() ;
    
    KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
    
private:
    
    void RenderPlanar( KCL::PlanarMap* pm_, id<MTLCommandBuffer> commandBuffer) ;
    
    void RenderShadow( KRL_ShadowMap* sm, id<MTLCommandBuffer> commandBuffer) ;
    
    void RenderWithCamera(id <MTLRenderCommandEncoder> renderEncoder, KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type, MetalRender::ShaderType shader_type) ;
    
    void RenderPrepass(id <MTLRenderCommandEncoder> renderEncoder, KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, MetalRender::ShaderType shader_type) ;
    
    const GlobalTestEnvironment* const m_gte ;
    
     
    MetalRender::QuadBuffer* m_full_screen_quad ;
    MetalRender::DynamicDataBuffer* m_dynamicDataBuffer ;
    MetalRender::DynamicDataBufferPool* m_dynamicDataBufferPool ;
    
    MetalRender::DynamicDataBuffer*     m_pseudoIndicesBuffer ;
    MetalRender::DynamicDataBufferPool* m_pseudoIndicesBufferPool ;
    
    Pipeline* m_motionBlurPipeLine ;
    
    Framebuffer27* m_frameBuffer27 ;
    
    MetalGraphicsContext* m_Context ;
    
    id <MTLCommandQueue> m_CommandQueue ;
    
    MetalRender::ShaderType m_planar_map_shader_type ;
    MetalRender::ShaderType m_default_shader_type ;
    
    KCL::uint32 m_lock_id ;
    
    MTLVertexDescriptor* m_quadBufferVertexLayout;
};
    
}


#endif // MTL_SCENE_27_H

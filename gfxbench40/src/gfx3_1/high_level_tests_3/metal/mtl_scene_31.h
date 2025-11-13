/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_31_H
#define MTL_SCENE_31_H


#include "mtl_scene_30.h"
#include "mtl_compute_lightning.h"
#include "mtl_instancedlightrenderer.h"
#include "metal/mtl_fragment_blur.h"
#include "mtl_compute_hdr31.h"


class MTL_Scene_31 : public MTL_Scene_30
{
public:
    MTL_Scene_31(const GlobalTestEnvironment* const gte);
    virtual ~MTL_Scene_31();
    
    virtual KCL::KCL_Status Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples);
    virtual void Animate() ;
    virtual KCL::KCL_Status reloadShaders() ;
    
protected:
    
    virtual void DoLightingPass(id <MTLRenderCommandEncoder> render_encoder) ;
    
    virtual void RunPostProcess(id <MTLCommandBuffer> commandBuffer) ;
    virtual void RenderPostProcess(id <MTLCommandBuffer> commandBuffer) ;
    
    virtual void RunLightningEffectPass1(id <MTLCommandBuffer> commandBuffer) ;
    virtual void RunLightningEffectPass2(id <MTLCommandBuffer> commandBuffer) ;
    virtual void RenderLightningEffect(id <MTLRenderCommandEncoder> render_encoder) ;
    
    MTLComputeLightning *m_compute_lightning ;
    MTLInstancedLightRenderer *m_InstancedLightRenderer ;
    MetalRender::FragmentBlur *m_dof_blur;
    ComputeHDR31 *m_compute_hdr;
    
    MetalRender::Pipeline* m_hdr_pipeline ;
    
    id <MTLTexture> m_ldr_color_buffer ;
    
    KCL::int32 m_last_animation_time ;
    UBOFrame m_ubo_frame ;
};

#endif // MTL_SCENE_31_H


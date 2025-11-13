/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_30_IMP_H
#define MTL_SCENE_30_IMP_H

#include "mtl_scene_30.h"

class MTL_Scene_30_Imp : public MTL_Scene_30
{
public:
    MTL_Scene_30_Imp(const GlobalTestEnvironment* const gte);
    virtual ~MTL_Scene_30_Imp();
    
    virtual KCL::KCL_Status reloadShaders();
    
protected:
    
    virtual void DoLightingPass(id <MTLRenderCommandEncoder> render_encoder) ;
    
    virtual void RunPostProcess(id <MTLCommandBuffer> commandBuffer) ;
    virtual void RenderPostProcess(id <MTLCommandBuffer> commandBuffer) ;
    
    virtual void RunLightningEffectPass1(id <MTLCommandBuffer> commandBuffer) ;
    virtual void RunLightningEffectPass2(id <MTLCommandBuffer> commandBuffer) ;
    virtual void RenderLightningEffect(id <MTLRenderCommandEncoder> render_encoder) ;
};


#endif // MTL_SCENE_30_IMP_H


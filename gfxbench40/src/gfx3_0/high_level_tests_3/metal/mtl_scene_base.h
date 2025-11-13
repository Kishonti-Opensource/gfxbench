/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MTL_SCENE_BASE_H
#define MTL_SCENE_BASE_H


#include "krl_scene.h"

#include "mtl_texture.h"
#include "mtl_factories.h"
#include "mtl_material.h"
#include "mtl_pipeline.h"
#include "mtl_cubemap.h"
#include "mtl_light.h"
#include "mtl_particlesystem.h"

#include <set>

class MTL_Scene_Base : public KRL_Scene
{
public:
    MTL_Scene_Base() ;
    
    virtual void InitFactories();
    
    virtual KCL::Mesh3Factory &Mesh3Factory(){
        return m_mesh3_factory;
    };
    
    virtual ~MTL_Scene_Base();
     
    virtual void DeleteVBOPool() { ; }
    virtual void DeleteShaders() { ; }
    virtual void CreateVBOPool() { ; }
    
    virtual KCL::AnimatedEmitterFactory &EmitterFactory()
    {
        return m_emitter_factory ;
    }
    
    virtual KCL::LightFactory &LightFactory()
    {
        return m_light_factory ;
    }
    
    virtual KCL::KCL_Status reloadShaders();
	
	std::set<std::string> global_defines;

protected:
    
    virtual KCL::TextureFactory &TextureFactory() { return m_texture_factory ; }
    
    CubeEnvMap_Metal* CreateEnvMap(  const KCL::Vector3D &pos, KCL::uint32 idx, bool use_mipmaps);
    
    MetalRender::TextureFactory m_texture_factory ;
    MetalRender::Mesh3Factory m_mesh3_factory;
    std::set<MetalRender::ShaderType> m_requied_shader_types ;
    
    const std::string common_dir = "common/" ;
    
    MetalRender::MTLLightFactory m_light_factory ;
    MetalRender::MTLAnimatedEmitterFactory m_emitter_factory ;
    KCL::AnimatedEmitterFactory m_animated_factory ;
    MetalRender::MaterialFactory* m_material_factory;
};


#endif // MTL_SCENE_BASE_H

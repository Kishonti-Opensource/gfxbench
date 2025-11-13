/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import "mtl_scene_base.h"

#include "mtl_material.h"


MTL_Scene_Base::MTL_Scene_Base()
{
    m_requied_shader_types.clear() ;
}


void MTL_Scene_Base::InitFactories()
{
    m_factory.RegisterFactory(&m_animated_factory, KCL::EMITTER1) ;
    m_factory.RegisterFactory(&m_emitter_factory, KCL::EMITTER2) ;
    m_factory.RegisterFactory(&m_light_factory, KCL::LIGHT);
    
    m_material_factory = new MetalRender::MaterialFactory(m_scene_version);
    m_factory.RegisterFactory(m_material_factory, KCL::MATERIAL);
}


MTL_Scene_Base::~MTL_Scene_Base()
{
    delete m_material_factory;
    m_material_factory = nullptr;
}


KCL::KCL_Status MTL_Scene_Base::reloadShaders()
{
    KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;
    
    assert(m_max_joint_num_per_mesh < 255);
    uint32_t maxJointsPerMesh = static_cast<uint32_t>(m_max_joint_num_per_mesh);
    
    std::set< std::string> def;
    def.insert("DEP_TEXTURING");    /* force mediump */
    
    printf("Going to load %lu materials\n", m_materials.size());
    
    for(size_t i=0; i<m_materials.size(); ++i)
    {
        std::string s;
        
        if( m_shadow_method_str == "depth map(depth)")
        {
            s += "shadow_depth_map_depth ";
        }
        else if( m_shadow_method_str == "depth map(color)")
        {
            s += "shadow_depth_map_color ";
        }
        else if( m_shadow_method_str == "simple projective")
        {
            s += "shadow_simple_projective ";
        }
        if( m_soft_shadow_enabled)
        {
            s += "soft_shadow ";
        }
        s += GetVersionStr() + " ";
        
        MetalRender::Material* material = dynamic_cast<MetalRender::Material*>(m_materials[i]);
        result = material->InitShaders(s.c_str(), maxJointsPerMesh,m_requied_shader_types, m_force_highp);
        
        if(result != KCL::KCL_TESTERROR_NOERROR)
        {
            return result;
        }
    }
    
    if( m_mblur_enabled  && (m_scene_version == KCL::SV_27) )
    {
        m_blur_shader = Shader::CreateShader( "pp.vs", "mblur_final.fs", &def, result);
    }

    return result ;
}


CubeEnvMap_Metal* MTL_Scene_Base::CreateEnvMap( const KCL::Vector3D &pos, KCL::uint32 idx, bool use_mipmaps)
{
    std::string envmap_path = EnvmapsDirectory();
    CubeEnvMap_Metal *cubemap = CubeEnvMap_Metal::Load( idx, envmap_path.c_str(), use_mipmaps);
    
    // GLB has a method to generate the cubemap by rendering to a texture
    //  We shouldn't need this for manhattan though
    assert(cubemap);
    
    cubemap->SetPosition( pos);
    
    return cubemap;
}


/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_scene_opengl4.h"

#if defined HAVE_GLES3 || defined HAVE_GLEW

#include <cmath>
#include <cstddef>
#include <map>

#include <ng/log.h>
#include <ng/stringutil.h>

#include <kcl_light2.h>
#include <kcl_planarmap.h>
#include <kcl_room.h>
#include <kcl_actor.h>
#include <kcl_animation4.h>
#include <kcl_io.h>

#include "platform.h"
#include "misc2.h"
#include "../global_test_environment.h"

#include "glb_kcl_adapter.h"
#include "gfxb4_mesh.h"
#include "glb_material4.h"
#include "glb_particlesystem4.h"

#include "opengl/cubemap.h"
#include "opengl/shadowmap.h"
#include "opengl/misc2_opengl.h"
#include "opengl/fbo.h"
#include "opengl/vbopool.h"
#include "opengl/compute_reduction4.h"
#include "opengl/fragment_blur.h"
#include "opengl/glb_stride_blur.h"
#include "opengl/glb_compute_motion_blur.h"
#include "opengl/glb_cascaded_shadow_map.h"
#include "opengl/glb_occlusion_cull.h"
#include "opengl/glb_lensflare.h"
#include "opengl/glb_particlesystem.h"
#include "opengl/glb_opengl_state_manager.h"
#include "opengl/glb_image.h"
#include "opengl/glb_texture.h"
#include "opengl/glb_light.h"
#include "opengl/glb_shader2.h"
#include "opengl/glb_filter.h"
#include "opengl/glb_compute_hdr.h"
#include "opengl/glb_gbuffer.h"

using namespace GLB;

#define TESSELLATED_SHADOWS 1
#define DEBUG_SHADOW 0

#if ENABLE_QUICK_STATS
#include "opengl/gl_wrapper/gl_wrapper.h"
#endif

#include "property.h"
#include "gui_interface.h"

#include "dummy_gui.h"
//#undef HAVE_GUI_FOLDER
#ifdef HAVE_GUI_FOLDER
#include "gfxgui.h"
#endif

using namespace GFXB4;

KCL::KCL_Status GLB_Scene4::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
    KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;

    // Clear the second material argument. Car Chases uses it for override the material
    if (m_rooms.size())
    {
        for (size_t i = 0; i < m_rooms[0]->m_meshes.size(); i++)
        {
            KCL::Mesh *m = m_rooms[0]->m_meshes[i];
            m->m_materials[RENDER_MATERIAL_ID] = NULL;
        }
    }

    for (size_t i = 0; i < m_actors.size(); i++)
    {
        for (size_t j = 0; j < m_actors[i]->m_meshes.size(); j++)
        {
            KCL::Mesh *m = m_actors[i]->m_meshes[j];
            m->m_materials[RENDER_MATERIAL_ID] = NULL;
        }
    }    

    {
        KCL::AssetFile track_file("animations/cc_track");
        if (!track_file.GetLastError())
        {
            _key_node::Read( m_camera_cut_track, track_file);
            track_file.Close();
        }
        else
        {
            INFO("ERROR: Can not load cc_track!");
            return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
        }
    }

    //{
    //    KCL::AssetFile file(ImagesDirectory() + "topdown_ao_depth.png");
    //    m_topdown_shadow = TextureFactory().CreateAndSetup( KCL::Texture_2D, file.getFilename().c_str(), KCL::TC_NoMipmap);
    //    if (!m_topdown_shadow)
    //    {
    //        INFO("ERROR: Can not load topdown_ao_shadow_depth!");
    //    }
    //}

    PropertyLoader prr;
    std::string path = "car_chase/scene_4.prop";
    std::vector<SerializeEntry> entries = prr.DeSerialize(m_ubo_frame, path);
    for (auto entry = entries.begin(); entry != entries.end(); ++entry)
    {
        if (entry->m_key == "fogColorStrength")
        {
            m_fogColorStrength = ng::atof(entry->m_value.c_str());
            break;
        }
        if (entry->m_key == "SunColorStrength")
        {
            m_sunColorStrength = ng::atof(entry->m_value.c_str());
            break;
        }
    }

    m_ubo_manager = std::shared_ptr<UBOManager>(new UBOManager());
    m_light_color.x = m_ubo_frame.m_ubo.global_light_color.x;
    m_light_color.y = m_ubo_frame.m_ubo.global_light_color.y;
    m_light_color.z = m_ubo_frame.m_ubo.global_light_color.z;

    m_fogColor.x = m_ubo_frame.m_ubo.fogCol.x;
    m_fogColor.y = m_ubo_frame.m_ubo.fogCol.y;
    m_fogColor.z = m_ubo_frame.m_ubo.fogCol.z;

    // Screen-space scale factor for adaptive tesselation
    m_tessellation_viewport_scale = sqrtf(float(m_viewport_width) / 1920.0f * float(m_viewport_height) / 1080.0f);

    INFO("We have %d occluder(s)", m_occluders.size());
    if (m_occluders.empty())
    {
        INFO("!!! Warning !!! The scene does not contain any occluder meshes !!! !!! !!!");
        //assert(false);
    }   

    //find car actors
    for(int i=0; i<m_actors.size(); ++i)
    {
        if(m_actors[i]->m_name.find("car_evil") != std::string::npos)
        {
            m_carActor_evil = m_actors[i];
        }
        else if(m_actors[i]->m_name.find("car_hero") != std::string::npos)
        {
            m_carActor_hero = m_actors[i];
        }
    }

    // Global light direction comes from scene.xml so be sure it's normalized
    m_light_dir.normalize();

    m_instance_manager = new InstanceManager<GLB::Mesh3::InstanceData>();

    // Allocate space for 64 instances. This will be enough for the whole scene.
    m_instance_manager->PreallocateBuffers(64);

#ifdef GL_TEXTURE_CUBE_MAP_SEAMLESS
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

    GLBShader2::InitShaders(KCL::SV_40, m_gte->GetTestDescriptor()->m_force_highp);

    GLBShaderBuilder::AddGlobalDefine("MAX_INSTANCES", m_instance_manager->GetMaxInstanceCount());
    GLBShaderBuilder::AddGlobalDefine("VELOCITY_BUFFER_RGBA8", GBuffer::VELOCITY_BUFFER_RGBA8 ? 1 : 0);
    GLBShaderBuilder::AddGlobalDefine("NORMAL_BUFFER_RGBA8", GBuffer::NORMAL_BUFFER_RGBA8 ? 1 : 0);

#if ENABLE_CUBEMAP_FP_RENDER_TARGETS
    GLBShaderBuilder::AddGlobalDefine("CUBEMAP_FP_RENDERTARGET_ENABLED", 1);
#endif

#if ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET
    GLBShaderBuilder::AddGlobalDefine("LIGHTCOMBINE_FP_RENDERTARGET_ENABLED", 1);
#endif
    
#if DEBUG_SHADOW
    GLBShaderBuilder::AddGlobalDefine("DEBUG_SHADOW", 1);
#endif

#if HAVE_GUI_FOLDER
     //GLBShaderBuilder::AddGlobalDefine("EDITOR_MODE", 0);
#endif

    result = InitEmitters();
    if (result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result;
    }

    // Create the custom materials 
    m_sky_lightcombine_pass = dynamic_cast<Material4*>(CreateMaterial("gfxb4_sky_mat_light_combine"));   
    m_sky_mat_paraboloid = CreateMaterial( "gfxb4_sky_paraboloid_mat");
    m_sky_mat_paraboloid->m_material_type = KCL::Material::SKY;

    // Shadow caster materials (solid and transparent)
    m_shadow_material = dynamic_cast<Material4*>(CreateMaterial("shadow_caster_solid_mat"));
    m_transparent_shadow_material = dynamic_cast<Material4*>(CreateMaterial("shadow_caster_transparent_mat"));
    m_transparent_billboard_shadow_material = dynamic_cast<Material4*>(CreateMaterial("shadow_caster_transparent_billboard_mat"));
#if TESSELLATED_SHADOWS
    m_tessellated_shadow_material = dynamic_cast<Material4*>(CreateMaterial("shadow_caster_tessellated_mat"));
#else
    m_tessellated_shadow_material = m_shadow_material;
#endif

    // Create the dynamic envmaps
    const KCL::uint32 envmap_size = m_viewport_width / 2;

    m_fboEnvMap = new FboEnvMap( envmap_size);

#if ENABLE_CUBEMAP_FP_RENDER_TARGETS
    m_dynamic_cubemaps[0] = m_fboEnvMap->CreateParaboloidEnvMapWithFormat(GLB_Scene4Tools::GetSupportedFPRT()); //allow mipmapping
    m_dynamic_cubemaps[1] = m_fboEnvMap->CreateParaboloidEnvMapWithFormat(GLB_Scene4Tools::GetSupportedFPRT());
#else
    m_dynamic_cubemaps[0] = m_fboEnvMap->CreateParaboloidEnvMapWithFormat(GL_RGBA8); //allow mipmapping
    m_dynamic_cubemaps[1] = m_fboEnvMap->CreateParaboloidEnvMapWithFormat(GL_RGBA8); //allow mipmapping
#endif
    m_paraboloid_culling = new ParaboloidCulling(envmap_size, envmap_size, 128.0f);

    // Init G-buffers
    m_gbuffer = new GFXB4::GBuffer();
    m_gbuffer->Init(m_viewport_width, m_viewport_height);

    {
        Vector4D v[4] = 
        {
            Vector4D( -1, -1, 0, 0),
            Vector4D( 1, -1, 1, 0),
            Vector4D( -1, 1, 0, 1),
            Vector4D( 1, 1, 1, 1)
        };

        glGenBuffers( 1, &m_fullscreen_quad_vbo);
        glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
        glBufferData_chunked( GL_ARRAY_BUFFER, 4 * 4 * sizeof(float), v[0].v, GL_STATIC_DRAW);
        glBindBuffer( GL_ARRAY_BUFFER, 0);

        glGenVertexArrays( 1, &m_fullscreen_quad_vao);

        glBindVertexArray( m_fullscreen_quad_vao);
        glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
        glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, sizeof( Vector4D), 0);
        glEnableVertexAttribArray( 0);

        glBindVertexArray( 0);
        glBindBuffer( GL_ARRAY_BUFFER, 0);
    }
    InitPostProcessPipeline();

    IncrementProgressTo(0.5f);

    /*
    for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
    {
        m_global_shadowmaps[i] = ShadowMap::Create(m_fboShadowMap_size, m_shadow_method_str, m_fullscreen_quad_vbo, true, true);
    }
    */

    {
        IncrementProgressTo(0.59f);
        for( uint32 i=0; i<m_rooms.size(); i++)
        {
            XRoom *room = m_rooms[i];
            for( uint32 j=0; j<room->m_meshes.size(); j++)
            {
                room->m_meshes[j]->DeleteUnusedAttribs();
            }
        }
    }

    result = KCL::g_os->LoadingCallback(0);
    if (result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result;
    }


    IncrementProgressTo(0.6f);

    for( uint32 i=0; i<m_sky_mesh.size(); i++)
    {
        m_sky_mesh[i]->DeleteUnusedAttribs();
    }

    for(size_t i = 0; i < m_meshes.size(); ++i)
    {
        GFXB4::Mesh3* glb_mesh3 = dynamic_cast<GFXB4::Mesh3*>(m_meshes[i]);
        glb_mesh3->InitVertexAttribs();
        glb_mesh3->InitVAO4() ;

#ifndef HAVE_GUI_FOLDER
        glb_mesh3->DeleteVertexAttribs();
#endif
    }

    result = KCL::g_os->LoadingCallback(0);
    if (result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result;
    }

    IncrementProgressTo(0.7f);
    INFO("Loading shaders...");
    result = reloadShaders();
    if(result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result;
    }

    result = KCL::g_os->LoadingCallback(0);
    if (result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result;
    }

    for(size_t i = 0; i < m_materials.size(); ++i)
    {
        m_materials[i]->InitImages();
        if(result != KCL::KCL_TESTERROR_NOERROR)
        {
            return result;
        }

        result = KCL::g_os->LoadingCallback(0);
        if (result != KCL::KCL_TESTERROR_NOERROR)
        {
            return result;
        }
    }

    //topdown shadow sampler
    {
        for (size_t i = 0; i < m_textures.size(); ++i)
        {
            if (m_textures[i]->getName().find("topdown_ao_depth") != std::string::npos)
            {
                m_topdown_shadow = m_textures[i];

                glGenSamplers(1, &m_topdown_shadow_sampler);
                glSamplerParameteri(m_topdown_shadow_sampler, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glSamplerParameteri(m_topdown_shadow_sampler, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glSamplerParameteri(m_topdown_shadow_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glSamplerParameteri(m_topdown_shadow_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glSamplerParameteri(m_topdown_shadow_sampler, GL_TEXTURE_MIN_LOD, 0);
                glSamplerParameteri(m_topdown_shadow_sampler, GL_TEXTURE_MAX_LOD, 0);
                break;
            }
        }

        if (!m_topdown_shadow)
        {
            INFO("Error! Can not load topdown_ao_depth");
            return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
        }
    }

    IncrementProgressTo(0.9f);

    /*
    GLB::GLBTextureFactory f;
    m_logo = (GLBTexture*)f.CreateAndSetup(KCL::Texture_2D, "common/kishonti_logo_512.png", KCL::TC_Clamp | KCL::TC_NoMipmap);
    if (!m_logo)
    {
        INFO("Error! Can not load kishonti_logo_512.png");
    }
    */
    
    if(m_logo)
    {
        GLBShaderBuilder sb;
        m_logo_shader = sb.ShaderFile("logo.shader").Build(result);

        float v[] =
        {
            0, 0, 0, 0,
            (float)m_logo->getWidth(), 0, 1, 0,
            0, (float)m_logo->getHeight(), 0, 1,
            (float)m_logo->getWidth(), (float)m_logo->getHeight(), 1, 1
        };

        glGenBuffers( 1, &m_logo_vbo);

        glBindBuffer( GL_ARRAY_BUFFER, m_logo_vbo);
        glBufferData( GL_ARRAY_BUFFER, sizeof(float) * 16, v, GL_STATIC_DRAW);
        glBindBuffer( GL_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &m_logo_vao);

        glBindVertexArray( m_logo_vao);

        glBindBuffer( GL_ARRAY_BUFFER, m_logo_vbo);

        glVertexAttribPointer( m_logo_shader->m_attrib_locations[GLB::attribs::in_position], 3, GL_FLOAT, GL_FALSE, sizeof(float)*4, 0);
        glVertexAttribPointer(m_logo_shader->m_attrib_locations[GLB::attribs::in_texcoord0], 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(sizeof(float) * 2));

        glEnableVertexAttribArray(m_logo_shader->m_attrib_locations[GLB::attribs::in_position]);
        glEnableVertexAttribArray(m_logo_shader->m_attrib_locations[GLB::attribs::in_texcoord0]);

        glBindVertexArray( 0);
        glBindBuffer( GL_ARRAY_BUFFER, 0);
    }

    /*
    // Static envmap capture
    {
        Animate();
        for( uint32 i = 0; i < m_envmap_descriptors.size(); i++)
        {
            CaptureEnvmap( m_envmap_descriptors[i].m_position, i);
        }
    }
    */

    {
        INFO("Loading cube maps...");
        std::string filename = EnvmapsDirectory() + "envmap";
        m_static_cubemaps = TextureFactory().CreateAndSetup(KCL::Texture_CubeArray, filename.c_str(), KCL::TC_Clamp);
        if (m_static_cubemaps)
        {
            // Set the default filtering
            glBindTexture( GL_TEXTURE_CUBE_MAP_ARRAY, m_static_cubemaps->textureObject());            
            glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri( GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
            glBindTexture( GL_TEXTURE_CUBE_MAP_ARRAY, 0);
        }
        else
        {
            INFO("Error! Can not load static cube maps: %s", filename.c_str());
        }
    }

    m_prev_camera_clip = -1;

#ifdef HAVE_GUI_FOLDER
    m_gui = std::unique_ptr<GFXGui>(new GFXGui(*this));
#else
    m_gui = std::unique_ptr<DummyGUI>(new DummyGUI());
#endif

    m_gui->Init();

    // Execute warm up process
    NGLOG_INFO("Warm up compute shaders...");
    double warmup_start = KCL::g_os->GetTimeMilliSec();
    m_warmup_helper = new WarmUpHelper(this, m_gte->GetTestDescriptor()->m_workgroup_sizes);
    WarmUpShaders();
    NGLOG_INFO("warm up time: %s", int(KCL::g_os->GetTimeMilliSec() - warmup_start));

    m_last_animation_time = -1;

    return KCL::KCL_TESTERROR_NOERROR;
}

void GLB_Scene4::InitPostProcessPipeline()
{
    filters[HDR_FINAL_PASS].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, m_gbuffer->m_depth_texture, m_viewport_width, m_viewport_height, false , 1, 0, GL_RGB8);
    filters[HDR_FINAL_PASS].m_scene = this;
        
    filters[DOF].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, 0, m_viewport_width, m_viewport_height, true, 1, 0, GL_RGB8) ;
    filters[DOF].m_scene = this;

    filters[MOTION_BLUR].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, 0, m_viewport_width, m_viewport_height, !m_depth_of_field_enabled, 1, 0, GL_RGB8);
    filters[MOTION_BLUR].m_scene = this;

    filters[SSAO].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, 0, m_viewport_width/2, m_viewport_height/2, false, 1, 0, GL_R8) ; //1/2 res SSAO target
    filters[SSAO].m_scene = this;
    filters[SSAO].SetClearColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 1.0f));

    filters[SSDS].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, 0, m_viewport_width, m_viewport_height, false, 1, 0, GL_RGBA8);
    filters[SSDS].m_scene = this;
    filters[SSDS].m_reconstructPosInWS = true;

#if ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET
    filters[LIGHT_COMBINE].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, pp.m_depth_texture, m_viewport_width, m_viewport_height, false, 1, 0, GLB_Scene4Tools::GetSupportedFPRT());
    //NOTE: use the commented line when capturing static envprobes and ambient colors
    //filters[LIGHT_COMBINE].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, pp.m_depth_texture, m_viewport_width, m_viewport_height, false, 1000, 0, GLB_Scene4Tools::GetSupportedFPRT());
#else
    filters[LIGHT_COMBINE].Init(m_fullscreen_quad_vao,m_fullscreen_quad_vbo, m_gbuffer->m_depth_texture, m_viewport_width, m_viewport_height, false, 1, 0, GL_RGBA8);
#endif
    filters[LIGHT_COMBINE].m_reconstructPosInWS = true;
    filters[LIGHT_COMBINE].m_scene = this;
        
    LoadHDR();
}

void GLB_Scene4::LoadHDR()
{
    KCL::KCL_Status error = KCL::KCL_TESTERROR_NOERROR;

    // depth of field blur strength for full HD
    int dof_blur_strength = 7;
    dof_blur_strength = (dof_blur_strength*m_viewport_height) / 1080; // normalize for actual resolution

    delete m_dof_blur;
    m_dof_blur = new FragmentBlur();
    m_dof_blur->Init(m_fullscreen_quad_vao, m_fullscreen_quad_vbo, m_viewport_width, m_viewport_height, dof_blur_strength, GL_RGBA8, 1, this);

    GLB::GLBShaderBuilder sb;
    sb.FileFs("hdr_luminance.h").FileFs("hdr_common.h").ShaderFile("pp_hdr.shader");
    if (m_framebuffer_gamma_correction)
    {
        sb.AddDefine("USE_SHADER_GAMMA_CORRECTION") ;
    }
    filters[HDR_FINAL_PASS].m_shader = sb.Build(error);

    if (m_gbuffer->HIZ_DEPTH_ENABLED)
    {
        sb.AddDefine("USE_HIZ_DEPTH");
        sb.AddDefineInt("MAX_MIP_LEVEL", m_gbuffer->m_hiz_depth_levels - 1);
    }
    filters[SSAO].m_shader = sb.ShaderFile("pp_ssao.shader").Build(error);

    filters[DOF].m_shader = sb.ShaderFile("pp_dof.shader").Build(error);
    filters[SSDS].m_shader = sb.ShaderFile("pp_ssds.shader").Build(error);
    filters[LIGHT_COMBINE].m_shader = sb.ShaderFile("light_combine.shader").Build(error);
    
    const KCL::uint32 shadow_size = m_viewport_width / 2;
    delete m_cascaded_shadow_map;
    m_cascaded_shadow_map = new CascadedShadowMap(shadow_size);
    m_cascaded_shadow_map->Init(m_viewport_width, m_viewport_height, GL_DEPTH_COMPONENT16);

    // Load luminance and bright pass
    delete m_compute_hdr;
    m_compute_hdr = new ComputeHDR();
    m_compute_hdr->Init(m_viewport_width, m_viewport_height, GL_RGBA8, m_compute_bright_pass_enabled, m_fullscreen_quad_vao, m_fullscreen_quad_vbo, this);

    delete m_lensflare ;
    m_lensflare = new Lensflare();
    m_lensflare->Init(m_viewport_width,m_viewport_height);
    
    int ao_shadow_blur_strength = 3;
    ao_shadow_blur_strength = (ao_shadow_blur_strength * m_viewport_height) / 1080; // normalize for actual resolution
    delete m_ao_shadow_blur;
    m_ao_shadow_blur = new StrideBlur();
    
    m_ao_shadow_blur->Init(m_viewport_width, m_viewport_height, 3, ao_shadow_blur_strength); 

    WarmUpShaders();
}

void GLB_Scene4::WarmUpShaders()
{
    m_is_compute_warmup = true;

    OcclusionCull *new_occlusion_cull = new OcclusionCull();
    new_occlusion_cull->Init(m_warmup_helper, this, m_fullscreen_quad_vao, m_viewport_width / 2, m_viewport_height / 2, GL_DEPTH_COMPONENT24, m_instance_manager->GetMaxInstanceCount(), m_cull_non_instanced_meshes_enabled, false);
    delete m_occlusion_cull;
    m_occlusion_cull = new_occlusion_cull;

    ComputeMotionBlur *new_mb = new ComputeMotionBlur(m_fullscreen_quad_vao, m_fullscreen_quad_vbo, m_viewport_width, m_viewport_height, 8, ComputeMotionBlur::Adaptive);
    new_mb->Init(m_warmup_helper);
    delete m_compute_motion_blur;
    m_compute_motion_blur = new_mb;

    filters[MOTION_BLUR].m_shader = m_compute_motion_blur->GetBlurShader();

    // warmup particle system with a specific emitter
    m_emitters[0]->Init(m_warmup_helper);

    // apply the warmup workgroup size for all emitter
    for (int i = 0; i < NUM_OF_EMITTERS; i++)
    {
        m_emitters[i]->Init(m_warmup_helper);
    }

    m_is_compute_warmup = false;
}

KCL::KCL_Status GLB_Scene4::reloadShaders()
{
    GLBShader2::InvalidateShaderCache();

    GLB::GLBShaderBuilder sb;
    KCL::KCL_Status error;


    sb.AddDefine( "USE_GEOMSHADER");
    m_billboard_point_gs = sb.ShaderFile( "billboard_point_gs.shader").Build(error);

    // Load material shaders
    for (KCL::uint32 i = 0; i < m_materials.size(); i++)
    {
        error = ((GLB::Material4*)m_materials[i])->InitShaders(this);
        if (error != KCL::KCL_TESTERROR_NOERROR)
        {
            INFO("Can not init material: %s", m_materials[i]->m_name.c_str());
            return error;
        }

        error = KCL::g_os->LoadingCallback(0);
        if (error != KCL::KCL_TESTERROR_NOERROR)
        {
            return error;
        }
    }
    return KCL::KCL_TESTERROR_NOERROR;
}

void GLB_Scene4::ReloadHDR()
{
    GLBShader2::InvalidateShaderCache();
    m_gbuffer->InitShaders();
    LoadHDR();
}

void GLB_Scene4::ReloadExportedMeshes(const std::vector<std::string> & filenames)
{
    std::vector<KCL::Mesh3*> updated_mesh_list;
    KRL_Scene::ReloadExportedMeshes(filenames, updated_mesh_list);
}

void GLB_Scene4::ReloadExportedMaterials(const std::vector<std::string> & filenames)
{
    if (!filenames.empty())
    {
        GLBShader2::InvalidateShaderCache();
    }
    GLB_Scene_ES2_::ReloadExportedMaterials(filenames);
}

void GLB_Scene4::SetWireframeRenderEnabled(bool value)
{
    GLB_Scene_ES2_::SetWireframeRenderEnabled(value);
    if (m_wireframe_mode == WireframeGS)
    {
        if (m_wireframe_render)
        {
            GLBShaderBuilder::AddGlobalDefine("DRAW_GS_WIREFRAME", 1);
        }
        else
        {
            GLBShaderBuilder::RemoveGlobalDefine("DRAW_GS_WIREFRAME");
        }
#ifdef _DEBUG
        reloadShaders();
        ReloadHDR();
#endif
    }
}

GLB_Scene4::GLB_Scene4(const GlobalTestEnvironment *gte) : m_fboEnvMap(0)
{
    m_gte = gte;
    m_adaptation_mode = m_gte->GetTestDescriptor()->m_adaptation_mode;

    m_gbuffer = NULL;

    m_is_main_pass = false;

    m_wireframe_mode = WireframeGS;
    cubemaps_inited = false;
    m_occlusion_cull_enabled = true;
    m_cull_non_instanced_meshes_enabled = false;
    m_render_occluders_enabled = false;
    m_compute_bright_pass_enabled = false;
    m_framebuffer_gamma_correction = true;
    m_is_compute_warmup = false;

    m_sunColorStrength = 1.0f;
    m_fogColorStrength = 1.0f;
    m_last_animation_time = -1;

    m_fullscreen_quad_vbo = 0;
    m_fullscreen_quad_vao = 0;

    m_dynamic_cubemaps[0] = NULL;
    m_dynamic_cubemaps[1] = NULL;
    m_paraboloid_culling = NULL;
    
    m_logo = NULL;
    m_logo_shader = NULL;

    m_logo_vbo = 0;
    m_logo_vao = 0;
    
    m_instance_manager = NULL;
    m_occlusion_cull = NULL;

    m_compute_hdr = NULL;
    m_dof_blur = NULL;
    m_ao_shadow_blur = NULL;
    m_compute_motion_blur = NULL;
    m_cascaded_shadow_map = NULL;
    m_shadow_material = NULL;
    m_transparent_shadow_material = NULL;
    m_transparent_billboard_shadow_material = NULL;
    m_tessellated_shadow_material = NULL;
    m_lensflare = NULL;

    m_carActor_hero = NULL;
    m_carActor_evil = NULL;

    m_emitter_rate_anim[0] = NULL;
    m_emitter_rate_anim[1] = NULL;

    m_camera_cut_track = NULL;
    m_prev_camera_clip = -1;
    m_camera_clip_changed = false;
    m_prev_mvp_valid = false;

    m_topdown_shadow = NULL;
    m_static_cubemaps = NULL;

    for( int i = 0; i< NUM_OF_EMITTERS; i++)
    {
        m_emitters[i] = 0;
    }
    m_test_emitter = 0;
    m_warmup_helper = NULL;

    m_tessellation_viewport_scale = 0.0f;
    
    GLB::ComputeEmitter::ResetParticleSystemSeed();
}


GLB_Scene4::~GLB_Scene4()
{
    delete m_gbuffer;
      
    for(size_t i=0; i<m_actors.size(); i++)
    {
        Actor* a = m_actors[i];
        for(size_t j=0; j<a->m_lights.size(); j++)
        {
            GLB::Light* l = (GLB::Light*)a->m_lights[j];

            if (l->m_has_lensflare)
            {
                glDeleteQueries(4, l->m_query_objects);
            }
        }
    }

    glDeleteVertexArrays( 1, &m_fullscreen_quad_vao);
    glDeleteBuffers( 1, &m_fullscreen_quad_vbo);

    glDeleteVertexArrays( 1, &m_logo_vao);
    glDeleteBuffers( 1, &m_logo_vbo);
        
    for (int i = 0; i < 4; i++)
    {
        delete m_emitters[i] ;
        m_emitters[i] = 0 ;
    }

    delete m_static_cubemaps;

    //delete m_topdown_shadow; //note: deleted by scene_handler
    glDeleteSamplers(1, &m_topdown_shadow_sampler);
    delete m_logo;
    
    delete m_fboEnvMap;
    delete m_dynamic_cubemaps[0];
    delete m_dynamic_cubemaps[1];
    delete m_paraboloid_culling;
    
    delete m_instance_manager;

    GLBShader2::DeleteShaders();

    delete m_occlusion_cull;
    delete m_compute_hdr;
    delete m_dof_blur;
    delete m_ao_shadow_blur;
    delete m_compute_motion_blur;
    delete m_cascaded_shadow_map;
    delete m_lensflare;

    delete m_emitter_rate_anim[0];
    delete m_emitter_rate_anim[1];

    delete m_camera_cut_track;

    delete m_warmup_helper;

    Profiler *profiler = Profiler::GetInstance();
    if (profiler)
    {
        profiler->PrintResults(true);
    }
    ProfilerRelease();

    //psys clears itself
}

void GLB_Scene4::RenderShadow( ShadowMap* sm)
{
    sm->Bind();
    sm->Clear();
    
    glViewport( 1, 1, sm->Size() - 2, sm->Size() - 2);

    std::vector<Mesh*> visible_meshes[2];

    for( uint32 j=0; j<2; j++)
    {
        for( uint32 k=0; k<sm->m_caster_meshes[j].size(); k++)
        {
            visible_meshes[j].push_back( sm->m_caster_meshes[j][k]);
        }
    }

    Render(&sm->m_camera, visible_meshes[0], m_shadowCasterMaterial, 0, 1, 0, PassType::NORMAL, false);
    Render(&sm->m_camera, visible_meshes[1], m_shadowCasterMaterial, 0, 1, 0, PassType::NORMAL, false);

#if 0
    m_shadowCasterMaterial->preInit( 0);

    OpenGLStateManager::GlEnableVertexAttribArray( m_shadowCasterMaterial->m_shaders[0]->m_attrib_locations[GLB::attribs::in_position]);

    glUniformMatrix4fv( m_shadowCasterMaterial->m_shaders[0]->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, sm->m_camera.GetViewProjection().v);

    glLineWidth( 100);


    DrawAABB( dab, m_shadowCasterMaterial->m_shaders[0]->m_attrib_locations[GLB::attribs::in_position]);
    glLineWidth( 4);
    OpenGLStateManager::GlDisableVertexAttribArray( m_shadowCasterMaterial->m_shaders[0]->m_attrib_locations[GLB::attribs::in_position]);

    m_shadowCasterMaterial->postInit();
#endif

    glViewport( 0, 0, m_viewport_width, m_viewport_height);

    m_shadowStaticReceiverMeshes = sm->m_receiver_meshes[0];

    sm->Unbind();
}

void GLB_Scene4::RenderCascadedShadow()
{
    ProfilerBeginPass("Shadow");
    static std::vector<Mesh*> visible_meshes[3];
    static std::vector<KCL::MeshInstanceOwner2*> visible_mios;
    static std::vector<GLB::Mesh3::InstanceData> instance_data;
    static std::vector<KCL::Mesh*> instances;

    KCL::Camera2 *shadow_camera;

    m_cascaded_shadow_map->BuildFrustums(m_light_dir, m_active_camera);

    for (int i = 0; i < CASCADE_COUNT;  i++)
    {
        //ProfilerBeginPass("Shadow", i);
        visible_meshes[0].clear();
        visible_meshes[1].clear();
        visible_mios.clear();

        m_cascaded_shadow_map->FrustumCull(i, this, visible_meshes, visible_mios, m_force_cast_shadows);

        shadow_camera = m_cascaded_shadow_map->GetCamera(i);
        m_cascaded_shadow_map->Bind(i);

        CollectRenderMaterials(visible_meshes[0], m_shadow_material, PassType::SHADOW, i);
        CollectRenderMaterials(visible_meshes[1], m_transparent_shadow_material, PassType::SHADOW, i);

        GLB_Scene4Tools::SortMeshes(visible_meshes[0], shadow_camera, false);

        Render(shadow_camera, visible_meshes[0], m_shadow_material , 0, 0, 0, PassType::SHADOW, false, i);
        Render(shadow_camera, visible_meshes[1], m_transparent_shadow_material, 0, 0, 0, PassType::SHADOW, false, i);

        {            
            instance_data.clear();

            CollectRenderMaterials(visible_mios, m_shadow_material, PassType::SHADOW, i);

            CollectInstances( instance_data, instances, visible_mios);

            m_instance_manager->UploadInstanceData( instance_data);

            Render(shadow_camera, instances, m_shadow_material, 0, 0, 0, PassType::SHADOW, false, i);
        }
        // ProfilerEndPass();
    }

    m_cascaded_shadow_map->Unbind();

    ProfilerEndPass();
}

void GLB_Scene4::Render( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, PassType::Enum pass_type, bool occlusion_cull, KCL::uint32 slice_id)
{
    if (visible_meshes.empty())
    {
        return;
    }
           
    bool is_instanced_draw = visible_meshes[0]->m_mio2 != NULL;
    if (occlusion_cull)
    { 
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_occlusion_cull->GetIndirectCommandBuffer());
    }  

    KCL::uint32 current_vao = 0;
    GLB::Material4 *override_material = dynamic_cast<GLB::Material4*>(_override_material);
    GLB::Material4 *prev_material = NULL;
    int prev_shader_variant = -1;
    KCL::uint32 texture_num_from_material = 0;
    KCL::int32 shadow_texture_binding_point = -1;

    GLenum primitive_type = 0;

    /*
    if (!is_instanced_draw)
    {
        GLB_Scene4Tools::SortMeshes(visible_meshes, camera);
    }
    */

    KCL::Texture* ovrds[KCL::Material::MAX_IMAGE_TYPE] = {};
    if (pass_type == PassType::REFLECTION)
    {
        ovrds[KCL::Material::AUX2] = m_topdown_shadow;
    }
    else
    {
        //clear all overrides
        ovrds[KCL::Material::AUX2] = 0;
    }
    GLB::Material4::SetTextureOverrides(ovrds);

    // Calculate FOV scale for adaptive tessellation
    float tessellation_fov_scale;
    {
        float aperture = (float)tanf( KCL::Math::Rad( camera->GetFov() / 2.0f)) * camera->GetNear();
        float left0 = -aperture * camera->GetAspectRatio();
        float right0 = aperture * camera->GetAspectRatio();
        float top0 = aperture;
        float bottom0 = -aperture;
                
        float w = right0 - left0;
        float h = top0 - bottom0;

        //values for 60 deg FOV (freecam) with 0.1m near plane dist
        float fov60_w = 0.205280095f;
        float fov60_h = 0.115470052f;

        tessellation_fov_scale = sqrtf(fov60_w / w  *  fov60_h / h);
    }
        
    for( uint32 j=0; j<visible_meshes.size(); j++)
    {
        Mesh* sm = (Mesh*)visible_meshes[j];
        GLB::Mesh3 *glb_mesh  = (GLB::Mesh3 *)sm->m_mesh;

        if( !glb_mesh)
        {
            continue;
        }

        //use LOD for reflection pass
        KCL::Mesh3* orig_selected_mesh = sm->m_mesh;
        if((pass_type == PassType::REFLECTION) && (sm->m_mesh_variants[1]))
        {
            sm->m_mesh = sm->m_mesh_variants[1];
        }

//#define FORCE_LODS_FOR_TESTING
#ifdef FORCE_LODS_FOR_TESTING
        //NOTE: - currently render passes with occlusion cull can only handle lod0 meshes, as they upload primitive count during init
        //      - but even if the main pass cannot use lods, refl pass still can (no occl. cull there)
        m_occlusion_cull_enabled = false;
        if(sm->m_mesh_variants[1])
        {
            sm->m_mesh = sm->m_mesh_variants[1];
        }
#endif

        //GLB::Material4 *material = (GLB::Material4*)sm->m_material;
        GLB::Material4 *material = (GLB::Material4*)sm->m_materials[RENDER_MATERIAL_ID];
        if (!material)
        {
            material = (GLB::Material4*)sm->m_material;
        }
        
        /*
        if( override_material)
        {
            if((pass_type == PassType::SHADOW) && (material->m_is_billboard))
            {
                m_transparent_billboard_shadow_material->m_textures[0] = material->m_textures[0];
                material = m_transparent_billboard_shadow_material;
            }
            // Do not tesselate in far cascades
            else if (pass_type == PassType::SHADOW && material->m_displacement_mode == Material::DISPLACEMENT_ABS && slice_id < 2)
            {
                m_tessellated_shadow_material->m_textures[0] = material->m_textures[0];
                material = m_tessellated_shadow_material;
            }
            else if(pass_type == PassType::SHADOW && material->m_opacity_mode == Material::ALPHA_TEST)
            {
                m_transparent_shadow_material->m_textures[0] = material->m_textures[0];
                material = m_transparent_shadow_material;
            }
            else
            {
                override_material->m_textures[0] = material->m_textures[0];
                material = override_material ? override_material : material;
            }
        }
        */

        ShaderVariant::Enum shader_variant = sm->m_mesh->m_vertex_matrix_indices.size() ? ShaderVariant::SKELETAL : ShaderVariant::NORMAL;
        if ( is_instanced_draw)
        {
            shader_variant = ShaderVariant::INSTANCED;
        }
        
        GLB::GLBShader2 *s = material->m_shaders4[pass_type][shader_variant];
        CubeEnvMap *envmaps[2];
        float envmaps_interpolator = 0.0f;
        GLB::Matrix4x4 mvp;
        GLB::Matrix4x4 mv;
        GLB::Matrix4x4 model;
        GLB::Matrix4x4 inv_model;
        GLB::Matrix4x4 inv_modelview;
        Vector3D pos( sm->m_world_pom.v[12], sm->m_world_pom.v[13], sm->m_world_pom.v[14]);

        if( prev_material != material || prev_shader_variant != shader_variant || override_material)
        {
            if( prev_material)
            {
                prev_material->Unbind();
            }

            if (shadow_texture_binding_point > -1)
            {
                glBindSampler(shadow_texture_binding_point, 0);
                shadow_texture_binding_point = -1;
            }

            texture_num_from_material = 0;
            material->Bind(pass_type, shader_variant, texture_num_from_material);
        }

        KCL::uint32 texture_num = texture_num_from_material;
        prev_material = material;
        prev_shader_variant = shader_variant;

        // 
        if ( shader_variant == ShaderVariant::NORMAL)
        {
            if( material->m_material_type == KCL::Material::SKY)
            {
                mvp = sm->m_world_pom * camera->GetViewProjectionOrigo();
                mv = sm->m_world_pom * camera->GetView();
                mv.v[12] = 0.0f;
                mv.v[13] = 0.0f;
                mv.v[14] = 0.0f;
            }
            else
            {
                mvp = sm->m_world_pom * camera->GetViewProjection();
                mv = sm->m_world_pom * camera->GetView();
            }
                
            
            GLB::Matrix4x4::InvertModelView(mv, inv_modelview);
                
            model = sm->m_world_pom;
            inv_model = Matrix4x4::Invert4x3( sm->m_world_pom);
        }
                
        if (s->m_uniform_locations[GLB::uniforms::mvp] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, mvp.v);
        }
        
        if (s->m_uniform_locations[GLB::uniforms::mv] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mv], 1, GL_FALSE, mv.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::vp] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::vp], 1, GL_FALSE, camera->GetViewProjection().v);
        }

        if (s->m_uniform_locations[GLB::uniforms::prev_vp] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::prev_vp], 1, GL_FALSE, m_prev_vp.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::view] > -1)
        {
            GLB::Matrix4x4 view = camera->GetView();
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::view], 1, GL_FALSE, view.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::model] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::model], 1, GL_FALSE, model.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::inv_model] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::inv_model], 1, GL_FALSE, inv_model.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::inv_modelview] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::inv_modelview], 1, GL_FALSE, inv_modelview.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::frustum_planes] > -1 && m_active_camera)
        {            
            glUniform4fv(s->m_uniform_locations[GLB::uniforms::frustum_planes], 6, m_active_camera->GetCullPlane(0).v);
        }
        
        if (s->m_uniform_locations[GLB::uniforms::view_dir] > -1)
        {
            Vector3D view_dir( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10]);
            glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_dir], 1, view_dir.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::view_pos] > -1)
        {
            glUniform3fv(s->m_uniform_locations[GLB::uniforms::view_pos], 1, camera->GetEye().v);
        }
    
        if (s->m_uniform_locations[GLB::uniforms::carindex_translucency_ssaostr_fovscale] > -1 && m_actors.size())
        {
            float carIdx;

            if(sm->m_owner == m_carActor_evil)
            {
                carIdx = 254;

                //HACK FOR CAR PAINT MATERIAL
                if (sm->m_material->m_is_car_paint)
                {
                    carIdx = 255;
                }
            }
            else if(sm->m_owner == m_carActor_hero)
            {
                carIdx = 252;

                //HACK FOR CAR PAINT MATERIAL
                if (sm->m_material->m_is_car_paint)
                {
                    carIdx = 253;
                }
            }
            else if(sm->m_material->m_translucent_lighting_strength > 0.0f) //50..100, HACK: assume billboard is a branch with leafs, use two sided lighting to fake scattering
            {
                assert(sm->m_envmap_id < 50);
                carIdx = 50 + sm->m_envmap_id;
            }
            else //under 50
            {
                assert(sm->m_envmap_id < 50);
                carIdx = sm->m_envmap_id;
            }

            float translucent_factor = 0.0;
            if(sm->m_material->m_is_billboard && (sm->m_material->m_translucent_lighting_strength > 0.0))
            {
                translucent_factor = sm->m_material->m_translucent_lighting_strength * 0.5f; //lower half of range (0..0.5) for billboards
            }
            else if(sm->m_material->m_translucent_lighting_strength > 0.0)
            {
                translucent_factor = sm->m_material->m_translucent_lighting_strength * 0.5f + 0.5f; //upper half for non-billboards
            }

            KCL::Vector4D carindex_translucency_ssaostr_fovscale = KCL::Vector4D(carIdx / 255.0, translucent_factor, 0.0, tessellation_fov_scale);

            glUniform4fv(s->m_uniform_locations[GLB::uniforms::carindex_translucency_ssaostr_fovscale], 1, carindex_translucency_ssaostr_fovscale.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::fresnel_params] > -1)
        {
            glUniform3fv(s->m_uniform_locations[GLB::uniforms::fresnel_params], 1, material->m_fresnel_params.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::envmaps_interpolator] > -1 && m_cubemaps.size())
        {
            KRL_CubeEnvMap *em0 = envmaps[0];
            KRL_CubeEnvMap *em1 = envmaps[1];

            Get2Closest( pos, em0, em1, envmaps_interpolator);

            glUniform1f(s->m_uniform_locations[GLB::uniforms::envmaps_interpolator], envmaps_interpolator);

            envmaps[0]->GetTexture()->bind( texture_num);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap0], texture_num++);

            envmaps[1]->GetTexture()->bind( texture_num);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap1], texture_num++);

            OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
        }

        if ((s->m_uniform_locations[GLB::uniforms::light_pos] > -1) && m_visible_lights.size()) //hack GI for garage by faking reflected light with a point light
        {
            KCL::Light* l = m_visible_lights[0];
            Vector3D light_pos( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]);

            glUniform3fv(s->m_uniform_locations[GLB::uniforms::light_pos], 1, light_pos.v);
        }

        if ((s->m_uniform_locations[GLB::uniforms::light_color] > -1) && m_visible_lights.size())
        {
            KCL::Light* l = m_visible_lights[0];
            float i = 1.0f;
            if( l->m_intensity_track)
            {
                Vector4D v;

                l->t = m_animation_time / 1000.0f;

                _key_node::Get( v, l->m_intensity_track, l->t, l->tb, false);

               i = v.x;// / l->m_intensity;
            }

            glUniform3f(s->m_uniform_locations[GLB::uniforms::light_color],
                l->m_diffuse_color.x * i,
                l->m_diffuse_color.y * i,
                l->m_diffuse_color.z * i
                );
        }

        if (s->m_uniform_locations[GLB::uniforms::shadow_matrix0] > -1 && m_num_shadow_maps)
        {
            //HACK
            //if we render car1, bind 1 to slot 0, if terrain/else bind 0 to slot 0, 1 will be bound to slot 1 later if needed
            uint32 actor_idx = 0;
            if ((sm->m_owner != m_carActor_hero) && (s->m_uniform_locations[GLB::uniforms::shadow_matrix1] <= -1))
            {
                actor_idx = 1;
            }

            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix0], 1, GL_FALSE, m_global_shadowmaps[actor_idx]->m_matrix.v);

            ((ShadowMap*)m_global_shadowmaps[actor_idx])->BindShadowMap(texture_num) ;

            glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit0], texture_num++);
            OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
        }

        //car ao projection
        if (s->m_uniform_locations[GLB::uniforms::car_ao_matrix0] > -1 && m_carActor_hero)
        {
            static const Matrix4x4 shadowM (0.5f, 0, 0, 0,
                0, 0.5f, 0, 0,
                0, 0, 0.5f, 0,
                0.5f, 0.5f, 0.5f, 1);

            KCL::Actor *car_actor = m_carActor_hero;

            KCL::Vector3D carPos = /*KCL::Vector3D(-261, 375,1067);//*/KCL::Vector3D(car_actor->m_root->m_world_pom.v[12], car_actor->m_root->m_world_pom.v[13], car_actor->m_root->m_world_pom.v[14]);
            /*
            KCL::Vector3D carRight = KCL::Vector3D(car_actor->m_root->m_world_pom.v[0], car_actor->m_root->m_world_pom.v[1], car_actor->m_root->m_world_pom.v[2]);
            carRight.normalize();
            */
            KCL::Vector3D carUp = KCL::Vector3D(car_actor->m_root->m_world_pom.v[4], car_actor->m_root->m_world_pom.v[5], car_actor->m_root->m_world_pom.v[6]);
            KCL::Vector3D carFwd = KCL::Vector3D(car_actor->m_root->m_world_pom.v[8], car_actor->m_root->m_world_pom.v[9], car_actor->m_root->m_world_pom.v[10]);
            //carUp.normalize();
            //carFwd.normalize();

            //m_car_ao_cam.OrthoFocus(&(car_actor->m_aabb), carPos + carUp * 2, carPos, carFwd);
            m_car_ao_cam.LookAt(carPos + carUp, carPos, carFwd);
            float sizeTweak = 2.6f;
            m_car_ao_cam.Ortho(-sizeTweak,sizeTweak,-sizeTweak,sizeTweak, -2, 2);
            m_car_ao_cam.Update();

            KCL::Matrix4x4 car_ao_matrix = m_car_ao_cam.GetViewProjection() * shadowM;

            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::car_ao_matrix0], 1, GL_FALSE, car_ao_matrix.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::car_ao_matrix1] > -1 && m_carActor_evil)
        {
            static const Matrix4x4 shadowM (0.5f, 0, 0, 0,
                0, 0.5f, 0, 0,
                0, 0, 0.5f, 0,
                0.5f, 0.5f, 0.5f, 1);

            KCL::Actor *car_actor = m_carActor_evil;

            KCL::Vector3D carPos = /*KCL::Vector3D(-261, 375,1067);//*/KCL::Vector3D(car_actor->m_root->m_world_pom.v[12], car_actor->m_root->m_world_pom.v[13], car_actor->m_root->m_world_pom.v[14]);
            /*
            KCL::Vector3D carRight = KCL::Vector3D(car_actor->m_root->m_world_pom.v[0], car_actor->m_root->m_world_pom.v[1], car_actor->m_root->m_world_pom.v[2]);
            carRight.normalize();
            */
            KCL::Vector3D carUp = KCL::Vector3D(car_actor->m_root->m_world_pom.v[4], car_actor->m_root->m_world_pom.v[5], car_actor->m_root->m_world_pom.v[6]);
            KCL::Vector3D carFwd = KCL::Vector3D(car_actor->m_root->m_world_pom.v[8], car_actor->m_root->m_world_pom.v[9], car_actor->m_root->m_world_pom.v[10]);
            //carUp.normalize();
            //carFwd.normalize();

            //m_car_ao_cam.OrthoFocus(&(car_actor->m_aabb), carPos + carUp * 2, carPos, carFwd);
            m_car_ao_cam.LookAt(carPos + carUp, carPos, carFwd);
            float sizeTweak = 2.6f;
            m_car_ao_cam.Ortho(-sizeTweak,sizeTweak,-sizeTweak,sizeTweak,-2,2);
            m_car_ao_cam.Update();

            KCL::Matrix4x4 car_ao_matrix = m_car_ao_cam.GetViewProjection() * shadowM;

            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::car_ao_matrix1], 1, GL_FALSE, car_ao_matrix.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::shadow_matrix1] > -1 && m_num_shadow_maps)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::shadow_matrix1], 1, GL_FALSE, m_global_shadowmaps[1]->m_matrix.v);

            ((ShadowMap*)m_global_shadowmaps[1])->BindShadowMap(texture_num) ;
           
            glUniform1i(s->m_uniform_locations[GLB::uniforms::shadow_unit1], texture_num++);

            OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
        }

        if (s->m_uniform_locations[GLB::uniforms::inv_resolution] > -1)
        {
            glUniform2f(s->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
        }

        if (s->m_uniform_locations[GLB::uniforms::color] > -1)
        {
            glUniform3fv(s->m_uniform_locations[GLB::uniforms::color], 1, sm->m_color.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::translate_uv] > -1)
        {
            if( sm->m_material->m_frame_when_animated != m_frame)
            {
                //if the test has looped, reset animation offsets
                if(sm->m_material->m_animation_time > m_animation_time / 1000.0f)
                {
                    sm->m_material->m_animation_time_base = 0.0f;
                }

                sm->m_material->m_animation_time = m_animation_time / 1000.0f;

                if( sm->m_material->m_translate_u_track)
                {
                    Vector4D r;

                    _key_node::Get( r, sm->m_material->m_translate_u_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

                    sm->m_material->m_uv_offset.x = -r.x;
                }
                if( sm->m_material->m_translate_v_track)
                {
                    Vector4D r;

                    _key_node::Get( r, sm->m_material->m_translate_v_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

                    sm->m_material->m_uv_offset.y = -r.x;
                }
                sm->m_material->m_frame_when_animated = m_frame;
            }

            glUniform2fv(s->m_uniform_locations[GLB::uniforms::translate_uv], 1, sm->m_material->m_uv_offset.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::particle_data] > -1)
        {
            glUniform4fv(s->m_uniform_locations[GLB::uniforms::particle_data], MAX_PARTICLE_PER_MESH, m_particle_data[sm->m_offset1].v);
        }
        if (s->m_uniform_locations[GLB::uniforms::particle_color] > -1)
        {
            glUniform4fv(s->m_uniform_locations[GLB::uniforms::particle_color], MAX_PARTICLE_PER_MESH, m_particle_color[sm->m_offset1].v);
        }
        if (s->m_uniform_locations[GLB::uniforms::alpha_threshold] > -1)
        {
            glUniform1f(s->m_uniform_locations[GLB::uniforms::alpha_threshold], sm->m_alpha);
        }

        if (s->m_uniform_locations[GLB::uniforms::world_fit_matrix] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::world_fit_matrix], 1, GL_FALSE, m_world_fit_matrix.v);
        }

        if (s->m_uniform_locations[GLB::uniforms::tessellation_factor] > -1)
        {
            if (m_tessellation_enabled)
            {
                glUniform4fv(s->m_uniform_locations[GLB::uniforms::tessellation_factor], 1, sm->m_material->m_tessellation_factor.v);
            }
            else
            {
                glUniform4fv(s->m_uniform_locations[GLB::uniforms::tessellation_factor], 1, KCL::Vector4D(1.0f, 1.0f, 1.0f, 100000.0f).v);
            }
        }

        if( s->m_uniform_locations[GLB::uniforms::cam_near_far_pid_vpscale] > -1)
        {
            int val = 1;
            if (slice_id == 1)
            {
                val = 1;
            }
            else if (slice_id == 2)
            {
                val = -1;
            }

            float tess_vp_scale = sqrtf(float(m_viewport_width) / 1920.0 * float(m_viewport_height) / 1080.0);

            glUniform4fv(s->m_uniform_locations[GLB::uniforms::cam_near_far_pid_vpscale], 1, KCL::Vector4D(camera->GetNear(), camera->GetFar(), val, tess_vp_scale).v);
        }

        if (s->m_uniform_locations[GLB::uniforms::inv_view] > -1)
        {
            GLB::Matrix4x4 inv_view;
            GLB::Matrix4x4 v = camera->GetView();
            GLB::Matrix4x4::InvertModelView(v, inv_view);
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::inv_view], 1, GL_FALSE, inv_view.v);
        }

        // mvp2: prev mvp for motion blur. For instanced meshes it comes from the instance buffer
        if (s->m_uniform_locations[GLB::uniforms::mvp2] > -1)
        {
            switch( shader_variant)
            {
            case ShaderVariant::NORMAL:
                {
                    KCL::Matrix4x4 mvp2;

                    if( material->m_material_type == KCL::Material::SKY)
                    {
                        mvp2 = sm->m_world_pom * camera->GetViewProjectionOrigo();
                    }
                    else
                    {
                        mvp2 = sm->m_prev_world_pom * m_prev_vp;
                    }

                    glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp2], 1, GL_FALSE, mvp2.v);

                    break;
                }
            case ShaderVariant::SKELETAL:
                {
                    glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp2], 1, GL_FALSE, m_prev_vp.v);
                    break;
                }
            }
        }

        if (s->m_uniform_locations[GLB::uniforms::mblur_mask] > -1)
        {
            glUniform1f(s->m_uniform_locations[GLB::uniforms::mblur_mask], sm->m_is_motion_blurred);
        }

        if (s->m_uniform_locations[GLB::uniforms::view_port_size] > -1)
        {
            glUniform2f(s->m_uniform_locations[GLB::uniforms::view_port_size], float(m_viewport_width), float(m_viewport_height));
        }
                
        if (s->m_uniform_locations[GLB::uniforms::hdr_params] > -1)
        {
            glUniform2f(s->m_uniform_locations[GLB::uniforms::hdr_params], 1.5f, 12.0f);
        }

        if( s->m_uniform_locations[GLB::uniforms::dpcam_view] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::dpcam_view], 1, GL_FALSE, m_dpcam.GetView().v);
        }

        // cascaded_shadow_texture_array
        if (s->m_uniform_locations[GLB::uniforms::cascaded_shadow_texture_array] > -1)
        {
            OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
            m_cascaded_shadow_map->BindTexture(texture_num);
            shadow_texture_binding_point = texture_num;
            glUniform1i(s->m_uniform_locations[GLB::uniforms::cascaded_shadow_texture_array], texture_num++);
            OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
        }
         
        // cascaded_shadow_matrices
        if (s->m_uniform_locations[GLB::uniforms::cascaded_shadow_matrices] > -1)
        {
            glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::cascaded_shadow_matrices], CASCADE_COUNT, false, m_cascaded_shadow_map->GetShadowMatrices()[0].v);
        }

        // cascaded_frustum_distances
        if (s->m_uniform_locations[GLB::uniforms::cascaded_frustum_distances] > -1)
        {
            glUniform4fv(s->m_uniform_locations[GLB::uniforms::cascaded_frustum_distances], 1, m_cascaded_shadow_map->GetFrustumDistances().v);
        }

        if (s->m_uniform_locations[GLB::uniforms::envmap1] > -1)
        {
            //Get2Closest( pos, em0, em1, envmaps_interpolator);
            //glUniform1f( s->m_uniform_locations[GLB::uniforms::envmaps_interpolator], envmaps_interpolator);

            OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_dynamic_cubemaps[0]->GetId()); //uses texparameter, not samplers for now

            glBindSampler(texture_num, 0);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap1], texture_num++);
        }
        if (s->m_uniform_locations[GLB::uniforms::envmap2] > -1)
        {
            OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
            glBindTexture(GL_TEXTURE_CUBE_MAP, m_dynamic_cubemaps[1]->GetId()); //uses texparameter, not samplers for now

            glBindSampler(texture_num, 0);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap2], texture_num++);
        }

        if (s->m_uniform_locations[GLB::uniforms::envmap1_dp] > -1)
        {
            //Get2Closest( pos, em0, em1, envmaps_interpolator);
            //glUniform1f( s->m_uniform_locations[GLB::uniforms::envmaps_interpolator], envmaps_interpolator);

            OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_dynamic_cubemaps[0]->GetId()); //uses texparameter, not samplers for now

            glBindSampler(texture_num, 0);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap1_dp], texture_num++);
        }
        if (s->m_uniform_locations[GLB::uniforms::envmap2_dp] > -1)
        {
            OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
            glBindTexture(GL_TEXTURE_2D_ARRAY, m_dynamic_cubemaps[1]->GetId()); //uses texparameter, not samplers for now

            glBindSampler(texture_num, 0);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::envmap2_dp], texture_num++);
        }
        if (s->m_uniform_locations[GLB::uniforms::static_envmaps] > -1 && m_static_cubemaps)
        {
            OpenGLStateManager::GlActiveTexture( GL_TEXTURE0 + texture_num);
            glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, m_static_cubemaps->textureObject());

            glBindSampler(texture_num, 0);
            glUniform1i(s->m_uniform_locations[GLB::uniforms::static_envmaps], texture_num++);
        }
        if (s->m_uniform_locations[GLB::uniforms::near_far_ratio] > -1)
        {
            glUniform1f(s->m_uniform_locations[GLB::uniforms::near_far_ratio], -camera->GetProjection().v33);
        }
        if (s->m_uniform_locations[GLB::uniforms::view_port_size] > -1)
        {
            glUniform2f(s->m_uniform_locations[GLB::uniforms::view_port_size], m_viewport_width, m_viewport_height);
        }
        if (s->m_uniform_locations[GLB::uniforms::hiz_texture] > -1)
        {
            OpenGLStateManager::GlActiveTexture(GL_TEXTURE0 + texture_num);
            glBindTexture(GL_TEXTURE_2D, m_occlusion_cull->GetHiZDepthTexture());

            glBindSampler(texture_num, m_occlusion_cull->GetHiZDepthSampler());
            glUniform1i(s->m_uniform_locations[GLB::uniforms::hiz_texture], texture_num++);
        }
        if (s->m_uniform_locations[GLB::uniforms::tessellation_multiplier] > -1)
        {
            float tessellation_multiplier;
            // Precalculate the tessellation multiplier factor
            if (material->m_displacement_mode == KCL::Material::DISPLACEMENT_LOCAL)
            {
                // Local displacement
                tessellation_multiplier = 800.0f * m_tessellation_viewport_scale * tessellation_fov_scale;
            }
            else
            {
                // Bezier patch
                float tessellation_scale = m_tessellation_enabled ? sm->m_material->m_tessellation_factor.y : 1.0f;
                tessellation_multiplier = 1200.0f * (1.0f + tessellation_scale) * m_tessellation_viewport_scale * tessellation_fov_scale;
            }
            glUniform1f(s->m_uniform_locations[GLB::uniforms::tessellation_multiplier], tessellation_multiplier);
        }
        
#ifdef HAVE_GUI_FOLDER
        if (s->m_uniform_locations[GLB::uniforms::editor_mesh_selected] > -1)
        {
            //glUniform1i(s->m_uniform_locations[GLB::uniforms::editor_mesh_selected], ((GFXGui*) m_gui.get())->IsMeshSelected(sm) ? 1 : 0);
        }
#endif
        GFXB4::Mesh3* glb_mesh3 = dynamic_cast<GFXB4::Mesh3*>(sm->m_mesh);
        KCL::uint32 new_vao = glb_mesh3->m_vao_4;
        if (new_vao != current_vao)
        {
            glBindVertexArray(new_vao);
            current_vao = new_vao;
        }

        if ((pass_type == PassType::REFLECTION) && (slice_id == 1) && (material->m_material_type != KCL::Material::SKY))
        {
            OpenGLStateManager::GlEnable(GL_CULL_FACE);
            OpenGLStateManager::GlCullFace(GL_FRONT);
        }

        GLB::OpenGLStateManager::Commit();

        primitive_type = GL_TRIANGLES;
        if (s->HasShader(GLB::GLBShader2::ShaderTypes::TessEvaluationShader) )
        {
            glPatchParameteriProc( GL_PATCH_VERTICES, sm->m_mesh->m_num_patch_vertices);
            primitive_type = GL_PATCHES;
        }

        if(shader_variant != ShaderVariant::INSTANCED)
        {   
            if (occlusion_cull)
            {
                assert(sm->m_indirect_draw_id > -1);
                glDrawElementsIndirectProc(
                    primitive_type,
                    GL_UNSIGNED_SHORT,
                    (const GLvoid*)(sm->m_indirect_draw_id * sizeof(OcclusionCull::DrawElementsIndirectCommand)));
            }
            else
            {
                glDrawElements(primitive_type, sm->m_primitive_count ? sm->m_primitive_count : KCL::uint32(sm->m_mesh->getIndexCount(lod)), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[lod].m_offset);
            }
        }
        else
        {
            // Instanced draw
            assert(s->m_uniform_locations[GLB::uniforms::instance_offset] > -1);
            if (occlusion_cull)
            {
                // Instancing with occlusiong culling
                assert(sm->m_mio2->m_indirect_draw_id > -1);
                
                glUniform1i(s->m_uniform_locations[GLB::uniforms::instance_offset], 0);

                // Bind the instance buffer
                KCL::uint32 buffer_offset = (m_occlusion_cull->GetInstanceBufferOffset() + sm->m_mio2->m_indirect_draw_id) * m_instance_manager->GetInstanceBlockSize();
                glBindBufferRange(GL_UNIFORM_BUFFER,
                    INSTANCE_BINDING_SLOT,
                    m_occlusion_cull->GetInstanceBuffer(),
                    buffer_offset,
                    m_instance_manager->GetInstanceBlockSize());
              
                glDrawElementsIndirectProc(
                    primitive_type,
                    GL_UNSIGNED_SHORT,
                    (const GLvoid*)(sm->m_mio2->m_indirect_draw_id * sizeof(OcclusionCull::DrawElementsIndirectCommand)));
            }
            else
            {
                // Instancing without occlusion culling
                KCL::uint32 instances_to_render = sm->m_num_visible_instances;
                KCL::uint32 instance_batch_size = 0;
                KCL::uint32 instance_offset = 0;
                while (instances_to_render > 0)
                {
                    m_instance_manager->BindInstanceBuffer(INSTANCE_BINDING_SLOT, instances_to_render, instance_offset, instance_batch_size);
                    glUniform1i(s->m_uniform_locations[GLB::uniforms::instance_offset], instance_offset);

#ifdef HAVE_GUI_FOLDER       
                    if (pass_type == PassType::NORMAL)
                    {
                        //((GFXGui*)m_gui.get())->BindSelectedUBO(instances_to_render);
                    }
#endif
                    const GLsizei element_count = static_cast<GLsizei>(sm->m_mesh->getIndexCount(lod));
                    const void *index_offset = sm->m_mesh->m_ebo[lod].m_offset;
                    glDrawElementsInstanced(primitive_type, element_count, GL_UNSIGNED_SHORT, index_offset, instance_batch_size);
                    
                    instances_to_render -= instance_batch_size;
                }
            }
        }
        
        //set back the mesh to what LOD selection decided to use in KCL Scenehandler's Animate()
        sm->m_mesh = orig_selected_mesh;

        sm->m_materials[RENDER_MATERIAL_ID] = NULL;
    }

    if (m_is_main_pass && m_visible_instances.size())
    {
        // Workaround for old drivers: Issue a draw call with zero index offset without rasterization before indirect draw calls
        // Note: during the test, primitve type will be always GL_TRIANGLES here, but it can be different during engine development
        if (primitive_type == GL_TRIANGLES)
        {
            OpenGLStateManager::GlCullFace(GL_FRONT_AND_BACK);
            OpenGLStateManager::Commit();

            glDrawElements(primitive_type, 3, GL_UNSIGNED_SHORT, 0);

            OpenGLStateManager::GlCullFace(GL_BACK);
            OpenGLStateManager::Commit();
        }
    }

    if(prev_material)
    {
        prev_material->Unbind();
    }

    if (shadow_texture_binding_point > -1)
    {
        glBindSampler(shadow_texture_binding_point, 0);
    }

    glBindVertexArray(0);

    VboPool::Instance()->BindBuffer(0);
    IndexBufferPool::Instance()->BindBuffer(0);
    if (occlusion_cull)
    {
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
    }
}


void GLB_Scene4::Animate()
{
    // Calculate the delta time. It can be negative
    KCL::int32 delta_time;
    if (m_last_animation_time < 0)
    {
        // Ensure first frame delta time is zero even during single frame rendering
        delta_time = 0;
    }
    else
    {
        delta_time = m_animation_time - m_last_animation_time;
#if 0
        if(delta_time == 0)
        {
            delta_time = 40; //40 ms to simulate 25 fps if animation is stopped
        }
#endif
    }   

    bool paused = m_last_animation_time == m_animation_time;

    m_last_animation_time = m_animation_time;

    {
        // Set the sun direction according to the animation time
        // (Times of the second camera shot)
        if((m_animation_time >= 5000) && (m_animation_time < 7209))
        {
            m_light_dir = KCL::Vector3D(-0.984f, 0.1736f, 0.0f).normalize();
        }
        else
        {
            m_light_dir = m_light_dir_orig;
        }
    }

    if (m_HDR_exposure && !paused)
    {
        KCL::Vector4D result;
        float time = m_animation_time / 1000.0f;
        float time_base = 0;
        _key_node::Get(result, m_HDR_exposure, time, time_base, false);
        m_ubo_frame.m_ubo.exposure_bloomthreshold_tone_map_white_pad.x = result.x;
    }

    //calculate tone map for linear white

    KCL::Vector4D ABCD = m_ubo_frame.m_ubo.ABCD;
    KCL::Vector4D EFW_tau = m_ubo_frame.m_ubo.EFW_tau;
    const float& A = ABCD.x; // ShoulderStrength;
    const float& B = ABCD.y; // LinearStrength;
    const float& C = ABCD.z; // LinearAngle;
    const float& D = ABCD.w; // ToeStrength;

    const float& E = EFW_tau.x; // ToeNumerator;
    const float& F = EFW_tau.y; // ToeDenominator;
    const float& W = EFW_tau.z; //LinearWhite

    m_ubo_frame.m_ubo.exposure_bloomthreshold_tone_map_white_pad.z= ((W*(A*W+C*B)+D*E)/(W*(A*W+B)+D*F))-E/F;

    m_ubo_frame.m_ubo.time_dt_pad2.x = m_animation_time * 0.001f;
    m_ubo_frame.m_ubo.time_dt_pad2.y = delta_time * 0.001f; // to seconds
    m_ubo_frame.m_ubo.global_light_dir = KCL::Vector4D(m_light_dir, 1.0);
    m_ubo_frame.m_ubo.global_light_color = KCL::Vector4D(m_light_color * m_sunColorStrength, 1.0);
    m_ubo_frame.m_ubo.fogCol = KCL::Vector4D(m_fogColor * m_fogColorStrength);

    // Motion blur
    const float max_velocity = float(m_compute_motion_blur->GetTileSize()) / float(m_viewport_width) * 1.5f;
    m_ubo_frame.m_ubo.mb_velocity_min_max_sfactor_pad.x = 0.5f / float(m_viewport_width);
    m_ubo_frame.m_ubo.mb_velocity_min_max_sfactor_pad.y = max_velocity;
    m_ubo_frame.m_ubo.mb_velocity_min_max_sfactor_pad.z = float(m_compute_motion_blur->GetSampleCount()) / max_velocity;
    
    // Detect camera clip changes
    m_camera_clip_changed = false;

    KCL::int32 current_camera_clip = GetCameraClipId(m_animation_time);
    if (current_camera_clip != m_prev_camera_clip)
    {
        m_prev_camera_clip = current_camera_clip;
        m_camera_clip_changed = true;
        m_prev_mvp_valid = false;
    }
    else
    {
        // Check if the prev model-view-projection matrices are valid. (Used by Motion Blur)
        // The prev MVP matrices are calculated during the animation of the scene by using time = m_animation_time - 5.
        // (See KCL::SceneHandler::Animate() for details.)
        KCL::int32 prev_mvp_clip = GetCameraClipId(m_animation_time - 5);
        m_prev_mvp_valid = (prev_mvp_clip == current_camera_clip);
    }

    GLB_Scene_ES2_::Animate();
}


void GLB_Scene4::CollectInstances( std::vector<GLB::Mesh3::InstanceData> &instance_data, std::vector<KCL::Mesh*> &instances, const std::vector<KCL::MeshInstanceOwner2*> &visible_mios)
{
    instances.resize( visible_mios.size());

    for( size_t i=0; i<visible_mios.size(); i++)
    {
        visible_mios[i]->m_visible_instances[0]->m_num_visible_instances = visible_mios[i]->m_visible_instances.size();

        for( size_t j=0; j<visible_mios[i]->m_visible_instances.size(); j++)
        {
            KCL::Mesh *m = visible_mios[i]->m_visible_instances[j];

            KRL::Mesh3::InstanceData id;

            KCL::Matrix4x4 mat = m->m_world_pom;
            if(m->m_material->m_is_billboard) //billboards shall only have the translation part
            {
                mat = KCL::Matrix4x4();
                mat.v41 = m->m_world_pom.v41; mat.v42 = m->m_world_pom.v42; mat.v43 = m->m_world_pom.v43;
            }
            id.mv = mat;
            
            KCL::Matrix4x4::InvertModelView( id.mv, id.inv_mv);

            instance_data.push_back( id);
        }

        instances[i] = visible_mios[i]->m_visible_instances[0];
    }
}


void GLB_Scene4::CollectInstances( std::vector<GLB::Mesh3::InstanceData> &instance_data, std::vector<KCL::Mesh*> &instances, const std::vector< std::vector<KCL::Mesh*> > &visible_instances)
{
    instances.resize( visible_instances.size());

    for( size_t i=0; i<visible_instances.size(); i++)
    {
        visible_instances[i][0]->m_num_visible_instances = visible_instances[i].size();

        for( size_t j=0; j<visible_instances[i].size(); j++)
        {
            KCL::Mesh *m = visible_instances[i][j];

            KRL::Mesh3::InstanceData id;

            KCL::Matrix4x4 mat = m->m_world_pom;
            if(m->m_material->m_is_billboard) //billboards shall only have the translation part
            {
                mat = KCL::Matrix4x4();
                mat.v41 = m->m_world_pom.v41; mat.v42 = m->m_world_pom.v42; mat.v43 = m->m_world_pom.v43;
            }
            id.mv = mat;
            
            KCL::Matrix4x4::InvertModelView( id.mv, id.inv_mv);

            instance_data.push_back( id);
        }
            
        instances[i] = visible_instances[i][0];
    }
}


void GLB_Scene4::CollectRenderMaterials(std::vector<KCL::Mesh*> &meshes, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id)
{
    for (size_t i = 0; i < meshes.size(); i++)
    {
        //SetRenderMaterial(meshes[i], override_material, pass_type, slice_id);

        KCL::Mesh *mesh = meshes[i];
        KCL::Material *material = mesh->m_material;

        if (override_material)
        {
            if (pass_type == PassType::SHADOW)
            {
                if (material->m_is_billboard)
                {
                    m_transparent_billboard_shadow_material->m_textures[0] = material->m_textures[0];
                    material = m_transparent_billboard_shadow_material;
                }
                else if (material->m_displacement_mode == Material::DISPLACEMENT_ABS && slice_id < 2)
                {
                    // Do not tesselate in far cascades
                    m_tessellated_shadow_material->m_textures[0] = material->m_textures[0];
                    material = m_tessellated_shadow_material;
                }
                else if (material->m_opacity_mode == Material::ALPHA_TEST)
                {
                    m_transparent_shadow_material->m_textures[0] = material->m_textures[0];
                    material = m_transparent_shadow_material;
                }
                else
                {
                    override_material->m_textures[0] = material->m_textures[0];
                    material = override_material;
                }
            }
            else
            {
                override_material->m_textures[0] = material->m_textures[0];
                material = override_material;
            }
        }
        mesh->m_materials[RENDER_MATERIAL_ID] = material;
    }
}


void GLB_Scene4::CollectRenderMaterials(std::vector<KCL::MeshInstanceOwner2*> &visible_mios, KCL::Material *override_material, PassType::Enum pass_type, KCL::uint32 slice_id)
{
    for (size_t i = 0; i < visible_mios.size(); i++)
    {
        //SetRenderMaterial(visible_mios[i]->m_visible_instances[0], override_material, pass_type, slice_id);

        KCL::Mesh *mesh = visible_mios[i]->m_visible_instances[0];
        KCL::Material *material = mesh->m_material;

        if (override_material)
        {
            if (pass_type == PassType::SHADOW)
            {
                if (material->m_is_billboard)
                {
                    m_transparent_billboard_shadow_material->m_textures[0] = material->m_textures[0];
                    material = m_transparent_billboard_shadow_material;
                }
                else if (material->m_displacement_mode == Material::DISPLACEMENT_ABS && slice_id < 2)
                {
                    // Do not tesselate in far cascades
                    m_tessellated_shadow_material->m_textures[0] = material->m_textures[0];
                    material = m_tessellated_shadow_material;
                }
                else if (material->m_opacity_mode == Material::ALPHA_TEST)
                {
                    m_transparent_shadow_material->m_textures[0] = material->m_textures[0];
                    material = m_transparent_shadow_material;
                }
                else
                {
                    override_material->m_textures[0] = material->m_textures[0];
                    material = override_material;
                }
            }
            else
            {
                override_material->m_textures[0] = material->m_textures[0];
                material = override_material;
            }
        }
        mesh->m_materials[RENDER_MATERIAL_ID] = material;

    }
}


KCL::KCL_Status GLB_Scene4::InitEmitters()
{
    int single_frame = m_gte->GetTestDescriptor()->m_single_frame;
    bool single_frame_mode = single_frame != -1;

    for (int i = 0; i < NUM_OF_EMITTERS; i++)
    {
        m_emitters[i] = new GLB::ComputeEmitter(256, single_frame_mode);
        m_emitters[i]->Init(m_warmup_helper);
        std::string name = "emitter";
        name.push_back('0' + i);
        m_emitters[i]->SetName( name );
        m_emitters[i]->SetParticleLifetime( 1.1f);
        m_emitters[i]->SetGravity( -2.0f);
        m_emitters[i]->SetFocusDistance( 0.01f);
    }

    {
        KCL::AssetFile pos_track_file("animations/car_hero_dust_emitter_rate_track");
        if(!pos_track_file.GetLastError())
        {
            _key_node::Read( m_emitter_rate_anim[0], pos_track_file);
            pos_track_file.Close();
        }
        else
        {
            INFO("ERROR: Can not load car_hero_dust_emitter_rate_track!");
            return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
        }
    }

    {
        KCL::AssetFile pos_track_file("animations/car_evil_dust_emitter_rate_track");
        if(!pos_track_file.GetLastError())
        {
            _key_node::Read( m_emitter_rate_anim[1], pos_track_file);
            pos_track_file.Close();
        }
        else
        {
            INFO("ERROR: Can not load car_evil_dust_emitter_rate_track!");
            return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
        }
    }

    //
    //  load particle buffers if exists
    //
    if (single_frame_mode)
    {
        for (int i = 0; i < NUM_OF_EMITTERS; i++)
        {
            m_emitters[i]->LoadState(single_frame);
        }
    }

    return KCL::KCL_TESTERROR_NOERROR;
}

void GLB_Scene4::SimulateParticles()
{
    if (m_is_compute_warmup)
    {
        return;
    }

    bool need_to_emit = false;
    bool need_to_simulate = false;

    for( int i=0; i < NUM_OF_EMITTERS; i++)
    {
        KCL::_key_node *emitter_anim_track = m_emitter_rate_anim[i/2];

        KCL::Node* node = NULL;
        if ( m_carActor_hero)
        {
            if( i == 0)
            {
                node = KCL::Node::SearchNode( m_carActor_hero->m_root, "model_car_hero_wmove_BL_loc");
            }
            else if( i == 1)
            {
                node = KCL::Node::SearchNode( m_carActor_hero->m_root, "model_car_hero_wmove_BR_loc");
            }
        }

        if ( m_carActor_evil)
        {
            if( i == 2)
            {
                node = KCL::Node::SearchNode( m_carActor_evil->m_root, "model_car_evil_wmove_BL_loc");
            }
            else if( i == 3)
            {
                node = KCL::Node::SearchNode( m_carActor_evil->m_root, "model_car_evil_wmove_BR_loc");
            }
        }

        if ( node && emitter_anim_track)
        {
            KCL::Vector4D r;
            float time = m_animation_time * m_animation_multiplier / 1000.0f;
            float time_base = 0.0f;

            KCL::_key_node::Get( r, emitter_anim_track, time, time_base);

            m_emitters[i]->AdjustToDisplace4(node->m_world_pom, r.x);

            float delta_time = m_ubo_frame.m_ubo.time_dt_pad2.y;
            delta_time = (delta_time < 0) ? 0 : delta_time;
            m_emitters[i]->Animate(delta_time, m_animation_time);

            need_to_emit |= m_emitters[i]->NeedToEmit();
            need_to_simulate |= m_emitters[i]->NeedToSimulate();
        }
    }

    if (need_to_emit)
    {
        for (int i = 0; i < NUM_OF_EMITTERS; i++)
        {
            m_emitters[i]->Emit();
        }

        glMemoryBarrierProc((need_to_simulate ? 0 : GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT) | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    if (need_to_simulate)
    {
        for (int i = 0; i < NUM_OF_EMITTERS; i++)
        {
            m_emitters[i]->Simulate();
        }

        glMemoryBarrierProc(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
    }

    //
    //  save particle buffers
    //
    const std::vector<int> &frames = m_gte->GetTestDescriptor()->m_particle_buffer_save_frames;
    if (frames.size())
    {
        bool save_buffers = std::find(frames.begin(), frames.end(), m_animation_time) != frames.end();

        if (save_buffers)
        {
            glMemoryBarrierProc(GL_SHADER_STORAGE_BARRIER_BIT);

            for (int i = 0; i < NUM_OF_EMITTERS; i++)
            {
                m_emitters[i]->SaveState(m_animation_time);
            }
        }
    }
}


void GLB_Scene4::RenderParticles()
{  
    KCL::Texture *tt[2] = 
    {
        m_smokeMaterial->m_textures[2],
        m_topdown_shadow
    };
    KCL::Vector3D eye_forward(-m_active_camera->GetView().v13, -m_active_camera->GetView().v23, -m_active_camera->GetView().v33);

    for( int i=0; i < NUM_OF_EMITTERS; i++)
    {
        m_emitters[i]->Render( m_billboard_point_gs, m_active_camera->GetViewProjection(), m_active_camera->GetView(), m_active_camera->GetEye(), tt, m_topdown_shadow_sampler, eye_forward);
    }
}


void GLB_Scene4::Render()
{
    ProfilerBeginFrame(m_animation_time);

    ProfilerBeginPass("UBOs");
    m_ubo_manager->BeginFrame();

    m_ubo_manager->SetUBO<UBlockFrame>(m_ubo_frame.m_handle, &m_ubo_frame.m_ubo);
    m_ubo_manager->Upload();
    m_ubo_manager->BindUBO(m_ubo_frame.m_handle, FRAME_CONSTS_BINDING_SLOT);
    ProfilerEndPass();

    GLB::OpenGLStateManager::Reset();

//#if defined GL_FRAMEBUFFER_SRGB
//  if (m_framebuffer_gamma_correction)
//      glEnable(GL_FRAMEBUFFER_SRGB) ;
//#endif

    RenderCascadedShadow();   

    // Main camera occlusion cull
    ProfilerBeginPass("Generate HiZ");
    if (m_occlusion_cull_enabled)
    {
        m_occlusion_cull->CreateHiZ(m_active_camera, m_occluders);
    }
    ProfilerEndPass();

    // Execute the occlusion cull
    ProfilerBeginPass("OC");
    if (m_occlusion_cull_enabled)
    {
        m_occlusion_cull->ExecuteOcclusionCull(m_active_camera, m_visible_meshes[0], m_visible_instances);
    }
    ProfilerEndPass();

    bool hero_visible = false;
    bool evil_visible = false;
    for(int i=0;i<m_visible_actors.size();++i)
    {
        if(m_visible_actors[i] == m_carActor_hero)
        {
            hero_visible = true;
        }
        else if(m_visible_actors[i] == m_carActor_evil)
        {
            evil_visible = true;
        }
    }

    KCL::Material* orig_sky_mat0 = NULL;
    KCL::Material* orig_sky_mat1 = NULL;
    if (m_sky_mesh.size())
    {
        orig_sky_mat0 = m_sky_mesh[0]->m_materials[0];
        orig_sky_mat1 = m_sky_mesh[0]->m_materials[1];
        for(int i=0; i<m_sky_mesh.size(); ++i)
        {
            m_sky_mesh[i]->SetMaterials(m_sky_mat_paraboloid, m_sky_mat_paraboloid);
        }
    }   

    ProfilerBeginPass("Envmap#0");
    if(m_carActor_hero && (hero_visible || !cubemaps_inited))
    {
            KCL::Vector3D pos( m_carActor_hero->m_root->m_world_pom.v[12], m_carActor_hero->m_root->m_world_pom.v[13] , m_carActor_hero->m_root->m_world_pom.v[14]);

            pos.y += 2.0f;

            UpdateEnvmap( pos, 0);
    }
    ProfilerEndPass();
    
    ProfilerBeginPass("Envmap#1");
    if(m_carActor_evil && (evil_visible || !cubemaps_inited))
    {
            KCL::Vector3D pos( m_carActor_evil->m_root->m_world_pom.v[12], m_carActor_evil->m_root->m_world_pom.v[13] , m_carActor_evil->m_root->m_world_pom.v[14]);

            pos.y += 2.0f;

            UpdateEnvmap( pos, 1);
    }
    ProfilerEndPass();

    cubemaps_inited = true;

    for(int i=0; i<m_sky_mesh.size(); ++i)
    {
        m_sky_mesh[i]->SetMaterials(orig_sky_mat0, orig_sky_mat1);
    }
    //NOT USED IN GFXB4, CASCADED REPLACED THIS
    //for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
    //{
    //  if( m_global_shadowmaps[i])
    //  {
    //      RenderShadow( (ShadowMap*)m_global_shadowmaps[i] );
    //  }
    //}

    //NOT USED IN GFXB4, DYN ENVMAPS REPLACED THIS
    //for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
    //{
    //  //RenderPlanar( m_visible_planar_maps[i]);
    //}

    m_gbuffer->BindWorldBuffer();
    m_gbuffer->ClearWorldBuffer();

#ifdef HAVE_GLEW
    if( m_wireframe_render && m_wireframe_mode == WireframePolygon)
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE);
    } 
#endif

    // Render not instanced, opaque meshes
    ProfilerBeginPass("Main");
    m_is_main_pass = true;
    GLB_Scene4Tools::SortMeshes(m_visible_meshes[0], m_active_camera, false);
    Render(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0, PassType::NORMAL, m_occlusion_cull_enabled && m_cull_non_instanced_meshes_enabled);
    m_is_main_pass = false;
    ProfilerEndPass();

    if (m_render_occluders_enabled)
    {
        Render(m_active_camera, m_occluders, 0, 0, 0, 0, PassType::NORMAL, false);
    }

    if (m_visible_instances.size())
    {
        // Collect the instanced meshes
        std::vector<GLB::Mesh3::InstanceData> instance_data;
        std::vector<KCL::Mesh*> instances;

        CollectInstances(instance_data, instances, m_visible_instances);

        if (!m_occlusion_cull_enabled)
        {
            // Upload the mesh matrices
            m_instance_manager->UploadInstanceData(instance_data);

#ifdef HAVE_GUI_FOLDER
            //((GFXGui*)m_gui.get())->UploadSelectedData(m_visible_instances);
#endif
        }

        // Render instanced meshes
        ProfilerBeginPass("MainInst");
        Render(m_active_camera, instances, 0, 0, 0, 0, PassType::NORMAL, m_occlusion_cull_enabled);
        ProfilerEndPass();
    }

#ifdef HAVE_GLEW
    if( m_wireframe_render && m_wireframe_mode == WireframePolygon)
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL);
    }
#endif

    m_gbuffer->DownsampleDepth(m_active_camera, m_fullscreen_quad_vao);

//Render 1/2 res SSAO
    ProfilerBeginPass("SSAO");
    glDrawBuffers( 1, m_gbuffer->m_draw_buffers);
    filters[SSAO].m_active_camera = m_active_camera ;
    filters[SSAO].m_input_depth_textures[0] = m_gbuffer->m_depth_texture;
    filters[SSAO].m_input_textures[0] = m_gbuffer->m_depth_hiz_texture;
    filters[SSAO].m_input_textures[1] = m_gbuffer->m_normal_map;
    filters[SSAO].m_ssao_projection_scale = fabsf(-float(m_viewport_height) / (2.0f * tanf(KCL::Math::Rad(m_active_camera->GetFov() * 0.5f))));
    filters[SSAO].Render(STATISTIC_SAMPLES);
    ProfilerEndPass();

//Shadow target
    ProfilerBeginPass("SSDS");
    filters[SSDS].m_active_camera = m_active_camera;
    filters[SSDS].m_input_depth_textures[0] = m_gbuffer->m_depth_texture;
    filters[SSDS].m_input_textures[1] = filters[SSAO].m_color_texture;
    filters[SSDS].m_input_textures[7] = m_topdown_shadow ? m_topdown_shadow->textureObject() : 0;
    filters[SSDS].m_input_samplers[7] = m_topdown_shadow_sampler ? m_topdown_shadow_sampler : 0;
    filters[SSDS].m_shadow_texture = m_cascaded_shadow_map->GetTexture();
    filters[SSDS].m_shadow_sampler = m_cascaded_shadow_map->GetSampler();
    filters[SSDS].m_shadow_distances = m_cascaded_shadow_map->GetFrustumDistances();
    filters[SSDS].m_shadow_matrices = m_cascaded_shadow_map->GetShadowMatrices();
    filters[SSDS].Render(STATISTIC_SAMPLES);
    ProfilerEndPass();

//Blur SSAO and shadow to full-size target in a single pass   
    ProfilerBeginPass("AOShadowBlur");
    m_ao_shadow_blur->Render(m_fullscreen_quad_vao, m_active_camera, filters[SSDS].m_color_texture, m_gbuffer->m_depth_texture);
    ProfilerEndPass();

//Combine G-buffer with Lighting
    ProfilerBeginPass("LightCombine");
    filters[LIGHT_COMBINE].m_active_camera = m_active_camera ;
    filters[LIGHT_COMBINE].m_input_textures[0] = m_gbuffer->m_albedo_map;
    filters[LIGHT_COMBINE].m_input_textures[1] = m_gbuffer->m_normal_map;
    filters[LIGHT_COMBINE].m_input_textures[2] = m_gbuffer->m_params_map;
    filters[LIGHT_COMBINE].m_input_textures[3] = m_gbuffer->m_depth_texture;
    filters[LIGHT_COMBINE].m_input_textures[4] = m_ao_shadow_blur->m_color_texture;
    filters[LIGHT_COMBINE].m_input_textures[7] = m_topdown_shadow ? m_topdown_shadow->textureObject() : 0; //foliage lighting needs to read this, as this map contains no self-shadow
    filters[LIGHT_COMBINE].m_input_samplers[7] = m_topdown_shadow_sampler ? m_topdown_shadow_sampler : 0;
    filters[LIGHT_COMBINE].m_static_cubemaps_object = m_static_cubemaps ? m_static_cubemaps->textureObject() : 0;
    filters[LIGHT_COMBINE].m_dynamic_cubemap_objects[0] = m_dynamic_cubemaps[0]->GetId();
    filters[LIGHT_COMBINE].m_dynamic_cubemap_objects[1] = m_dynamic_cubemaps[1]->GetId();
#if DEBUG_SHADOW
    filters[LIGHT_COMBINE].m_shadow_matrices = m_cascaded_shadow_map->GetShadowMatrices();
#endif
    filters[LIGHT_COMBINE].Render(STATISTIC_SAMPLES);
    ProfilerEndPass();

//Add sky to LIGHT_COMBINE target
    CollectRenderMaterials(m_sky_mesh, m_sky_lightcombine_pass, PassType::REFLECTION);
    Render(m_active_camera, m_sky_mesh, m_sky_lightcombine_pass, 0, 0, 0, PassType::REFLECTION, false); //pass type == 1, so it will render simple color
    
    ProfilerBeginPass("Transparent");
#if !ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET
    m_gbuffer->BindTransparentAccumBuffer();
#endif

    Render(m_active_camera, m_visible_meshes[1], 0, 0, 0, 0, PassType::NORMAL, m_occlusion_cull_enabled && m_cull_non_instanced_meshes_enabled); //HACK HACK HACK - needs a separate pass & shader to render into light combine
    ProfilerEndPass();

    // Handle camera changes
    KCL::uint32 adaptation_mode = m_adaptation_mode;
    if (adaptation_mode == ComputeReduction::ADAPTATION_ENABLED && (m_camera_clip_changed || m_ubo_frame.m_ubo.time_dt_pad2.y <= 0.0f))
    {
        // Turn off the adaptation for the current frame
        adaptation_mode = ComputeReduction::ADAPTATION_DISABLED;
    }

    // Set the luminance adaption mode
    m_compute_hdr->GetComputeReduction()->SetAdaptationMode(adaptation_mode);

    // Disable motion blur on camera changes
    const bool enable_motion_blur = m_prev_mvp_valid  && m_mblur_enabled;

    //final pass
    glDrawBuffers( 1, m_gbuffer->m_draw_buffers);

    ProfilerBeginPass("SimulateParticles");
    SimulateParticles();
    ProfilerEndPass();

    ProfilerBeginPass("ComputeHDR");
    m_compute_hdr->SetInputTexture(filters[LIGHT_COMBINE].m_color_texture);
    m_compute_hdr->Execute();
    ProfilerEndPass();

    ProfilerBeginPass("ToneMap+Bloom");
    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, m_compute_hdr->GetLuminanceBuffer());
    filters[HDR_FINAL_PASS].m_input_textures[0] = filters[LIGHT_COMBINE].m_color_texture;
    filters[HDR_FINAL_PASS].m_input_textures[2] = m_compute_hdr->GetBloomTexture();
    filters[HDR_FINAL_PASS].m_input_samplers[2] = m_compute_hdr->GetBloomSampler();
#if !ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET
    filters[HDR_FINAL_PASS].m_input_textures[3] = m_gbuffer->m_transparent_accum_map;
#endif
    filters[HDR_FINAL_PASS].Render(STATISTIC_SAMPLES);
    glBindBufferBase(GL_UNIFORM_BUFFER, LUMINANCE_BINDING_SLOT, 0);
    ProfilerEndPass();

    ProfilerBeginPass("RenderParticles");
    RenderParticles();
    ProfilerEndPass();

//#define DEBUG_RENDERTARGETS
#ifdef DEBUG_RENDERTARGETS
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_gbuffer->m_depth_texture, 0);
    m_cascaded_shadow_map->DebugRender(m_active_camera, m_dynamic_cubemaps[1]->GetId());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 0, 0);
#endif

    if (enable_motion_blur)
    {
        ProfilerBeginPass("mb-comp");
        m_compute_motion_blur->Execute(m_gbuffer->m_velocity_buffer);
        ProfilerEndPass();

        //combine lighting here
        ProfilerBeginPass("mb-blur");
        filters[MOTION_BLUR].m_shader = m_compute_motion_blur->GetBlurShader();
        filters[MOTION_BLUR].m_input_textures[0] = filters[HDR_FINAL_PASS].m_color_texture;
        filters[MOTION_BLUR].m_input_textures[1] = m_gbuffer->m_depth_texture;
        filters[MOTION_BLUR].m_input_textures[4] = m_gbuffer->m_velocity_buffer;
        filters[MOTION_BLUR].m_input_textures[6] = m_compute_motion_blur->GetNeighborMaxTexture();
        filters[MOTION_BLUR].m_active_camera = m_active_camera;
        filters[MOTION_BLUR].Render(STATISTIC_SAMPLES);
        ProfilerEndPass();
    }

    if (m_depth_of_field_enabled)
    {
        KCL::uint32 dof_input_texture;
        if (enable_motion_blur)
        {
            dof_input_texture = filters[MOTION_BLUR].m_color_texture;
        }
        else
        {
            dof_input_texture = filters[HDR_FINAL_PASS].m_color_texture;
        }

        ProfilerBeginPass("DOF-blur");
        m_dof_blur->SetInputTexture(dof_input_texture);
        m_dof_blur->Execute();
        ProfilerEndPass();

        ProfilerBeginPass("DOF-pp");
        filters[DOF].m_active_camera = m_active_camera;
        filters[DOF].m_focus_distance = m_camera_focus_distance;
        filters[DOF].m_input_textures[0] = dof_input_texture;
        filters[DOF].m_input_textures[1] = m_dof_blur->GetOutputTexture();
        filters[DOF].m_input_textures[2] = m_gbuffer->m_depth_texture;
        filters[DOF].Render(STATISTIC_SAMPLES);
        ProfilerEndPass();
    }

    //
    //  Lensflare
    //
    ProfilerBeginPass("Lensflare");
    m_lensflare->m_color_texture = -1;
    m_lensflare->m_depth_texture = m_gbuffer->m_depth_texture;
    m_lensflare->m_camera = m_active_camera;
    m_lensflare->Execute();
    m_lensflare->Render();
    ProfilerEndPass();

    GLB::OpenGLStateManager::GlDepthMask( 1);
    for(int i=0; i<16; ++i)
    {
        glBindSampler(i, 0);
    }

//#if defined GL_FRAMEBUFFER_SRGB
//  if (m_framebuffer_gamma_correction)
//      glDisable(GL_FRAMEBUFFER_SRGB) ;
//#endif

/*    //deferred decal under the car to fake AO
    for(int i=0;i<m_CarActors.size();++i)
    {
        Shader *s = m_AOdecal_shader;

        glUseProgram( s->m_p);

        glUniformMatrix4fv( s->m_uniform_locations[GLB::uniforms::mvp], 1, 0, m_CarActors[i]->m_root->m_translation.v);

        glBindVertexArray( m_logo_vao); //just for drawing a quad

        glActiveTexture( GL_TEXTURE0);
        glBindTexture( GL_TEXTURE_2D, m_AOdecal->getId());
        glUniform1i( s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);

        //scene depth
        glActiveTexture( GL_TEXTURE0);
        glBindTexture( GL_TEXTURE_2D, m_AOdecal->getId());
        glUniform1i( s->m_uniform_locations[GLB::uniforms::texture_unit1], 1);

        glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
    }
*/
    if(0) // Note: logo disabled here
    {
        KCL::Matrix4x4 m;

        KCL::Matrix4x4::Ortho( m, 0, m_viewport_width, m_viewport_height, 0, -1, 1);

        m.translate( KCL::Vector3D( m_viewport_width - m_logo->getWidth() - 16, m_viewport_height - m_logo->getHeight() - 16, 0));

        GLB::GLBShader2 *s = m_logo_shader;

        glUseProgram( s->m_p);

        glUniformMatrix4fv(s->m_uniform_locations[GLB::uniforms::mvp], 1, 0, m.v);

        glBindVertexArray( m_logo_vao);

        GLB::OpenGLStateManager::GlActiveTexture( GL_TEXTURE0);
        glBindTexture( GL_TEXTURE_2D, m_logo->textureObject());
        glUniform1i(s->m_uniform_locations[GLB::uniforms::texture_unit0], 0);

        glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);
    }

#ifndef GL_WRAPPER_STATS
    m_gui->RenderGUI();
#endif

    m_ubo_manager->EndFrame();

    ProfilerEndFrame();
}

void GLB_Scene4::UpdateGUI(KCL::uint32 cursorX, KCL::uint32 cursorY, bool mouseLPressed, bool mouseLClicked, bool mouseRPressed, bool mouseRClicked, const bool *downKeys)
{
    m_gui->UpdateGUI(cursorX, cursorY, mouseLPressed, mouseLClicked, mouseRPressed, mouseRClicked, downKeys);
}

static const Vector3D refs[6] =
{
    Vector3D(1, 0, 0),
    Vector3D(-1, 0, 0),
    Vector3D(0, 1, 0),
    Vector3D(0, -1, 0),
    Vector3D(0, 0, 1),
    Vector3D(0, 0, -1)
};
static const Vector3D ups[6] =
{
    Vector3D(0, -1, 0),
    Vector3D(0, -1, 0),
    Vector3D(0, 0, 1),
    Vector3D(0, 0, -1),
    Vector3D(0, -1, 0),
    Vector3D(0, -1, 0)
};

static const Vector3D refsParab[2] =
{
    Vector3D(0, 1, 0),
    Vector3D(0, 1, 0)
//  Vector3D(0, 0, 1),
//  Vector3D(0, 0, 1)
};
static const Vector3D upsParab[2] =
{
    Vector3D(0, 0, 1),
    Vector3D(0, 0, 1)
//  Vector3D(0, 1, 0),
//  Vector3D(0, 1, 0)
};

extern float multi;

void GLB_Scene4::UpdateEnvmap( const GLB::Vector3D &pos, KCL::uint32 idx)
{
    std::vector<Mesh*> visible_meshes[2];
    std::vector<KCL::PlanarMap*> visible_planar_maps;
    std::vector<KCL::Mesh*> meshes_to_blur;
    std::vector< std::vector<KCL::Mesh*> > visible_instances;

    m_fboEnvMap->Bind();
    
    glViewport( 0, 0, m_fboEnvMap->GetSize(), m_fboEnvMap->GetSize());

    static const GLenum invalid_attachments[1] = { GL_DEPTH_ATTACHMENT };

    for(size_t i = 0; i < 2; ++i)
    {
        Vector2D nearfar;

        m_fboEnvMap->AttachParaboloid( m_dynamic_cubemaps[idx], i);

        m_dpcam.Ortho(-1000.0, 1000.0, -1000.0, 1000.0, 0.01f, 1000.0f);
        m_dpcam.LookAt( pos, pos+refsParab[i], upsParab[i]);
        m_dpcam.Update();

        visible_meshes[0].clear();
        visible_meshes[1].clear();

        std::vector<Actor*> exclude_list;
        exclude_list.push_back(m_carActor_hero);
        exclude_list.push_back(m_carActor_evil);

        m_paraboloid_culling->m_force_cast = m_force_cast_reflections;
        FrustumCull(&m_dpcam, visible_planar_maps, visible_meshes, visible_instances, meshes_to_blur, m_pvs, nearfar, 0, true, true, &exclude_list, m_paraboloid_culling);
        glClearColor( 0.0, 0.0, 0.0, 0.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        m_dpcam.Ortho(-1000.0, 1000.0, -1000.0, 1000.0, nearfar.x, nearfar.y);
        m_dpcam.LookAt( pos, pos+refsParab[i], upsParab[i]);
        m_dpcam.Update();

        GLB_Scene4Tools::SortMeshes(visible_meshes[0], &m_dpcam, false);
        Render( &m_dpcam, visible_meshes[0], 0, 0, 0, 0, PassType::REFLECTION, false, i+1); //0,1, or 2: 0 means not using paraboloid proj.

        {
            std::vector<GLB::Mesh3::InstanceData> instance_data;
            std::vector<KCL::Mesh*> instances;

            CollectInstances( instance_data, instances, visible_instances);

            m_instance_manager->UploadInstanceData( instance_data);

            Render( &m_dpcam, instances, 0 , 0, 0, 0, PassType::REFLECTION, false, i+1);
        }

        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalid_attachments[0]);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_dynamic_cubemaps[idx]->GetId());
    glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    m_fboEnvMap->Unbind();
    glViewport( 0, 0, m_viewport_width, m_viewport_height);
}

/*

CubeEnvMap* GLB_Scene4::CreateEnvmap( const KCL::Vector3D &pos, KCL::uint32 idx)
{
    std::vector<Mesh*> visible_meshes[2];
    std::vector<KCL::PlanarMap*> visible_planar_maps;
    std::vector<KCL::Mesh*> meshes_to_blur;
    std::vector< std::vector<KCL::Mesh*> > visible_instances;
    uint32 size = 512;

    if( !m_fboEnvMap)
    {
        m_fboEnvMap = new FboEnvMap( size);
    }

    CubeEnvMap* cem = m_fboEnvMap->CreateCubeEnvMapWithFormat(GL_RGBA16F);

    cem->SetPosition( pos);
    
    Camera2 camera;

    m_fboEnvMap->Bind();
    
    glViewport( 0, 0, m_fboEnvMap->GetSize(), m_fboEnvMap->GetSize());

    for(size_t i = 0; i < 6; ++i)
    {
        Vector2D nearfar;

        m_fboEnvMap->AttachCubemap( cem, i);

        camera.Perspective(90.0f, 1, 1, 0.01f, 1000.0f);
        camera.LookAt( pos, pos+refs[i], ups[i]);
        camera.Update();

        visible_meshes[0].clear();
        visible_meshes[1].clear();

        FrustumCull(&camera, visible_planar_maps, visible_meshes, visible_instances, meshes_to_blur, m_pvs, nearfar, 0, true, false);

        glClearColor( m_background_color.x, m_background_color.y, m_background_color.z, 1.0f);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        camera.Perspective(90.0f, 1, 1, nearfar.x, nearfar.y);
        camera.LookAt( pos, pos+refs[i], ups[i]);
        camera.Update();

        Render( &camera, visible_meshes[0], 0, 0, 0, 0, PassType::REFLECTION, false);
        {
            std::vector<GLB::Mesh3::InstanceData> instance_data;
            std::vector<KCL::Mesh*> instances;

            CollectInstances( instance_data, instances, visible_instances);

            m_instance_manager->UploadInstanceData( instance_data);

            Render( &camera, instances, 0 , 0, 0, 0, PassType::REFLECTION, false);
        }
    }

    glBindTexture(GL_TEXTURE_CUBE_MAP, cem->GetId());
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

#if 0
    float diffuse_colors[6][4];

    if( diffuse_colors)
    {
        uint32 lod1x1 = 0;
        for( ;;lod1x1++)
        {
            int size;
            glGetTexLevelParameteriv( GL_TEXTURE_CUBE_MAP_POSITIVE_X, lod1x1, GL_TEXTURE_WIDTH, &size);
            if( size == 1)
            {
                break;
            }
        }
        for( uint32 i=0; i<6; i++)
        {
            glGetTexImage( GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, lod1x1, GL_RGBA, GL_FLOAT, &diffuse_colors[i]);
        }
    }
#endif

#if 0
    {
        uint32 num_samples = size * size;
    
        KCL::Vector4D *samples4f = new KCL::Vector4D[num_samples];
        uint8 *samples4ui = new uint8[num_samples*4];

        for( uint32 face=0; face<6; face++)
        {
            char filename[256];

            sprintf(filename, "%senvmap%03d_%x.png", "", idx, GL_TEXTURE_CUBE_MAP_POSITIVE_X + face);

            glGetTexImage( GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA, GL_FLOAT, samples4f);

            for( uint32 i=0; i<num_samples; i++)
            {
                void float2rgbe(unsigned char rgbe[4], float red, float green, float blue);

                float2rgbe( &samples4ui[i*4], samples4f[i].x, samples4f[i].y, samples4f[i].z);
            }


            KCL::Image::savePng( filename, size, size, samples4ui, KCL::Image_RGBA8888, false);
        }

        delete [] samples4f;

        delete [] samples4ui;
    }
#endif

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    m_fboEnvMap->Unbind();
    glViewport( 0, 0, m_viewport_width, m_viewport_height);

    return cem;
}
*/

KCL::int32 GLB_Scene4::GetCameraClipId(KCL::int32 time)
{
    if (time < 0)
    {
        return -1;
    }

    KCL::Vector4D r;
    float time_seconds = time * m_animation_multiplier * 0.001f;
    float time_base = 0.0f;

    KCL::_key_node::Get(r, m_camera_cut_track, time_seconds, time_base);
    return KCL::int32(r.x);
}

static void float2rgbe(unsigned char rgbe[4], float red, float green, float blue)
{
  float v;
  int e;

  v = red;
  if (green > v) v = green;
  if (blue > v) v = blue;
  if (v < 1e-32) {
    rgbe[0] = rgbe[1] = rgbe[2] = rgbe[3] = 0;
  }
  else {
    v = frexp(v,&e) * 256.0/v;
    rgbe[0] = (unsigned char) (red * v);
    rgbe[1] = (unsigned char) (green * v);
    rgbe[2] = (unsigned char) (blue * v);
    rgbe[3] = (unsigned char) (e + 128);
  }
}

static float saturate(float val) 
{
    return std::min(std::max(val, 0.0f), 1.0f);
}

static void float2rgbm(unsigned char rgbm[4], float red, float green, float blue)
{
    KCL::Vector3D color(sqrtf(red), sqrtf(green), sqrtf(blue)); //linear to "gamma"
    
    color *= 1.0f / 6.0f;

    float alpha = saturate( std::max<float>( std::max<float>( red, green ), std::max<float>( blue, 1e-6f ) ) );
    
    alpha = ceil( alpha * 255.0f ) / 255.0f;
    
    color *= 1.0f / alpha;
   
    rgbm[0] = (unsigned char) (255.0f * saturate(color.x));
    rgbm[1] = (unsigned char) (255.0f * saturate(color.y));
    rgbm[2] = (unsigned char) (255.0f * saturate(color.z));

    rgbm[3] = (unsigned char) (255.0f * saturate(alpha));
}

//NOTES TO CAPTURE BAKED ENVMAPS AND AMBIENT COLORS:
//  - output dir is project folder, format is RGBE HDR
//  - scene4/envmaps.txt lists the world-space locations
//  - simplest way to get 512x512 cubes is to run the entire benchmark in 512x512 window
//  - filters[LIGHT_COMBINE].m_color_texture needs mipmapping
//  - all envmap reads (textureLod call in common.h/PBRhelper function) need to return vec4(0.5) or vec3(0.5)
//  - change the ENABLE_LIGHTCOMBINE_FP_RENDER_TARGET define to 1

void GLB_Scene4::CaptureEnvmap( const KCL::Vector3D &pos, KCL::uint32 idx)
{    
    std::vector<Mesh*> visible_meshes[2];
    std::vector<KCL::PlanarMap*> visible_planar_maps;
    std::vector<KCL::Mesh*> meshes_to_blur;
    std::vector< std::vector<KCL::Mesh*> > visible_instances;
    uint32 size = 512;
    
    Camera2 camera;
    char filename[256];

    sprintf(filename, "cubemap_colors.txt");

    FILE *cubemap_colors_file = fopen( (KCL::File::GetDataPath() + std::string(filename)).c_str(), "ab");

    std::string cubemap_name = "new_capture";
    if(m_envmap_descriptors.size() > idx)
    {
        cubemap_name = m_envmap_descriptors[idx].m_name;
    }

    if( cubemap_colors_file)
    {
        fprintf( cubemap_colors_file, "%s ", cubemap_name.c_str());
    }
    
    for(size_t i = 0; i < 6; ++i)
    {
        Vector2D nearfar;

        camera.Perspective(90.0f, 1, 1, 0.01f, 1000.0f);
        camera.LookAt( pos, pos+refs[i], ups[i]);
        camera.Update();

        visible_meshes[0].clear();
        visible_meshes[1].clear();

        if(idx != 0)
        {
            FrustumCull(&camera, visible_planar_maps, visible_meshes, visible_instances, meshes_to_blur, m_pvs, nearfar, 0, true, false);
        }
        else //first cube is the default, and it only contains the sky, and will serve as generic env
        {
            for( uint32 i=0; i<m_sky_mesh.size(); i++)
            {
                visible_meshes[0].push_back( m_sky_mesh[i]);
            }
        }

        camera.Perspective(90.0f, 1, 1, nearfar.x, nearfar.y);
        camera.LookAt( pos, pos+refs[i], ups[i]);
        camera.Update();

        m_active_camera = &camera;
        m_visible_meshes[0] = visible_meshes[0];
        m_visible_instances = visible_instances;

        Render();
    
        {
            glBindFramebuffer(GL_FRAMEBUFFER, filters[LIGHT_COMBINE].GetFramebufferObject(0));
            
            uint32 num_samples = size * size;
    
            KCL::Vector4D *samples4f = new KCL::Vector4D[num_samples];
            KCL::Vector4D *samples4f_flipped = new KCL::Vector4D[num_samples];
            uint8 *samples4ui = new uint8[num_samples*4];

            std::string fname;
            std::string fname_raw;
            char filename[256];

            sprintf(filename, "%senvmap%03d_%x", "", idx, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i);
            fname = std::string(filename) + ".png";
            fname_raw = std::string(filename) + "_ldr.png";

            glReadPixels( 0, 0, size, size, GL_RGBA, GL_FLOAT, samples4f);

            //flip horizontally to get correct image
            for(int i=0; i<num_samples; ++i)
            {
                //int rowidx = i % size;
                //int colidx = i - rowidx * size;
                samples4f_flipped[i] = samples4f[i];
            }

            for( uint32 i=0; i<num_samples; i++)
            {
                //float2rgbe( &samples4ui[i*4], samples4f_flipped[i].x, samples4f_flipped[i].y, samples4f_flipped[i].z);
                float2rgbm( &samples4ui[i*4], samples4f_flipped[i].x, samples4f_flipped[i].y, samples4f_flipped[i].z);
            }

            if(1) //dump image to float - some image editors can open them as raw data if their size is provided on open
            {
                KCL::uint8* samples3uiLDR = new uint8[num_samples*3];
                for(int i=0; i<num_samples;++i)
                {
                    samples3uiLDR[i*3] = std::min<int>(255, samples4f_flipped[i].x * 255.0);
                    samples3uiLDR[i*3+1] = std::min<int>(255, samples4f_flipped[i].y * 255.0);
                    samples3uiLDR[i*3+2] = std::min<int>(255, samples4f_flipped[i].z * 255.0);
                }

/*              KCL::AssetFile file(fname_raw, KCL::Write, KCL::RDir);
                if(!file.GetLastError())
                {
                    file.Write(samples3uiLDR, sizeof(uint8), num_samples*3);
                    file.Close(); //dtor would close it anyway
                }
*/
                KCL::Image::savePng( (KCL::File::GetDataPath() + fname_raw).c_str(), size, size, samples3uiLDR, KCL::Image_RGB888, false);

                delete [] samples3uiLDR;
            }

            KCL::Image::savePng( (KCL::File::GetDataPath() + fname).c_str(), size, size, samples4ui, KCL::Image_RGBA8888, false);

            delete [] samples4f;
            delete [] samples4f_flipped;

            delete [] samples4ui;
        }


#ifdef HAVE_GLEW
        {
            //TODO: this requires mipmapped filters[LIGHT_COMBINE].m_color_texture
            glBindTexture( GL_TEXTURE_2D, filters[LIGHT_COMBINE].m_color_texture);

            glGenerateMipmap( GL_TEXTURE_2D);

            uint32 lod1x1 = 0;
            for( ;;lod1x1++)
            {
                int size;
                glGetTexLevelParameteriv( GL_TEXTURE_2D, lod1x1, GL_TEXTURE_WIDTH, &size);
                if( size <= 1)
                {
                    break;
                }
            }

            float ambient_color[4];

            glGetTexImage( GL_TEXTURE_2D, lod1x1, GL_RGBA, GL_FLOAT, ambient_color);

            if( cubemap_colors_file)
            {
                fprintf( cubemap_colors_file, "%f %f %f ", ambient_color[0] * 2.0f, ambient_color[1] * 2.0f, ambient_color[2] * 2.0f); //*2 to compensate mipmap darkening
            }
        }
#endif
    }

    if( cubemap_colors_file)
    {
        fprintf( cubemap_colors_file, "\n");
        fclose( cubemap_colors_file);
    }
}

KCL::uint32 GLB_Scene4::GetVelocityBuffer() const
{
    return m_gbuffer->m_velocity_buffer;
}

GLB::ComputeMotionBlur *GLB_Scene4::GetComputeMotionBlur() const
{
    return m_compute_motion_blur;
}

GLB::InstanceManager<GLB::Mesh3::InstanceData> *GLB_Scene4::GetInstanceManager() const
{
    return m_instance_manager;
}

KCL::Texture* GLB_Scene4::GetTopdownShadowMap() const
{
    return m_topdown_shadow;
}

KCL::uint32 GLB_Scene4::GetTopdownShadowSampler() const
{
    return m_topdown_shadow_sampler;
}

#endif

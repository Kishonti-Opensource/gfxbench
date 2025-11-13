/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#import "mtl_scene_27.h"

#include <kcl_base.h>
#include "mtl_factories.h"
#include "mtl_shadowmap.h"
#include "mtl_cubemap.h"
#include "mtl_planarmap.h"
#include "mtl_material.h"
#include "mtl_mesh.h"
#include "fbo.h"
#include "fbo_metal.h"

#include "mtl_shader_constant_layouts_27.h"

using namespace KCL ;
using namespace MetalRender ;

#define NORMAL_MESH  0
#define SKINNED_MESH 1

#define DELETE_UNUSED_ATTRIBS 0

KRL_Scene* MetalRender::CreateMTLScene27(const GlobalTestEnvironment* const gte)
{
    return new MTL_Scene_27(gte);
}


MTL_Scene_27::MTL_Scene_27(const GlobalTestEnvironment* const gte) : m_gte(gte),
    m_motionBlurPipeLine(nullptr)
{
    m_Context = dynamic_cast<MetalGraphicsContext*>(gte->GetGraphicsContext()) ;

    m_full_screen_quad = nullptr ;

    m_dynamicDataBuffer = nullptr ;
    m_dynamicDataBufferPool = nullptr ;

    m_pseudoIndicesBuffer = nullptr ;
    m_pseudoIndicesBufferPool = nullptr ;

    m_motionBlurPipeLine = nullptr ;

    m_CommandQueue = nil ;
    m_quadBufferVertexLayout = nil ;

    m_frameBuffer27 = nullptr ;

    m_lock_id = 0 ;
}

MTL_Scene_27::~MTL_Scene_27()
{
    id <MTLCommandBuffer> finishBuffer = [m_CommandQueue commandBuffer];
    [finishBuffer commit] ;
    [finishBuffer waitUntilCompleted] ;
    releaseObj(finishBuffer) ;

    delete m_pseudoIndicesBufferPool ;
    delete m_blur_shader ;
    releaseObj(m_CommandQueue) ;

    delete m_frameBuffer27 ;

    delete m_full_screen_quad ;
    delete m_dynamicDataBufferPool ;
    releaseObj(m_quadBufferVertexLayout);

    MetalRender::Pipeline::ClearCashes() ;
}


KCL::KCL_Status MTL_Scene_27::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
#if !TARGET_OS_IPHONE
    m_force_highp = true;
#endif

    // metal port doesn't support antialiasing now
    assert(samples == 0) ;

    m_dynamicDataBufferPool = new DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
    MetalRender::Mesh3::InitVertexLayouts(KCL::SV_27) ;
    m_quadBufferVertexLayout = MetalRender::QuadBuffer::GetVertexLayout() ;


    m_dynamicDataBuffer = m_dynamicDataBufferPool->GetNewBuffer(512*1024) ;
    m_CommandQueue = m_Context->getMainCommandQueue() ;

#if TARGET_OS_IPHONE
    const unsigned int INDEX_BUFFER_COUNT = 2 ; // double buffering for intanced index buffers
    m_pseudoIndicesBufferPool = new DynamicDataBufferPool(INDEX_BUFFER_COUNT) ;
#else
    m_pseudoIndicesBufferPool = new DynamicDataBufferPool(MetalRender::METAL_MAX_FRAME_LAG) ;
#endif
    m_pseudoIndicesBuffer = m_pseudoIndicesBufferPool->GetNewBuffer(1024*1024) ;


    m_frameBuffer27 = new Framebuffer27(m_viewport_width,m_viewport_height,m_mblur_enabled) ;

    m_planar_map_shader_type = Pipeline::PixelFormatToShaderType(PlanarMap::PLANAR_FRAME_BUFFER_FORMAT) ;

    if (m_mblur_enabled)
    {
        m_default_shader_type = Pipeline::PixelFormatToShaderType(Framebuffer27::MAIN_FRAME_BUFFER_FORMAT) ;
    }
    else
    {
        // hardware layer format
        m_default_shader_type = kShaderTypeSingleBGRA8 ;
    }

    m_requied_shader_types.insert(m_planar_map_shader_type) ;
    m_requied_shader_types.insert(m_default_shader_type) ;

    IncrementProgressTo(0.5f);

    KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;

    // In metal there is no shader precision
    //Shader::SetForceHIGHP(m_force_highp);
    IncrementProgressTo(0.51f);


    //Shader::InitShaders( required_render_api, true);
    MetalRender::Pipeline::InitShaders(m_scene_version);
    IncrementProgressTo(0.52f);


    IncrementProgressTo(0.53f);


    m_full_screen_quad = new MetalRender::QuadBuffer(QuadBuffer::kBlitQuadLandscape) ;
    IncrementProgressTo(0.54f);


    for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
    {
        m_global_shadowmaps[i] = new ShadowMap(m_fboShadowMap_size, m_fboShadowMap_size,m_shadow_method_str);
    }
    IncrementProgressTo(0.55f);


    if( m_do_instancing)
    {
        std::map<KCL::Mesh3*, KCL::MeshInstanceOwner*> mios_map;

        IncrementProgressTo(0.56f);
        for( uint32 i = 0; i < m_rooms.size(); i++)
        {
            XRoom *room = m_rooms[i];

            for( uint32 j = 0; j < room->m_meshes.size(); j++)
            {
                bool to_be_instanced = false;
                KCL::Mesh3* m = room->m_meshes[j]->m_mesh;

                if( m->m_name.find( "jungle_01_Trans_long_instShape1") != std::string::npos)
                {
                    to_be_instanced = true;
                }
                if( m->m_name.find( "jungle_01_Trans_palm_inst1Shape") != std::string::npos)
                {
                    to_be_instanced = true;
                }
                if( m->m_name.find( "jungle_leaves_instShape") != std::string::npos)
                {
                    to_be_instanced = true;
                }
                if( m->m_name.find( "trex_foot_decal") != std::string::npos)
                {
                    to_be_instanced = true;
                }


                if( to_be_instanced)
                {
                    KCL::MeshInstanceOwner *mio;
                    std::map<KCL::Mesh3*, KCL::MeshInstanceOwner*>::iterator f = mios_map.find( m);

                    if( f == mios_map.end())
                    {
                        mio = new KCL::MeshInstanceOwner;
                        mio->m_mesh = GetMeshFactory().Create("instance_owner", 0, 0);
                        mio->m_mesh->m_material = room->m_meshes[j]->m_material;
                        mio->m_mesh->m_mesh = m;

                        MetalRender::Mesh3 *mtl_mesh = dynamic_cast<MetalRender::Mesh3 *>(m) ;
                        assert(mtl_mesh) ;

                        if (mtl_mesh)
                        {
                            mtl_mesh->m_indices_from_mio = true ;
                        }

                        mios_map[m] = mio;
                        m_mios.push_back(mio);
                    }
                    else
                    {
                        mio = f->second;
                    }

                    room->m_meshes[j]->m_user_data = mio;
                    room->m_meshes[j]->m_mesh = 0;
                    mio->m_instances.push_back( room->m_meshes[j]);
                }
#if DELETE_UNUSED_ATTRIBS
                if( room->m_meshes[j])
                {
                    room->m_meshes[j]->DeleteUnusedAttribs();
                }
#endif
            }
        }

        IncrementProgressTo(0.57f);

        std::vector<KCL::MeshInstanceOwner*>::iterator mio = m_mios.begin();

        int counter = 0;
        while( mio != m_mios.end())
        {
            std::vector<KCL::Mesh*> &instances = (*mio)->m_instances;

            std::string newName;
            std::stringstream tmp;
            tmp << ImagesDirectory();
            tmp << "mio_lightmaps_";
            tmp << counter++;
            tmp << ".png";
            newName = tmp.str().c_str();

#define COMBINED_LIGHTMAP_GENERATOR_LOADER
#ifdef COMBINED_LIGHTMAP_GENERATOR_LOADER
            KCL::Texture* texture = TextureFactory().CreateAndSetup( KCL::Texture_2D,  newName.c_str() );

            if( !texture)
            {
                // TODO rewrite KCL::Texture refactoring
                assert(0);

            }
#endif

            for(unsigned int k = 0 ;k < instances.size();k++)
            {
                KCL::Texture* tex = instances[k]->m_material->m_textures[1];
                instances[k]->m_material->m_textures[1] = 0;
                for (KCL::uint32 i=0;i<m_textures.size();i++)
                {
                    if (tex==m_textures[i])
                        m_textures[i] = 0;
                }
                delete tex;
            }

            delete texture;

            texture = TextureFactory().CreateAndSetup( KCL::Texture_2D,  newName.c_str() );
            texture->commit();

            m_textures.push_back(texture);

            (*mio)->m_mesh->m_material->m_textures[1] = texture;

            (*mio)->Instance();
#if DELETE_UNUSED_ATTRIBS
            (*mio)->m_mesh->DeleteUnusedAttribs();
#endif
            mio++;
        }
        IncrementProgressTo(0.58f);

    }
    else
    {
        IncrementProgressTo(0.59f);
#if DELETE_UNUSED_ATTRIBS
        for( uint32 i=0; i<m_rooms.size(); i++)
        {
            XRoom *room = m_rooms[i];
            for( uint32 j=0; j<room->m_meshes.size(); j++)
            {
                room->m_meshes[j]->DeleteUnusedAttribs();
            }
        }
#endif
    }

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

    IncrementProgressTo(0.6f);

    for( uint32 i = 0; i < m_actors.size(); i++)
    {
        Actor *actor = m_actors[i];

        for( uint32 i = 0; i < actor->m_meshes.size(); i++)
        {
            Mesh* m = actor->m_meshes[i];
#if DELETE_UNUSED_ATTRIBS
            m->DeleteUnusedAttribs();
#endif

            if( actor->m_name.find( "decal") == std::string::npos)
            {
                m->m_is_motion_blurred = true;
            }
        }

        for( KCL::uint32 j = 0; j < actor->m_emitters.size(); j++)
        {
            KCL::AnimatedEmitter *emitter = static_cast<KCL::AnimatedEmitter*>(actor->m_emitters[j]);

            for( KCL::uint32 k = 0; k < MAX_MESH_PER_EMITTER; k++)
            {
                emitter->m_meshes[k].m_mesh = m_particle_geometry;

                assert(emitter->m_meshes[k].m_owner == nullptr);

                emitter->m_meshes[k].m_owner = emitter ;


                //TODO: az emitter typehoz tartozo materialokat lekezelni
                switch( emitter->m_emitter_type)
                {
                    case 3:
                    {
                        emitter->m_meshes[k].m_color.set( .1f, .105f, .12f);
                        emitter->m_meshes[k].m_material = m_steamMaterial;
                        break;
                    }
                    case 0:
                    case 1:
                    case 2:
                    default:
                    {
                        emitter->m_meshes[k].m_color.set( 0.72f, 0.55f, 0.33f);
                        emitter->m_meshes[k].m_material = m_smokeMaterial;
                        break;
                    }
                }
            }
        }

    }

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

#if DELETE_UNUSED_ATTRIBS
    for( uint32 i=0; i<m_sky_mesh.size(); i++)
    {
        m_sky_mesh[i]->DeleteUnusedAttribs();
    }
#endif

    for(size_t i = 0; i < m_meshes.size(); ++i)
    {
        if( KCL::g_os->LoadingCallback( 0) != KCL::KCL_TESTERROR_NOERROR) break;
        MetalRender::Mesh3* akbMesh = dynamic_cast<MetalRender::Mesh3*>(m_meshes[i]);
        akbMesh->InitVertexAttribs();
    }

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

    IncrementProgressTo(0.7f);
    result = reloadShaders();
    if(result != KCL::KCL_TESTERROR_NOERROR)
    {
        return result;
    }

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

    IncrementProgressTo(0.75f);

    for(size_t i = 0; i < m_materials.size(); ++i)
    {
        MetalRender::Material* material = dynamic_cast<MetalRender::Material*>(m_materials[i]);
        material->InitImages();
        if(result != KCL::KCL_TESTERROR_NOERROR)
        {
            return result;
        }

		switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
		{
		case KCL::KCL_TESTERROR_NOERROR:
			break;
		default:
			return status;
		}

    }
    IncrementProgressTo(0.8f);

    //NOTE: Manhattan uses instancing, which does not work currently with
    //      multiple cubemaps - this reduces the cubes to always select the
    //      same 2 - which will make sure image data is consistent between
    //      consequtive runs
    if(m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31)
    {
        std::vector<KCL::CubeEnvMapDescriptor> envmap_fixup;
        envmap_fixup.push_back(m_envmap_descriptors[0]);
        envmap_fixup.push_back(m_envmap_descriptors[1]);
        m_envmap_descriptors = envmap_fixup;
    }


    for( KCL::uint32 i = 0; i < m_envmap_descriptors.size(); i++)
    {
        CubeEnvMap_Metal *cem = CreateEnvMap( m_envmap_descriptors[i].m_position, i, false);
        m_cubemaps.push_back(cem);
    }


	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}


    // GeneratePVS();

#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
    //creating dummy stuff
    {

    }
#endif

    IncrementProgressTo(0.9f);

    if (m_mblur_enabled)
    {
        Pipeline::GFXPipelineDescriptor motionblurPipeLineDesc(Pipeline::DISABLED, kShaderTypeSingleBGRA8, m_quadBufferVertexLayout,false, m_force_highp) ;
        m_motionBlurPipeLine = Pipeline::CreatePipeline(m_blur_shader, motionblurPipeLineDesc, result) ;
    }

    return result;
}


void MTL_Scene_27::RenderWithCamera(id <MTLRenderCommandEncoder> renderEncoder, KCL::Camera2* camera, std::vector<KCL::Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type, MetalRender::ShaderType shader_type)
{
    MetalRender::Material *override_material = dynamic_cast<MetalRender::Material*>(_override_material);
    MetalRender::Material *last_material = NULL;
    int last_mesh_type = -1;
    KCL::uint32 texture_num_from_material = 0;

    float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

    bool velocity_buffer_pass = (_override_material == m_mblurMaterial);


    for( uint32 j=0; j<visible_meshes.size(); j++)
    {
        KCL::Mesh* sm = (KCL::Mesh*)visible_meshes[j];
        MetalRender::Mesh3 *mtl_mesh = dynamic_cast<MetalRender::Mesh3 *>(sm->m_mesh) ;

        if( !sm->m_mesh)
        {
            continue;
        }

        if (!mtl_mesh)
        {
            NSLog(@"%d mesh is not mtl mesh!",j) ;
            continue ;
        }


        int mesh_type = (sm->m_mesh->m_vertex_matrix_indices.size() == 0)?NORMAL_MESH:SKINNED_MESH ;

        MetalRender::Material *material = dynamic_cast<MetalRender::Material*>(sm->m_material);

        if( override_material)
        {
            material = override_material;
        }


        KRL_CubeEnvMap *envmaps[2];
        float envmaps_interpolator = 0.0f;
        KCL::Matrix4x4 mvp;
        KCL::Matrix4x4 mv;
        KCL::Matrix4x4 model;
        KCL::Matrix4x4 inv_model;
        Vector3D pos( sm->m_world_pom.v[12], sm->m_world_pom.v[13], sm->m_world_pom.v[14]);


        if( last_material != material || last_mesh_type != mesh_type)
        {
            material->Set(renderEncoder, pass_type, mesh_type, shader_type);

            if( last_material)
            {
                last_material->postInit();
            }

            texture_num_from_material = 0;
        }

        last_material = material;

        last_mesh_type = mesh_type;

        switch( mesh_type)
        {
            case NORMAL_MESH:
            {
                if( material->m_material_type == KCL::Material::SKY)
                {
                    mvp = sm->m_world_pom * camera->GetViewProjectionOrigo();
                }
                else
                {
                    mvp = sm->m_world_pom * camera->GetViewProjection();
                }
                mv = sm->m_world_pom * camera->GetView();
                model = sm->m_world_pom;
                inv_model = Matrix4x4::Invert4x3( sm->m_world_pom);
                break;
            }
            case SKINNED_MESH:
            {
                mvp = camera->GetViewProjection();
                mv = camera->GetView();
                model.identity();
                inv_model.identity();
                break;
            }
        }


        VertexUniforms   vu ;
        FragmentUniforms fu ;

        // glUniformMatrix4fv( s->m_uniform_locations[0], 1, GL_FALSE, mvp.v);
        // glUniformMatrix4fv( s->m_uniform_locations[1], 1, GL_FALSE, mv.v);
        // glUniformMatrix4fv( s->m_uniform_locations[4], 1, GL_FALSE, model.v);
        // glUniformMatrix4fv( s->m_uniform_locations[5], 1, GL_FALSE, inv_model.v);


        vu.mvp       = mvp ;
        vu.mv        = mv ;
        vu.model     = model ;
        vu.inv_model = inv_model ;




        Vector3D view_dir( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10]);

        // glUniform3fv( s->m_uniform_locations[6], 1, m_light_dir.v);
        // glUniform3fv( s->m_uniform_locations[7], 1, m_light_color.v);
        // glUniform3fv( s->m_uniform_locations[8], 1, view_dir.v);
        // glUniform3fv( s->m_uniform_locations[9], 1, camera->GetEye().v);
        // glUniform1f( s->m_uniform_locations[10], normalized_time);

        fu.global_light_dir   = KCL::Vector4D(m_light_dir,1.0) ;
        fu.global_light_color = KCL::Vector4D(m_light_color,1.0) ;
        fu.view_dir           = KCL::Vector4D(view_dir,1.0) ;
        vu.view_pos           = KCL::Vector4D(camera->GetEye(),1.0) ;
        vu.time               = normalized_time ;
        fu.time               = normalized_time ;


        // glUniform3fv( s->m_uniform_locations[11], 1, m_background_color.v);
        // glUniform1f( s->m_uniform_locations[12], m_fog_density);
        // glUniform1f( s->m_uniform_locations[13], material->m_diffuse_intensity);
        // glUniform1f( s->m_uniform_locations[14], material->m_specular_intensity);
        // glUniform1f( s->m_uniform_locations[15], material->m_reflect_intensity);

        fu.background_color = KCL::Vector4D(m_background_color,1.0) ;
        vu.fog_density = m_fog_density ;
        fu.diffuse_intensity = material->m_diffuse_intensity ;
        fu.specular_intensity = material->m_specular_intensity ;
        fu.reflect_intensity = material->m_reflect_intensity ;



        // glUniform1f( s->m_uniform_locations[16], material->m_specular_exponent);
        // glUniform1f( s->m_uniform_locations[17], material->m_transparency);
        // glUniform3fv( s->m_uniform_locations[20], 1, &light->m_world_pom.v[12]);
        // glUniform2f( s->m_uniform_locations[23], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
        // glUniform3fv( s->m_uniform_locations[24], 1, sm->m_color.v);

        fu.specular_exponent = material->m_specular_exponent ;
        fu.transparency      = material->m_transparency ;

        if (light)
        {
            Vector3D light_pos( light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]);
            vu.light_pos         = KCL::Vector4D(light_pos,1.0) ;
        }

        fu.inv_resolution.x = 1.0f / m_viewport_width ;
        fu.inv_resolution.y = 1.0f / m_viewport_height ;
        // MTL_TODO color ???


        //glUniform1f( s->m_uniform_locations[18], envmaps_interpolator);
        Get2Closest( pos, envmaps[0], envmaps[1], envmaps_interpolator);
        fu.envmaps_interpolator = envmaps_interpolator ;

        MetalRender::Texture* envMapTextures_0 = dynamic_cast<MetalRender::Texture*>(envmaps[0]->GetTexture()) ;
        MetalRender::Texture* envMapTextures_1 = dynamic_cast<MetalRender::Texture*>(envmaps[1]->GetTexture()) ;

        envMapTextures_0->Set(renderEncoder, ENVMAP_0_SLOT);
        envMapTextures_1->Set(renderEncoder, ENVMAP_1_SLOT);


        if( m_global_shadowmaps[0] )
        {
            //glUniformMatrix4fv( s->m_uniform_locations[21], 1, GL_FALSE, m_global_shadowmaps[0]->m_matrix.v);
            vu.shadow_matrix0 = m_global_shadowmaps[0]->m_matrix ;

            MetalRender::ShadowMap* metal_sm = dynamic_cast<MetalRender::ShadowMap*>(m_global_shadowmaps[0]) ;
            metal_sm->SetAsTexture(renderEncoder, SHADOW_TEXTURE_SLOT_0) ;
        }


        if( m_global_shadowmaps[1])
        {
            //glUniformMatrix4fv( s->m_uniform_locations[30], 1, GL_FALSE, m_global_shadowmaps[1]->m_matrix.v);
            vu.shadow_matrix1 = m_global_shadowmaps[1]->m_matrix ;

            MetalRender::ShadowMap* metal_sm = dynamic_cast<MetalRender::ShadowMap*>(m_global_shadowmaps[1]) ;
            metal_sm->SetAsTexture(renderEncoder, SHADOW_TEXTURE_SLOT_1) ;
        }


        //glUniform2fv( s->m_uniform_locations[25], 1, sm->m_material->m_uv_offset.v);
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

                KCL::_key_node::Get( r, sm->m_material->m_translate_u_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

                sm->m_material->m_uv_offset.x = -r.x;
            }
            if( sm->m_material->m_translate_v_track)
            {
                Vector4D r;

                KCL::_key_node::Get( r, sm->m_material->m_translate_v_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

                sm->m_material->m_uv_offset.y = -r.x;
            }
            sm->m_material->m_frame_when_animated = m_frame;
        }

        vu.translate_uv = sm->m_material->m_uv_offset.v ;


        //glUniform1f( s->m_uniform_locations[28], sm->m_alpha);
        fu.alpha_threshold = sm->m_alpha ;



        // Light Color not used in T-Rex
        // glUniform3fv( s->m_uniform_locations[65], 1, light->m_diffuse_color.v);



        //glUniformMatrix4fv( s->m_uniform_locations[29], 1, GL_FALSE, m_world_fit_matrix.v);
        vu.world_fit_matrix = m_world_fit_matrix ;


        // Uniform 40, planar map
        if( override_material != 0 && override_material->m_planar_map != 0)
        {
            MetalRender::PlanarMap* metal_pm = dynamic_cast<MetalRender::PlanarMap*>(override_material->m_planar_map) ;

            metal_pm->SetAsTexture(renderEncoder, PLANAR_TEXTURE_SLOT) ;
        }



        //glUniformMatrix4fv( s->m_uniform_locations[41], 1, GL_FALSE, mvp2.v);
        switch( mesh_type)
        {
            case NORMAL_MESH:
            {
                KCL::Matrix4x4 mvp2;

                mvp2 = sm->m_prev_world_pom * m_prev_vp;

                vu.mvp2 = mvp2 ;

                break;
            }
            case SKINNED_MESH:
            {
                //glUniformMatrix4fv( s->m_uniform_locations[41], 1, GL_FALSE, m_prev_vp.v);

                vu.mvp2 = m_prev_vp ;

                break;
            }
        }





        //glUniform1f( s->m_uniform_locations[43], sm->m_is_motion_blurred);
        fu.mblur_mask = sm->m_is_motion_blurred ;

        m_dynamicDataBuffer->WriteAndSetData<true, false>(renderEncoder, VERTEX_UNIFORM_DATA_INDEX, &vu, sizeof(VertexUniforms)) ;


        bool is_particle = sm->m_owner && ((sm->m_owner->m_type == EMITTER1) || (sm->m_owner->m_type == EMITTER2)) ;
        if (is_particle)
        {
            //glUniform4fv( s->m_uniform_locations[26], MAX_PARTICLE_PER_MESH, m_particle_data[sm->m_offset1].v);
            m_dynamicDataBuffer->WriteAndSetData<true, false>(renderEncoder, PARTICLE_DATA_INDEX,
                                                              m_particle_data[sm->m_offset1].v, 4*sizeof(float)*MAX_PARTICLE_PER_MESH) ;


            //glUniform4fv( s->m_uniform_locations[27], MAX_PARTICLE_PER_MESH, m_particle_color[sm->m_offset1].v) ;
            m_dynamicDataBuffer->WriteAndSetData<true, false>(renderEncoder, PARTICLE_COLOR_INDEX,
                                                              m_particle_color[sm->m_offset1].v, 4*sizeof(float)*MAX_PARTICLE_PER_MESH) ;
        }

        if (mesh_type == SKINNED_MESH)
        {
            // glUniform4fv( s->m_uniform_locations[2], 3 * sm->m_mesh->m_nodes.size(), sm->m_mesh->m_node_matrices);
            assert( 3*sm->m_mesh->m_nodes.size() <= MAX_BONES ) ;

            KCL::uint8* src = (KCL::uint8*)sm->m_mesh->m_node_matrices.data() ;

            m_dynamicDataBuffer->WriteAndSetData<true, false>(renderEncoder, SKELETAL_DATA_INDEX, src, sizeof(KCL::uint32)*4*MAX_BONES);
        }

        if ( (mesh_type == SKINNED_MESH) && velocity_buffer_pass )
        {
            //glUniform4fv( s->m_uniform_locations[42], 3 * m_max_joint_num_per_mesh, sm->m_mesh->m_prev_node_matrices);
            assert( 3 * m_max_joint_num_per_mesh <= MAX_BONES ) ;

            KCL::uint8* src = (KCL::uint8*)sm->m_mesh->m_prev_node_matrices.data() ;

            m_dynamicDataBuffer->WriteAndSetData<true, false>(renderEncoder, SKELETAL_MOTION_BLUR_DATA_INDEX, src, sizeof(KCL::uint32)*4*MAX_BONES);
        }

        m_dynamicDataBuffer->WriteAndSetData<false, true>(renderEncoder, FRAGMENT_UNIFORM_DATA_INDEX, &fu, sizeof(FragmentUniforms)) ;

        mtl_mesh->SetVertexBuffer(renderEncoder) ;

        KCL::uint32 indexCount = sm->m_primitive_count ? sm->m_primitive_count : sm->m_mesh->getIndexCount(lod);
        if (mtl_mesh->m_indices_from_mio)
        {
            assert(m_do_instancing) ;
            m_pseudoIndicesBuffer->DrawWithIndicesAtOffset(renderEncoder,mtl_mesh->m_mio_offsets[lod],indexCount,MTLIndexTypeUInt16);
        }
        else
        {
            mtl_mesh->Draw(renderEncoder, lod, 1,indexCount);
        }

    }

    if(last_material)
    {
        last_material->postInit();
    }
}


void MTL_Scene_27::RenderPlanar( KCL::PlanarMap* pm_, id<MTLCommandBuffer> commandBuffer)
{
    MetalRender::PlanarMap *pm = dynamic_cast<MetalRender::PlanarMap*>(pm_);

    id <MTLRenderCommandEncoder> renderEncoder = pm->SetAsTargetAndGetEncoder(commandBuffer);

    [renderEncoder setFrontFacingWinding:MTLWindingClockwise];

    MTLViewport viewport = { 0.0, 0.0, static_cast<double>(pm->m_width), static_cast<double>(pm->m_height), 0.0, 1.0 };

    [renderEncoder setViewport:viewport];


    if(UseZPrePass())
        RenderPrepass(renderEncoder,&pm->m_camera, pm->m_visible_meshes[0], 0, pm, 1, 0, m_planar_map_shader_type);
    else
        RenderWithCamera(renderEncoder, &pm->m_camera, pm->m_visible_meshes[0], 0, pm, 1, 0, 0, m_planar_map_shader_type);

    RenderWithCamera(renderEncoder, &pm->m_camera, pm->m_visible_meshes[1], 0, pm, 1, 0, 0, m_planar_map_shader_type);


    [renderEncoder endEncoding];

    releaseObj(renderEncoder);


#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
    //dummy render to flush parameter buffer
    {

    }
#endif
}


void MTL_Scene_27::RenderShadow( KRL_ShadowMap* sm, id<MTLCommandBuffer> commandBuffer)
{
    MetalRender::ShadowMap* metal_sm = dynamic_cast<MetalRender::ShadowMap*>(sm) ;

    std::vector<Mesh*> visible_meshes[2];

    for( uint32 j=0; j<2; j++)
    {
        for( uint32 k=0; k<sm->m_caster_meshes[j].size(); k++)
        {
            visible_meshes[j].push_back( sm->m_caster_meshes[j][k]);
        }
    }

    id <MTLRenderCommandEncoder> renderEncoder = metal_sm->SetAsTargetAndGetEncoder(commandBuffer);

    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

    MTLViewport viewport = { 1.0f, 1.0f, 1.0f * (metal_sm->Size() - 2), 1.0f * (metal_sm->Size() - 2), 0.0f, 1.0f };

    [renderEncoder setViewport:viewport];

    RenderWithCamera(renderEncoder,&metal_sm->m_camera, visible_meshes[0], m_shadowCasterMaterial, 0, 1, 0, 0, m_default_shader_type);
    RenderWithCamera(renderEncoder,&metal_sm->m_camera, visible_meshes[1], m_shadowCasterMaterial, 0, 1, 0, 0, m_default_shader_type);


    [renderEncoder endEncoding];

    releaseObj(renderEncoder);


    m_shadowStaticReceiverMeshes = sm->m_receiver_meshes[0];
}



//#define DISABLE_PREPASS


void MTL_Scene_27::Render()
{
    @autoreleasepool {

        m_dynamicDataBufferPool->InitFrame() ;
        m_pseudoIndicesBufferPool->InitFrame() ;


        id <MTLCommandBuffer> commandBuffer = [m_CommandQueue commandBuffer];
        [commandBuffer enqueue] ;




        std::vector<KCL::MeshInstanceOwner*>::iterator mioi = m_mios.begin();

        while( mioi != m_mios.end())
        {
            KCL::MeshInstanceOwner *mio = *mioi;

            for( KCL::uint32 j = 0; j < 2; j++)
            {
                KCL::Mesh* sm = mio->m_mesh;
                MetalRender::Mesh3 *mtl_mesh = dynamic_cast<MetalRender::Mesh3 *>(sm->m_mesh) ;

                if (!mtl_mesh)
                {
                    NSLog(@"Mesh is not mtl mesh!") ;
                    assert(0) ;
                }

                mtl_mesh->m_mio_offsets[j] = m_pseudoIndicesBuffer->WriteDataAndGetOffset(nil, &mio->m_current_vertex_indices[j][0], mio->m_current_vertex_indices[j].size() * 2);
            }

            mioi++;
        }



        for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
        {
            if( m_global_shadowmaps[i])
            {
                RenderShadow( m_global_shadowmaps[i],commandBuffer);
            }
        }


        for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
        {
    #ifndef DUMMY_RENDER_FOR_PLANAR_FLUSH
            RenderPlanar( m_visible_planar_maps[i],commandBuffer);
    #else
            if(m_visible_planar_maps.size() != i+1)
            {
                RenderPlanar( m_visible_planar_maps[i], m_visible_planar_maps[i+1]);
            }
            else
            {
                RenderPlanar( m_visible_planar_maps[i], 0);
            }
    #endif
        }


        id <MTLRenderCommandEncoder> renderEncoder = nil ;


        if( m_mblur_enabled)
        {
            //glBindFramebuffer( GL_FRAMEBUFFER, m_main_fbo->getName());

            renderEncoder = m_frameBuffer27->SetMainBufferAsTargetAndGetEncoder(commandBuffer);
        }
        else
        {
            id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
            renderEncoder = m_frameBuffer27->SetFinalBufferAsTargetAndGetEncoder(commandBuffer,frameBufferTexture);
        }


#ifndef DISABLE_PREPASS
        if(UseZPrePass())
        {
            RenderPrepass(renderEncoder,m_active_camera, m_visible_meshes[0], 0, 0, 0, 0,m_default_shader_type);
        }
        else
        {
            RenderWithCamera(renderEncoder,m_active_camera, m_visible_meshes[0], 0, 0, 0, 0, 0, m_default_shader_type);
        }
#else

        RenderWithCamera(renderEncoder,m_active_camera, m_visible_meshes[0], 0, 0, 0, 0, 0);
#endif


        RenderWithCamera(renderEncoder,m_active_camera, m_visible_meshes[1], 0, 0, 0, 0, 0, m_default_shader_type);



        for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
        {
            dynamic_cast<Material*>(m_planarReflectionMaterial)->m_planar_map = dynamic_cast<MetalRender::PlanarMap*>(m_visible_planar_maps[i]);
            m_planarReflectionMaterial->m_transparency = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_reflect_intensity;
            m_planarReflectionMaterial->m_textures[2] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[2];
            m_planarReflectionMaterial->m_textures[3] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[3];

            RenderWithCamera(renderEncoder,m_active_camera, m_visible_planar_maps[i]->m_receiver_meshes, m_planarReflectionMaterial, 0, 0, 0, 0, m_default_shader_type);

            m_planarReflectionMaterial->m_textures[2] = 0;
            m_planarReflectionMaterial->m_textures[3] = 0;
        }


        if( m_num_shadow_maps )
        {
            // MTL_TODO maybe this is do nothing? (egypt only). Also present in the gl version
            RenderWithCamera(renderEncoder,m_active_camera, m_shadowStaticReceiverMeshes, m_shadowStaticReceiverMaterial, 0, 0, 0, 0, m_default_shader_type);
        }


        [renderEncoder endEncoding] ;
        releaseObj(renderEncoder) ;


        if( m_mblur_enabled)
        {
            //
            //  Render Velocity buffer
            //

            id <MTLRenderCommandEncoder> velocityEncoder = m_frameBuffer27->SetVelocityBufferAsTargetAndGetEncoder(commandBuffer) ;

            RenderWithCamera(velocityEncoder,m_active_camera, m_motion_blurred_meshes, m_mblurMaterial, 0, 0, 0, 0,m_default_shader_type);

            [velocityEncoder endEncoding] ;

            releaseObj(velocityEncoder) ;

            //
            //  Final Pass
            //


            id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
            id <MTLRenderCommandEncoder> motionBlurEncoder = m_frameBuffer27->SetFinalBufferAsTargetAndGetEncoder(commandBuffer,frameBufferTexture);


            m_frameBuffer27->SetMainColorBufferForMotionBlur(motionBlurEncoder, 0) ;
            m_frameBuffer27->SetVelocityBufferForMotionBlur(motionBlurEncoder, 1) ;

            m_motionBlurPipeLine->Set(motionBlurEncoder) ;

            m_full_screen_quad->Draw(motionBlurEncoder) ;


            [motionBlurEncoder endEncoding] ;
            releaseObj(motionBlurEncoder) ;
        }


        unsigned char slot = m_dynamicDataBufferPool->GetCurrentSlot();
        unsigned char pseudo_intance_slot = m_pseudoIndicesBufferPool->GetCurrentSlot();


#define WAIT_FOR_FRAME 0

#if WAIT_FOR_FRAME

        [commandBuffer commit] ;
        [commandBuffer waitUntilCompleted] ;
        m_dynamicDataBufferPool->MarkSlotUnused(slot) ;

#else

        [commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
         {
             m_dynamicDataBufferPool->MarkSlotUnused(slot);
             m_pseudoIndicesBufferPool->MarkSlotUnused(pseudo_intance_slot);
         }];
        [commandBuffer commit] ;

#endif

        releaseObj(commandBuffer) ;

#if MULTI_LOCK_MUTEX
        m_lock_id++ ;
        m_lock_id %= MetalRender::Mesh3::MAX_LOCK_COUNT ;
#endif

    }
}


void MTL_Scene_27::RenderPrepass(id <MTLRenderCommandEncoder> renderEncoder, Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, MetalRender::ShaderType shader_type)
{
    std::vector<Mesh *> default_visible_meshes;
    std::vector<Mesh *> foliage_visible_meshes;
    KCL::uint32 i;

    /* filter opaque meshes (DEFAULT|FOLIAGE) for prepass */
    for (i = 0; i < visible_meshes.size(); i++)
    {
        if (visible_meshes[i]->m_material->m_material_type == KCL::Material::DEFAULT)
            default_visible_meshes.push_back(visible_meshes[i]);

        if (visible_meshes[i]->m_material->m_material_type == KCL::Material::FOLIAGE)
            foliage_visible_meshes.push_back(visible_meshes[i]);
    }


    /* render opaque_visible_meshes to depth only */
    //glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
    // the disable of color write is done by the material sub pass


    RenderWithCamera(renderEncoder, camera, default_visible_meshes, _override_material, pm, lod, light, -1, shader_type);
    RenderWithCamera(renderEncoder, camera, foliage_visible_meshes, _override_material, pm, lod, light, -1, shader_type);


    /* dispatch shadepass */
    RenderWithCamera(renderEncoder, camera, visible_meshes, _override_material, pm, lod, light, +1, shader_type);
}



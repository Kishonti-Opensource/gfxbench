/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_scene_30.h"
#include "mtl_mesh.h"
#include "mtl_material.h"
#include "mtl_shader_constant_layouts_30.h"
#include "mtl_dynamic_data_buffer.h"
#include "mtl_framebuffer_30.h"
#include "mtl_shadowmap.h"
#include "mtl_pipeline.h"
#include "mtl_quadBuffer.h"

#include <mach/mach.h>
#include <mach/mach_time.h>

#include <kcl_light2.h>
#include <kcl_planarmap.h>
#include <kcl_room.h>
#include <kcl_actor.h>
#include <kcl_animation4.h>

#include "platform.h" 
#include "mtl_cubemap.h"

#include "vbopool.h"
#include "fbo.h"
#include "fbo_metal.h"
#include "mtl_light.h"

#include "misc2.h"
#include "kcl_io.h"

#include <cmath>
#include <cstddef>
#include <algorithm> //std::sort

#import "graphics/graphicscontext.h"
#import "../gfxbench/gfxbench.h"

#import "../gfxbench/global_test_environment.h"

#include "ng/log.h"

//#include "apple_tar_io.h"

#import <assert.h>

#define MOVE_PARTICLES
#define ISSUE_OCCLUSION_QUERIES


#define RENDER_SHADOWMAPS
#define RENDER_GBUFFER
#define RENDER_LIGHTS
#define RENDER_SKY
#define RENDER_REFLECTION_EMISSION_FILTER
#define RENDER_DYNAMIC_SHADOW_DECALS
#define RENDER_STATIC_SHADOW_DECALS
#define RENDER_PARTICLES
#define RENDER_LIGHTSHAFTS
#define RENDER_TRANSPARENT_MESHES
#define RENDER_LENSFLARES
#define RENDER_POSTPROCESSING

// Manhattan 3.1 only
#define ENABLE_LIGHTNING_EFFECT

#if defined(ISSUE_OCCLUSION_QUERIES) || defined(RENDER_OCCLUSION_QUERIES)
#define CREATE_OCCLUSION_QUERIES
#endif

//static KCL::uint32 hud_texids[16];


MTL_Scene_30::MTL_Scene_30(const GlobalTestEnvironment* const gte) :
m_framebuffer(NULL),
m_shadowMaps(),
m_dynamicDataBuffer(NULL),
m_lensFlareDataBuffer(NULL),
m_envMapTextures(),
m_light_noise(NULL),
m_lbos(),
m_commandQueue(nil),
m_lightDepthState(nil),
m_lightShaftDepthState(nil),
m_quadBuffer(NULL),
m_finalBlitQuadBuffer(NULL),
m_lighting_pipelines(),
m_reflection_emission_pipeline(NULL),
m_subFilter_pipeline(NULL),
m_blurFilter_pipeline(NULL),
m_depthOfField_pipeline(NULL),
m_fog_pipeline(NULL),
m_particleAdvect_pipeline(NULL),
m_totalDuration(0)
{
    m_Context = (MetalGraphicsContext*)gte->GetGraphicsContext();
        
    // find a usable Device
    m_Device = m_Context->getDevice();
        
    if(m_Device == nil)
    {
        NSLog(@"Failed to create device");
        exit(-2);
    }
    
    m_framebuffer = nullptr;
    
    m_dynamicDataBufferPool = nullptr;
    m_dynamicDataBuffer = nullptr;
    m_lensFlareDataBuffer = nullptr;
    
    m_quadBuffer = nullptr ;
    m_finalBlitQuadBuffer = nullptr ;
    
    m_light_noise = nullptr ;
    m_noiseTexture = nullptr ;
    m_fireTexture = nullptr ;
    
    for(int i = 0 ; i < NUM_SHADOW_MAPS; i++)
    {
        m_shadowMaps[i] = nullptr ;
    }
    
    for(int i = 0 ; i < 2; i++)
    {
        m_envMapTextures[i] = nullptr ;
    }
    
    for(int i = 0 ; i < MetalRender::LightBuffer::NUM_SHAPES; i++)
    {
        m_lbos[i] = nullptr ;
    }
    
    //
    //  Metal resources
    //
    
    m_commandQueue = nil ;
    
    m_blitDepthState = nil ;
    m_lightDepthState = nil ;
    m_lightShaftDepthState = nil ;
    m_particleDepthStencilState = nil ;
    m_DepthTestOffDepthWritesOff = nil ;
    
    m_TransformFeedbackFramebuffer = nil ;
    m_TransformFeedbackDummyTexture = nil ;
    m_pointSampler = nil ;
    m_linearSampler = nil ;
    
    m_quadBufferVertexLayout = nil ;
}


MTL_Scene_30::~MTL_Scene_30()
{
    mach_timebase_info_data_t sTimebaseInfo;

    mach_timebase_info(&sTimebaseInfo);
    uint64_t ONE_MILLION = 1000 * 1000;

    uint64_t durationInMilliseconds = (m_totalDuration * (double)sTimebaseInfo.numer) / (ONE_MILLION * (double)sTimebaseInfo.denom);
    // Do the maths. We hope that the multiplication doesn't
    // overflow; the price you pay for working in fixed point.

    printf("Total ms in Render() %llu", durationInMilliseconds);

    
	//make sure all outstanding work is done before dealloc
	id <MTLCommandBuffer> finishBuffer = [m_commandQueue commandBuffer];
	
	[finishBuffer commit];
	[finishBuffer waitUntilCompleted];
	releaseObj(finishBuffer);
    
    
    MetalRender::Pipeline::ClearCashes() ;

    
    for(int i = 0; i < NUM_SHADOW_MAPS; i++)
    {
        // Don't delete the shadowmaps as m_shadowMaps is only an
        // alias for m_global_shadowmaps managed by the base class
        m_shadowMaps[i] = nullptr;
    }

    for(int i = 0; i < NUM_LIGHT_TYPES; i++)
    {
        m_lighting_pipelines[i] = nullptr;
    }

    for(int i = 0; i < MetalRender::LightBuffer::NUM_SHAPES; i++)
    {
        delete m_lbos[i];
        m_lbos[i] = nullptr;
    }

    delete m_dynamicDataBufferPool;

    delete m_framebuffer;
    m_framebuffer = nullptr;
	
	if(m_finalBlitQuadBuffer != m_quadBuffer)
	{
		delete m_finalBlitQuadBuffer;
		m_finalBlitQuadBuffer = nullptr;
	}
	
    delete m_quadBuffer;
    m_quadBuffer = nullptr;
    
    delete m_light_noise  ; m_light_noise  = nullptr ;
    delete m_noiseTexture ; m_noiseTexture = nullptr ;
    delete m_fireTexture  ; m_fireTexture  = nullptr ;
    
    m_Context = nullptr ;
    
	MetalRender::TF_emitter::ReleaseBillboardDataBuffer();
	
    Release_MetalResources();
}


KCL::KCL_Status MTL_Scene_30::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
#if !TARGET_OS_IPHONE
    m_force_highp = true;
#endif
	
	@autoreleasepool {
		MetalRender::TF_emitter::InitBillboardDataBuffer();
        
        int manhattan_frame_lag = MetalRender::METAL_MAX_FRAME_LAG ;
        
        m_dynamicDataBufferPool = new MetalRender::DynamicDataBufferPool(manhattan_frame_lag) ;
		
        m_dynamicDataBuffer = m_dynamicDataBufferPool->GetNewBuffer(1024*1024) ;
		m_lensFlareDataBuffer = m_dynamicDataBufferPool->GetNewBuffer(1024*1024) ;
        
        //
        //
        //  Main Framebuffer type:
        //
        //
        MTLPixelFormat main_framebuffer_format = MTLPixelFormatInvalid ;
        if (m_scene_version == KCL::SV_30)
        {
            main_framebuffer_format = MTLPixelFormatRGBA8Unorm ;
        }
        else if (m_scene_version == KCL::SV_31)
        {
            main_framebuffer_format = MTLPixelFormatRGB10A2Unorm ;
        }

#ifndef RENDER_POSTPROCESSING
        // for onscreen rendering without postprocessing we need to force BGRA8 hardware framebufferformat
        main_framebuffer_format = MTLPixelFormatBGRA8Unorm ;
#endif
        
        
        m_default_shader_type = MetalRender::Pipeline::PixelFormatToShaderType(main_framebuffer_format) ;
        
        m_requied_shader_types.insert(m_default_shader_type) ;
        
        MetalRender::Mesh3::InitVertexLayouts(m_scene_version) ;
        
		IncrementProgressTo(0.32f);
		
	//	m_light_noise = reinterpret_cast<MetalRender::Texture*>(KCL::Texture::factory->Create2D("light.png", KCL::TC_Commit));
	//	m_noiseTexture = KCL::Texture::factory->Create2D("noise2_color.png", KCL::TC_Commit | KCL::TC_NearestFilter | KCL::TC_NoMipmap);

        m_light_noise = (MetalRender::Texture*)TextureFactory().CreateAndSetup(KCL::Texture_2D, (common_dir+"light.png").c_str(), KCL::TC_Commit) ;
        m_noiseTexture = (MetalRender::Texture*)TextureFactory().CreateAndSetup(KCL::Texture_2D, (common_dir+"noise2_color.png").c_str(), KCL::TC_Commit | KCL::TC_NearestFilter | KCL::TC_NoMipmap);
        
		//LoadHudTextures( m_path + std::string(ImagesDirectory()) + std::string("hud/"));
		m_fireTexture = (MetalRender::Texture*)TextureFactory().CreateAndSetup(KCL::Texture_3D, (ImagesDirectory() + std::string("anims/fire")).c_str(), KCL::TC_Commit | KCL::TC_Clamp);
		//smoke_texid = LoadTexture3D( m_path + ImagesDirectory() + std::string("anims/caustic") );
	
		#pragma mark Occlusion Query Loading
		#ifdef CREATE_OCCLUSION_QUERIES
		{
            //queries take 64bits
            uint32_t totalLights = 0; //can take this to be an offset later, number*sizeof(uint64_t)
            for( KCL::uint32 i=0; i<m_actors.size(); i++)
            {
                KCL::Actor *actor = m_actors[i];
                
                for( KCL::uint32 j=0; j<actor->m_lights.size(); j++)
                {
                    MetalRender::Light* l = dynamic_cast<MetalRender::Light*>(actor->m_lights[j]);
                    
                    //are there lights attached to multiple actors?
                    l->m_query_objects[0] = totalLights;
                    totalLights++;
                }
            }
            
            printf("Double buffering occlusion query buffers for %u lights\n", totalLights);
            
            assert(totalLights != 0);
            uint64_t* blank = reinterpret_cast<uint64_t*>(malloc(sizeof(uint64_t)*totalLights));
            memset(blank, 0, sizeof(uint64_t)*totalLights);
            
            for(int i = 0; i < MetalRender::Light::QUERY_COUNT; i++)
            {
                m_occlusionQueries[i].m_size = totalLights*sizeof(uint64_t);
                m_occlusionQueries[i].m_frame = 0;
                m_occlusionQueries[i].m_queriesRetired = 1;
#if TARGET_OS_EMBEDDED
                m_occlusionQueries[i].m_queryResults = [m_Device newBufferWithLength:m_occlusionQueries[i].m_size options:MTLResourceOptionCPUCacheModeDefault];
#else
                m_occlusionQueries[i].m_queryResults = [m_Device newBufferWithLength:m_occlusionQueries[i].m_size options:MTLResourceStorageModeManaged];
#endif
            }
            m_currentQueryBufferIndex = 0;
            m_lastQueryBufferIndex = 1;
            
            free(blank);
		}
		#endif
		
		IncrementProgressTo(0.34f);

        GLB::FBO::bind(0) ;
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
		m_framebuffer = new MetalRender::Framebuffer(frameBufferTexture, m_viewport_width, m_viewport_height,  0, main_framebuffer_format,m_scene_version);

        m_framebuffer->CreateOcclusionQueryFrameBuffer(m_occlusionQueries, MetalRender::Light::QUERY_COUNT);
		
		{
			MTLTextureDescriptor* desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm width:1 height:1 mipmapped:NO];
            desc.usage = MTLTextureUsageRenderTarget;
#if !TARGET_OS_IPHONE
			desc.storageMode = MTLStorageModePrivate;
#endif
			m_TransformFeedbackDummyTexture = [m_Device newTextureWithDescriptor:desc];
            
			
			m_TransformFeedbackFramebuffer = [[MTLRenderPassDescriptor alloc] init];
            
            
			MTLRenderPassColorAttachmentDescriptor *colorAttachment = m_TransformFeedbackFramebuffer.colorAttachments[0];
			colorAttachment.texture = m_TransformFeedbackDummyTexture;
			colorAttachment.loadAction = MTLLoadActionDontCare;
            colorAttachment.storeAction = MTLStoreActionDontCare;
			
			{
				MTLSamplerDescriptor* samplerDesc = [[MTLSamplerDescriptor alloc] init];
				samplerDesc.minFilter = MTLSamplerMinMagFilterLinear;
				samplerDesc.magFilter = MTLSamplerMinMagFilterLinear;
				samplerDesc.mipFilter = MTLSamplerMipFilterNotMipmapped;
                

				samplerDesc.maxAnisotropy = 1;
				samplerDesc.sAddressMode = MTLSamplerAddressModeClampToEdge;
				samplerDesc.tAddressMode = MTLSamplerAddressModeClampToEdge;
				samplerDesc.rAddressMode = MTLSamplerAddressModeClampToEdge;
				samplerDesc.normalizedCoordinates = YES;
				samplerDesc.lodMinClamp = 0;
				samplerDesc.lodMaxClamp = FLT_MAX;
				m_linearSampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];
				
				samplerDesc.minFilter = MTLSamplerMinMagFilterNearest;
				samplerDesc.magFilter = MTLSamplerMinMagFilterNearest;
				
				m_pointSampler = [m_Device newSamplerStateWithDescriptor:samplerDesc];
				
				releaseObj(samplerDesc);
			}
			
			{
				MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
				desc.depthWriteEnabled = NO;
				desc.depthCompareFunction = MTLCompareFunctionLess;
				m_particleDepthStencilState = [m_Device newDepthStencilStateWithDescriptor:desc];
				
				releaseObj(desc);
			}
			
			{
				MTLDepthStencilDescriptor* desc = [[MTLDepthStencilDescriptor alloc] init];
				desc.depthWriteEnabled = NO;
				desc.depthCompareFunction = MTLCompareFunctionAlways;
				m_DepthTestOffDepthWritesOff = [m_Device newDepthStencilStateWithDescriptor:desc];
				releaseObj(desc);
			}
		}
		
		MTLDepthStencilDescriptor *blitDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
		blitDepthStateDesc.depthWriteEnabled = NO;
		blitDepthStateDesc.depthCompareFunction = MTLCompareFunctionAlways;

		m_blitDepthState = [m_Device newDepthStencilStateWithDescriptor:blitDepthStateDesc];

		releaseObj(blitDepthStateDesc);


		IncrementProgressTo(0.36f);

		IncrementProgressTo(0.38f);

		for(int shape = 0; shape < MetalRender::LightBuffer::NUM_SHAPES; shape++)
		{
			m_lbos[shape] = new MetalRender::LightBuffer((MetalRender::LightBuffer::Shape)shape);
		}

		IncrementProgressTo(0.4f);

		//create buffers for TF_emitters
		for(std::vector<KCL::Actor*>::size_type i=0; i<m_actors.size(); ++i)
		{
			for(std::vector<KCL::AnimatedEmitter*>::size_type j=0; j<m_actors[i]->m_emitters.size(); ++j)
			{
				KCL::_emitter* em = static_cast<KCL::_emitter*>(m_actors[i]->m_emitters[j]);
				em->Process();
			}
		}

		IncrementProgressTo(0.42f);


		IncrementProgressTo(0.44);

		IncrementProgressTo(0.46);


		IncrementProgressTo(0.5f);

		std::string required_render_api = "metal";

		IncrementProgressTo(0.51f);

        MetalRender::Pipeline::InitShaders(m_scene_version);

		IncrementProgressTo(0.52f);

		// Blur FBO initialized in Framebuffer

		IncrementProgressTo(0.53f);
		
        m_quadBufferVertexLayout = MetalRender::QuadBuffer::GetVertexLayout() ;
		m_quadBuffer = new MetalRender::QuadBuffer(MetalRender::QuadBuffer::kBlitQuadLandscape);
       // m_quadBuffer = new MetalRender::QuadBuffer(MetalRender::QuadBuffer::kBlitQuadPortrait);
        
		m_finalBlitQuadBuffer = m_quadBuffer;
			
		IncrementProgressTo(0.54f);

		assert(m_num_shadow_maps <= NUM_SHADOW_MAPS);
		for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
		{
			m_shadowMaps[i] = new MetalRender::ShadowMap(m_fboShadowMap_size, m_fboShadowMap_size,m_shadow_method_str);

			m_global_shadowmaps[i] = m_shadowMaps[i];
		}

		IncrementProgressTo(0.55f);

		// NOTE: m_do_instancing is false for manhattan so nothing to do between .55 and .59

		IncrementProgressTo(0.59f);
        for( KCL::uint32 i=0; i<m_rooms.size(); i++)
		{
            KCL::XRoom *room = m_rooms[i];
            for( KCL::uint32 j=0; j<room->m_meshes.size(); j++)
			{
				room->m_meshes[j]->DeleteUnusedAttribs();
			}
		}

		IncrementProgressTo(0.6f);

        for( KCL::uint32 i = 0; i < m_actors.size(); i++)
		{
            KCL::Actor *actor = m_actors[i];

            for( KCL::uint32 i = 0; i < actor->m_meshes.size(); i++)
			{
                KCL::Mesh* m = actor->m_meshes[i];
				m->DeleteUnusedAttribs();

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

		IncrementProgressTo(0.65f);

        for( KCL::uint32 i=0; i<m_sky_mesh.size(); i++)
		{
			m_sky_mesh[i]->DeleteUnusedAttribs();
		}

		IncrementProgressTo(0.67f);

		for(size_t i = 0; i < m_meshes.size(); ++i)
		{
			MetalRender::Mesh3* akbMesh = dynamic_cast<MetalRender::Mesh3*>(m_meshes[i]);

			akbMesh->InitVertexAttribs();
		}

		IncrementProgressTo(0.7f);

        KCL::KCL_Status result = reloadShaders();
        
        if (result != KCL::KCL_TESTERROR_NOERROR)
        {
            return result ;
        }

		// Commit textures for any materials we previously created
		for(size_t i = 0; i < m_materials.size(); ++i)
		{
			MetalRender::Material* material = dynamic_cast<MetalRender::Material*>(m_materials[i]);

			material->InitImages();
		}

		IncrementProgressTo(0.8f);

		// Initi envmaps

		//NOTE: Manhattan uses instancing, which does not work currently with
		//      multiple cubemaps - this reduces the cubes to always select the
		//      same 2 - which will make sure image data is consistent between
		//      consequtive runs
		{
            std::vector<KCL::CubeEnvMapDescriptor> envmap_fixup;
			envmap_fixup.push_back(m_envmap_descriptors[0]);
			envmap_fixup.push_back(m_envmap_descriptors[1]);
			m_envmap_descriptors = envmap_fixup;
		}

        for( KCL::uint32 i = 0; i < m_envmap_descriptors.size(); i++)
		{
            CubeEnvMap_Metal *cem = CreateEnvMap( m_envmap_descriptors[i].m_position, i, m_scene_version == KCL::SV_31);
			m_cubemaps.push_back(cem);
			m_envMapTextures[i] = static_cast<MetalRender::Texture*>(cem->GetTexture());
		}
		
		m_commandQueue = m_Context->getMainCommandQueue();
		
	} // autoreleasepool

    return KCL::KCL_TESTERROR_NOERROR;
}


void MTL_Scene_30::Release_MetalResources()
{
    releaseObj(m_commandQueue);
    releaseObj(m_particleDepthStencilState) ;
    releaseObj(m_DepthTestOffDepthWritesOff) ;
    
    releaseObj(m_pointSampler) ;
    releaseObj(m_linearSampler) ;
    
    releaseObj(m_quadBufferVertexLayout) ;
    
    releaseObj(m_lightShaftDepthState);
    releaseObj(m_lightDepthState);
    releaseObj(m_blitDepthState);
    releaseObj(m_TransformFeedbackFramebuffer) ;
    releaseObj(m_TransformFeedbackDummyTexture) ;
    releaseObj(m_Device) ;
}


#ifdef DEPTH_SORT_DRAWCALLS
struct mesh_depth_sort_info
{
    KCL::Mesh *mesh;
    Vector2D extents;
    float vertexCenterDist;
};

static bool depth_compare(const mesh_depth_sort_info &A, const mesh_depth_sort_info &B)
{
    return A.vertexCenterDist < B.vertexCenterDist;
}

static void sort_vis_list(std::vector<KCL::Mesh*> &visible_meshes, KCL::Camera2 *camera)
{
    std::vector<mesh_depth_sort_info> sorted_visible_meshes;

    for (uint32 i = 0; i < visible_meshes.size(); i++)
    {
        mesh_depth_sort_info mesh_info;

        mesh_info.mesh = visible_meshes[i];
        mesh_info.extents = visible_meshes[i]->m_aabb.DistanceFromPlane(camera->GetCullPlane(KCL::CULLPLANE_NEAR));
        mesh_info.vertexCenterDist = Vector4D::dot( KCL::Vector4D(visible_meshes[i]->m_vertexCenter), camera->GetCullPlane(KCL::CULLPLANE_NEAR));

        // force objects with infinite bounds (actors) to draw first
        if (mesh_info.extents.x < -FLT_MAX || mesh_info.extents.y > FLT_MAX)
        {
            mesh_info.extents.x = 0.0f;
            mesh_info.extents.y = 0.0f;
            mesh_info.vertexCenterDist = 0.0f;
        }
        sorted_visible_meshes.push_back(mesh_info);
    }

    // depth sort
    std::sort (sorted_visible_meshes.begin(), sorted_visible_meshes.end(), &depth_compare);

    // remap original visible meshes
    for (uint32 i = 0; i < visible_meshes.size(); i++)
        visible_meshes[i] = sorted_visible_meshes[i].mesh;
}
#endif // DEPTH_SORT_DRAWCALLS

template<bool SetVertex, bool SetFragment>
void MTL_Scene_30::WriteAndSetFrameConsts(KCL::Camera2* camera,
                                       id <MTLRenderCommandEncoder> renderEncoder)
{
    // Upload per-frame constants
    frameConsts uFrameConsts;
    uFrameConsts.global_light_dirXYZ_pad = KCL::Vector4D(m_light_dir);
    uFrameConsts.global_light_colorXYZ_pad = KCL::Vector4D(m_light_color);
    
    uFrameConsts.view_dirXYZ_pad = KCL::Vector4D( -camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10], 0.0f);
    
    float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;
    uFrameConsts.view_posXYZ_time = KCL::Vector4D(camera->GetEye().x, camera->GetEye().y, camera->GetEye().z, normalized_time);
    
    uFrameConsts.background_colorXYZ_fogdensity = KCL::Vector4D(m_background_color.x, m_background_color.y, m_background_color.z, m_fog_density);
    uFrameConsts.depth_parameters = camera->m_depth_linearize_factors;
    
    uFrameConsts.inv_resolutionXY_pad = KCL::Vector4D(1.0f / m_viewport_width, 1.0f / m_viewport_height, 0.0f, 0.0f);

    m_dynamicDataBuffer->WriteAndSetData<SetVertex, SetFragment>(renderEncoder, FRAME_CONST_INDEX, &uFrameConsts, sizeof(frameConsts));
}

void MTL_Scene_30::RenderWithCamera(KCL::Camera2* camera,
                                 id <MTLRenderCommandEncoder> renderEncoder,
                                 std::vector<KCL::Mesh*> &visible_meshes,
                                 KCL::Material *kcl_override_material,
                                 KCL::PlanarMap* pm,
                                 KCL::uint32 lod,
                                 KCL::Light* light,
                                 int pass_type)
{
    MetalRender::Material *override_material = dynamic_cast<MetalRender::Material*>(kcl_override_material);
    MetalRender::Material *last_material = NULL;
    int last_mesh_type = -1;

    if(!visible_meshes.size())
    {
        return;
    }

    // MTL FIXME:  We crash in this sort.  (Running out of stack space?)
    // Should we even do this with the MTL version?
    // Perhaps only to maintain parity with the GL version.
    #ifdef DEPTH_SORT_DRAWCALLS
    sort_vis_list(visible_meshes, camera);
    #endif
#define USE_INSTANCING
#ifdef USE_INSTANCING
    if( &visible_meshes == &m_visible_meshes[0])
        KRL_Scene::CollectInstances(visible_meshes);
#endif

    for(KCL::uint32 j=0; j<visible_meshes.size(); j++)
    {
        KCL::Mesh* sm = (KCL::Mesh*)visible_meshes[j];
        MetalRender::Mesh3 *mtl_mesh  =  dynamic_cast<MetalRender::Mesh3 *>(sm->m_mesh);

        assert(mtl_mesh);

        if( !sm->m_mesh)
        {
            continue;
        }

#ifdef FORCE_SINGLE_QUAD_MESH_RENDERING_ONLY
        if(mtl_mesh->m_instances[sm->m_material].size()  >= 2)
            return;
		
        if(mtl_mesh->getIndexCount(lod) > 6)
            continue;
#endif //FORCE_SINGLE_QUAD_MESH_RENDERING_ONLY

        int mesh_type = sm->m_mesh->m_vertex_matrix_indices.size() != 0;

        // Is there more than one instance of this mesh3 with this material?
        if(mtl_mesh->m_instances[sm->m_material].size() > 1)
        {
            std::set<KCL::Material*>::iterator iter = mtl_mesh->m_is_rendered.find(sm->m_material);

            // Have we already rendered this Mesh3 with instancing when we came across a different Mesh?
            if( iter != mtl_mesh->m_is_rendered.end())
            {
                continue;
            }

            // Force instancing version of the shader
            mesh_type = 2;
        }

        MetalRender::Material* material = dynamic_cast<MetalRender::Material*>(sm->m_material);
		assert(material);

        if( override_material)
        {
            material = override_material;
        }
    
        KRL_CubeEnvMap *envmaps[2];
        KCL::Matrix4x4 mvp;
        KCL::Matrix4x4 mv;
        KCL::Matrix4x4 model;
        KCL::Matrix4x4 inv_model;
        KCL::Matrix4x4 inv_modelview;
        KCL::Vector3D pos( sm->m_world_pom.v[12], sm->m_world_pom.v[13], sm->m_world_pom.v[14]);

        if( last_material != material || last_mesh_type != mesh_type)
        {
            material->Set(renderEncoder, pass_type, mesh_type, m_default_shader_type);

            if( material->HasVideoTexture())
            {
                if( material->m_frame_when_animated != m_frame)
                {
                    // MTL_TODO
                    // It should run once a frame
                    material->DecodeMipMapVideo() ;
                    material->m_frame_when_animated = m_frame;
                }
            }

            matConsts uMatConsts;
            uMatConsts.matparams_disiseri =
                KCL::Vector4D(material->m_diffuse_intensity, material->m_specular_intensity,
                              material->m_specular_exponent, material->m_reflect_intensity);
            uMatConsts.fresnelXYZ_transp =
                KCL::Vector4D(material->m_fresnel_params.x, material->m_fresnel_params.y,
                              material->m_fresnel_params.z, material->m_transparency);


            if( material->m_frame_when_animated != m_frame)
            {
                //if the test has looped, reset animation offsets
                if(material->m_animation_time > m_animation_time / 1000.0f)
                {
                    material->m_animation_time_base = 0.0f;
                }

                material->m_animation_time = m_animation_time / 1000.0f;

                if( material->m_translate_u_track)
                {
                    KCL::Vector4D r;

                    KCL::_key_node::Get( r, material->m_translate_u_track, material->m_animation_time, material->m_animation_time_base, true);

                    material->m_uv_offset.x = -r.x;
                }
                if( sm->m_material->m_translate_v_track)
                {
                    KCL::Vector4D r;

                    KCL::_key_node::Get( r, material->m_translate_v_track, material->m_animation_time, material->m_animation_time_base, true);

                    material->m_uv_offset.y = -r.x;
                }
                material->m_frame_when_animated = m_frame;
            }

            uMatConsts.tranlate_uv = material->m_uv_offset.v;

            m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, MATERIAL_CONST_INDEX, &uMatConsts, sizeof(matConsts));
        }

        last_material = material;

        last_mesh_type = mesh_type;
		
		
        switch( mesh_type)
        {
            case 0:
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
                KCL::Matrix4x4::InvertModelView(mv, inv_modelview);

                model = sm->m_world_pom;
                inv_model = KCL::Matrix4x4::Invert4x3( sm->m_world_pom);
                break;
            }
            case 2:
            case 1:
            {
                mvp = camera->GetViewProjection();
                mv = camera->GetView();
                KCL::Matrix4x4::InvertModelView(mv, inv_modelview);
                model.identity();
                inv_model.identity();
                break;
            }
        }


        meshConsts uMeshConsts;

        Get2Closest( pos, envmaps[0], envmaps[1], uMeshConsts.envmapInterpolator.x);

        uMeshConsts.mvp = mvp;
        uMeshConsts.mv = mv;
        uMeshConsts.model = model;
        uMeshConsts.inv_model = inv_model;
        uMeshConsts.inv_modelview = inv_modelview;

        m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, MESH_CONST_INDEX, &uMeshConsts, sizeof(meshConsts));

        m_envMapTextures[0]->Set(renderEncoder, ENVMAP_0_SLOT);
        m_envMapTextures[1]->Set(renderEncoder, ENVMAP_1_SLOT);
		
		//for transparent glows we look at the model color and assign to slot 3
		if(material->m_usesColorParam)
		{
			KCL::Vector4D color(sm->m_color.x,sm->m_color.y,sm->m_color.z,1.0);
			m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, COLOR_DATA_INDEX, color, sizeof(color));
		}
		
		//if the light exists, this is a lens flare
		if(light != NULL)
		{
			KCL::Vector4D lightPos(&light->m_world_pom.v[12]);
			KCL::Vector4D lightColor(light->m_diffuse_color);
			lightColor.w = 1.0f;
			m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, COLOR_DATA_INDEX, &lightColor, sizeof(lightColor));
			m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, LENSFLARE_LIGHT_POS_INDEX, &lightPos, sizeof(lightPos));
		}
		
        mtl_mesh->SetVertexBuffer(renderEncoder);

#ifdef USE_INSTANCING
        unsigned long numInstances = mtl_mesh->m_instances[sm->m_material].size();

        // If more than one instance...
        if( numInstances >= 2)
        {
            //  Begin: Doing what GLB::CollectInstances does per visible mesh
            mtl_mesh->m_is_rendered.insert(sm->m_material);

            size_t instanceDataSize = sizeof(KRL::Mesh3::InstanceData) * numInstances;

            m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder,
                                                 INSTANCE_DATA_INDEX,
                                                 &(mtl_mesh->m_instances[sm->m_material][0]),
                                                 instanceDataSize);

            // End: Doing what GLB::CollectInstances does per visible mesh

        }
        // if less than one instance...
        else
        {
            //...make sure numInstances is 1 (because 0 is illegal)
            numInstances = 1;
        }
#else // USE_INSTANCING
        unsigned long numInstances = 1;
#endif
        // End: Doing what GLB::CollectInstances does per visible mesh

        // GLB uses sm->m_primitive_count if it's non zero.
        // never seems to hit it though
        assert(!sm->m_primitive_count);

        mtl_mesh->Draw(renderEncoder, lod, numInstances);

#ifdef FORCE_SINGLE_QUAD_MESH_RENDERING_ONLY
        return;
#endif // FORCE_SINGLE_QUAD_MESH_RENDERING_ONLY
    } // end for each visible_mesh
}


 
void MTL_Scene_30::RenderLightShafts(id<MTLRenderCommandEncoder> renderEncoder)
{
    LightShaft lightShaftBuilder;

    for( KCL::uint32 j=0; j<m_lightshafts.size(); j++)
	{
        KCL::Light *l = m_lightshafts[j];

        for( KCL::uint32 i=0; i<8; i++)
		{
			lightShaftBuilder.m_corners[i].set( l->m_frustum_vertices[i].v);
		}

		bool isCamShaft = false;

		KCL::Matrix4x4 m2 = l->m_inv_light_projection * l->m_world_pom;
		lightShaftBuilder.CreateCone( m2, m_active_camera->GetCullPlane( KCL::CULLPLANE_NEAR), isCamShaft);
	}
    

    m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder,
                                            VERTEX_DATA_INDEX,
                                            lightShaftBuilder.m_vertices[0].v,
                                            lightShaftBuilder.m_vertices.size() * sizeof(KCL::Vector3D));
    

    size_t indexBufferOffset =
        m_dynamicDataBuffer->WriteDataAndGetOffset(renderEncoder,
                                                    &lightShaftBuilder.m_indices[0],
                                                    lightShaftBuilder.m_indices.size() * sizeof(uint16_t));

    [renderEncoder setCullMode:MTLCullModeBack];
    
    [renderEncoder setDepthStencilState:m_lightShaftDepthState];

    m_fog_pipeline->Set(renderEncoder);

    m_light_noise->Set(renderEncoder, 0);

    m_framebuffer->SetDepthAsFragmentTexture(renderEncoder, 1);
    
    for( KCL::uint32 j=0; j<m_lightshafts.size(); j++)
	{
        KCL::Light *l = m_lightshafts[j];

		if( !lightShaftBuilder.m_num_indices[j])
		{
			continue;
		}
        
        meshConsts uMeshConst;
        uMeshConst.mvp = m_active_camera->GetViewProjection();
        uMeshConst.mv = m_active_camera->GetView();

        m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder,
                                             MESH_CONST_INDEX,
                                             (const void*)&uMeshConst,
                                             sizeof(meshConsts));

        static const KCL::Matrix4x4 shadowM (0.5f, 0, 0, 0,
                                        0, 0.5f, 0, 0,
                                        0, 0, 0.5f, 0,
                                        0.5f, 0.5f, 0.5f, 1);
        lightConsts uLightConst;

        KCL::Matrix4x4 m0;
        KCL::Matrix4x4::Invert4x4( l->m_world_pom, m0);
        uLightConst.shadowMatrix0 = m0 * l->m_light_projection * shadowM;

        {
            KCL::Vector3D dir( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10]);
            dir.normalize();

            uLightConst.light_x = KCL::Vector4D(dir.x, dir.y, dir.z, l->m_world_pom[14]);
        }


        uLightConst.light_posXYZ_pad =  KCL::Vector4D(l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], 1.0);

        {
			float i = 1.0f;
			if( l->m_intensity_track)
			{
                KCL::Vector4D v;

				l->t = m_animation_time / 1000.0f;

                KCL::_key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

				i = v.x / l->m_intensity;
			}

            uLightConst.light_colorXYZ_pad = KCL::Vector4D(l->m_diffuse_color.x * i, l->m_diffuse_color.y * i, l->m_diffuse_color.z * i, 1.0);
		}

        {
			float fov = l->m_spotAngle;

            uLightConst.spotcosXY_attenZ_pad.x = cosf( KCL::Math::Rad( fov / 2.0f));
			uLightConst.spotcosXY_attenZ_pad.y = 1.0f / (1.0f - uLightConst.spotcosXY_attenZ_pad.x);


            float r = l->m_radius;
            uLightConst.spotcosXY_attenZ_pad.z = -1.0f / (r * r);
        }

        m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder,
                                             LIGHT_CONST_INDEX,
                                             (const void*)&uLightConst,
                                             sizeof(lightConsts));

        m_dynamicDataBuffer->DrawWithIndicesAtOffset(renderEncoder,
                                                     indexBufferOffset+lightShaftBuilder.m_index_offsets[j]*sizeof(uint16_t),
                                                     lightShaftBuilder.m_num_indices[j], MTLIndexTypeUInt16);
    }
}

void MTL_Scene_30::RenderShadow(MetalRender::ShadowMap* shadowMap, id<MTLCommandBuffer> commandBuffer)
{
    std::vector<KCL::Mesh*> visible_meshes[2];

	//FIXME the 2 is hardcoded here. Perhaps this should be read from a value set per-scene?
	
    for( KCL::uint32 j=0; j<2; j++)
    {
        for( KCL::uint32 k=0; k<shadowMap->m_caster_meshes[j].size(); k++)
        {
            visible_meshes[j].push_back( shadowMap->m_caster_meshes[j][k]);
        }
    }
	
	id <MTLRenderCommandEncoder> renderEncoder = shadowMap->SetAsTargetAndGetEncoder(commandBuffer);
	
    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

    MTLViewport viewport = { 1.0f, 1.0f, 1.0f * (shadowMap->Size() - 2), 1.0f * (shadowMap->Size() - 2), 0.0f, 1.0f };

    [renderEncoder setViewport:viewport];

    WriteAndSetFrameConsts<true, true>(&shadowMap->m_camera, renderEncoder);

    RenderWithCamera(&shadowMap->m_camera, renderEncoder, visible_meshes[0], m_shadowCasterMaterial, 0, 1, 0, 0);
    RenderWithCamera(&shadowMap->m_camera, renderEncoder, visible_meshes[1], m_shadowCasterMaterial, 0, 1, 0, 0);

    [renderEncoder endEncoding];

    releaseObj(renderEncoder);
}

void MTL_Scene_30::RenderLight(KCL::Light *l, id <MTLRenderCommandEncoder> renderEncoder, bool blend)
{
    KCL::Matrix4x4 model;

    LightPipelineType pipeType; // Equivalent to light_shader_index
    MetalRender::LightBuffer::Shape lightBufferShape;

    if( l->m_light_type == KCL::Light::OMNI)
    {
        pipeType = blend?LIGHT_POINT:LIGHT_POINT_NOBLEND;
        lightBufferShape = MetalRender::LightBuffer::SPHERE;

        model.translate( KCL::Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
        model.scale( KCL::Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
    }
    else if( l->m_light_type == KCL::Light::SPOT)
    {
        pipeType = blend?LIGHT_SPOT:LIGHT_SPOT_NOBLEND;
        lightBufferShape = MetalRender::LightBuffer::CONE;
        float halfSpotAngle = KCL::Math::Rad(0.5f * l->m_spotAngle);

        float scalingFactorX = KCL::Vector3D(l->m_world_pom.v[0], l->m_world_pom.v[1], l->m_world_pom.v[2]).length();
        float scalingFactorY = KCL::Vector3D(l->m_world_pom.v[4], l->m_world_pom.v[5], l->m_world_pom.v[6]).length();
        float scalingFactorZ = KCL::Vector3D(l->m_world_pom.v[8], l->m_world_pom.v[9], l->m_world_pom.v[10]).length();

        assert(fabs(scalingFactorX - scalingFactorY) < 0.001f);
        assert(fabs(scalingFactorY - scalingFactorZ) < 0.001f);
        assert(fabs(scalingFactorX - scalingFactorZ) < 0.001f);

        model.zero();
        model.v33 = l->m_radius * (1.0f / scalingFactorZ);
        model.v11 = model.v22 = model.v33 * tanf(halfSpotAngle) * 1.2f; //1.2 is: extra opening to counter low tess-factor of the cone
        model.v43 = -model.v33;    // Translate so the top is at the origo
        model.v44 = 1;
        model *= l->m_world_pom;
    }
    else if( l->m_light_type == KCL::Light::SHADOW_DECAL)
    {
        pipeType = LIGHT_SHADOW_DECAL;
        lightBufferShape = MetalRender::LightBuffer::SPHERE;

        model.translate( KCL::Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
        model.scale( KCL::Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
    }
    else
    {
        assert(!"Unsupported light type");
    }

    MetalRender::Pipeline *pipeline = m_lighting_pipelines[pipeType];

    pipeline->Set(renderEncoder);

    meshConsts uMeshConsts;
    uMeshConsts.mvp = model * m_active_camera->GetViewProjection();
    uMeshConsts.mv = model * m_active_camera->GetView();
    uMeshConsts.model = model;
    uMeshConsts.inv_model.identity();
    uMeshConsts.inv_modelview.identity();

    m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, MESH_CONST_INDEX, &uMeshConsts, sizeof(meshConsts));

    float i = 1.0f;
    if( l->m_intensity_track)
    {
        KCL::Vector4D v;

        l->t = m_animation_time / 1000.0f;

        KCL::_key_node::Get( v, l->m_intensity_track, l->t, l->tb, true);

        //i = v.x / l->m_intensity;
        i = 0.01 * v.x;// / l->m_intensity;
    }
    lightConsts uLightConsts;
    uLightConsts.light_colorXYZ_pad = KCL::Vector4D(l->m_diffuse_color.x * i, l->m_diffuse_color.y * i, l->m_diffuse_color.z * i, 0.0f);

    uLightConsts.light_posXYZ_pad = KCL::Vector4D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], 0.0f);
    
    KCL::Vector3D dir( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10]) ;
    dir.normalize() ;
    uLightConsts.light_x = KCL::Vector4D( dir.x, dir.y, dir.z, l->m_world_pom[14]);

    KCL::Vector2D spot_sin;
    float fov = l->m_spotAngle;
    spot_sin.x = cosf( KCL::Math::Rad( fov / 2.0f));
    spot_sin.y = 1.0f / (1.0f - spot_sin.x);
    uLightConsts.spotcosXY_attenZ_pad = KCL::Vector4D(spot_sin.x, spot_sin.y, -1.0f / (l->m_radius * l->m_radius), 0.0f);
    uLightConsts.shadowMatrix0 = m_global_shadowmaps[0]->m_matrix;
    m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, LIGHT_CONST_INDEX, &uLightConsts, sizeof(uLightConsts));

    m_shadowMaps[0]->SetAsTexture(renderEncoder, SHADOW_TEXTURE_SLOT);

    m_lbos[lightBufferShape]->Draw(renderEncoder);
}



void MTL_Scene_30::Render()
{
    @autoreleasepool {

		uint64_t startFrameTime = mach_absolute_time();
		
		m_dynamicDataBufferPool->InitFrame();
        
		id <MTLCommandBuffer> commandBuffer = [m_commandQueue commandBuffer];
        
        id <MTLCommandBuffer> genVideoMipMapCommandBuffer = nil;
        
        
        [commandBuffer enqueue] ;

		// Shadow pass
		#ifdef RENDER_SHADOWMAPS
		{
			for(KCL::uint32 i=0; i<m_num_shadow_maps; i++)
			{
				if(m_shadowMaps[i])
				{
					RenderShadow(m_shadowMaps[i], commandBuffer);
				}
			}
		}
		#endif // RENDER_SHADOWMAPS

		// GBuffer pass
		{
			id <MTLRenderCommandEncoder> renderEncoder = m_framebuffer->SetGBufferAsTargetAndGetEncoder(commandBuffer);

			WriteAndSetFrameConsts<true, true>(m_active_camera, renderEncoder);

			#ifdef RENDER_GBUFFER
			RenderWithCamera(m_active_camera, renderEncoder, m_visible_meshes[0], 0, 0, 0, 0, 0);
			#endif // RENDER_GBUFFER
			
			[renderEncoder endEncoding];

			releaseObj(renderEncoder);
		}
		
		#ifdef MOVE_PARTICLES
		#pragma mark MoveParticles - XFB particles and stuff
		{
			MoveParticles(commandBuffer);
		}
		#endif // MOVE_PARTICLES
        
        #ifdef ENABLE_LIGHTNING_EFFECT
        RunLightningEffectPass1(commandBuffer) ;
        #endif
		
		#ifdef ISSUE_OCCLUSION_QUERIES
		#pragma mark Occlusion Queries - Issue queries for lens flares
		{
			IssueQueries(commandBuffer);
		}
		#endif

		// Light sky pass
		{
			id <MTLRenderCommandEncoder> lightSkyEncoder = m_framebuffer->SetLightSkyBufferAsTargetAndGetEncoder(commandBuffer);

			WriteAndSetFrameConsts<true, true>(m_active_camera, lightSkyEncoder);

			#ifdef RENDER_LIGHTS
            
            DoLightingPass(lightSkyEncoder) ;
            
			#endif // RENDER_LIGHTS

			#ifdef RENDER_SKY
			{
				MTLViewport viewport = {0, 0, 1.0f*m_framebuffer->GetWidth(), 1.0f*m_framebuffer->GetHeight(), 0, 1};
				[lightSkyEncoder setViewport:viewport];

				RenderWithCamera(m_active_camera, lightSkyEncoder, m_sky_mesh, 0, 0, 0, 0, 0);
			}
			#endif // RENDER_SKY

			[lightSkyEncoder endEncoding];

			releaseObj(lightSkyEncoder);
		}
        
        
        #ifdef ENABLE_LIGHTNING_EFFECT
        RunLightningEffectPass2(commandBuffer) ;
        #endif

        
#ifndef RENDER_POSTPROCESSING
        //want to get the drawable as late as possible
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
#endif
        
		
		#pragma mark Main Pass (Reflection/Emission Shadow Decals Particles Lightshafts Transparency)
		{
#ifdef RENDER_POSTPROCESSING
			id <MTLRenderCommandEncoder> renderEncoder = m_framebuffer->SetMainBufferAsTargetAndGetEncoder(commandBuffer);
#else
            id <MTLRenderCommandEncoder> renderEncoder = m_framebuffer->SetOnScreenMainBufferAsTargetAndGetEncoder(commandBuffer,frameBufferTexture);
#endif
            
			WriteAndSetFrameConsts<true, true>(m_active_camera, renderEncoder);
			
			#ifdef RENDER_REFLECTION_EMISSION_FILTER
			{
				MTLViewport viewport = {0, 0, float(m_viewport_width), float(m_viewport_height), 0, 1};
				[renderEncoder setViewport:viewport];
				[renderEncoder setDepthStencilState:m_DepthTestOffDepthWritesOff];

				m_framebuffer->SetRenderedTexturesForReflectionEmissionFilter(renderEncoder);

				m_reflection_emission_pipeline->Set(renderEncoder);
				
				[renderEncoder setCullMode:MTLCullModeNone];

				m_quadBuffer->Draw(renderEncoder);
			}
			#endif // RENDER_REFLECTION_EMISSION_FILTER

			#ifdef RENDER_DYNAMIC_SHADOW_DECALS
			{
				//depth test off, writes off
				[renderEncoder setDepthStencilState:m_DepthTestOffDepthWritesOff];
				[renderEncoder setCullMode:MTLCullModeFront];
				//the shadow pass needs the depth buffer, so bind the gbuffer and friends
				m_framebuffer->SetRenderedTexturesForLightPass(renderEncoder);
				
                KCL::Light* ll = m_light_factory.Create( "", 0, 0);

                ll->m_light_type = KCL::Light::SHADOW_DECAL;
				ll->m_radius = 1000.0f;
				
				RenderLight( ll,  renderEncoder, false);
                
                delete ll ;
			} 
			#endif // RENDER_DYNAMIC_SHADOW_DECALS

			#ifdef RENDER_STATIC_SHADOW_DECALS
			{
				RenderWithCamera(m_active_camera, renderEncoder, m_visible_meshes[2], 0, 0, 0, 0, 0);
			}
			#endif // RENDER_STATIC_SHADOW_DECALS
            
            #ifdef ENABLE_LIGHTNING_EFFECT
            RenderLightningEffect(renderEncoder) ;
            #endif
			
			#ifdef RENDER_PARTICLES
			#pragma mark RenderParticles - Draw particles
			{
				RenderParticles(renderEncoder);
			}
			#endif // RENDER_PARTICLES

			#ifdef RENDER_LIGHTSHAFTS
			RenderLightShafts(renderEncoder);
			#endif //RENDER_LIGHTSHAFTS
			
			#ifdef RENDER_TRANSPARENT_MESHES
			if(m_visible_meshes[1].size() > 0)
			{
				RenderWithCamera(m_active_camera, renderEncoder, m_visible_meshes[1], 0, 0, 0, 0, 0);
			}
			#endif
			
			#ifdef RENDER_LENSFLARES
			{
				RenderLensFlares(renderEncoder);
			}
			#endif
			
			[renderEncoder endEncoding];
			
			releaseObj(renderEncoder);
		}
		
        #ifdef RENDER_POSTPROCESSING
		//want to get the drawable as late as possible
        id <MTLTexture> frameBufferTexture = dynamic_cast<GLB::FBOMetalBase*>(GLB::FBO::GetLastBind())->GetTexture() ;
        m_framebuffer->UpdateFinalFrameBufferColorBuffer(frameBufferTexture);
		
        RunPostProcess(commandBuffer) ;

		RenderPostProcess(commandBuffer);
		#endif
		
		unsigned char slot = m_dynamicDataBufferPool->GetCurrentSlot();
        
        
        //to to assign members to temporaries in order to capture correctly in the block
        uint32_t tempQueryIndex = m_currentQueryBufferIndex;
        uint32_t tempFrameNumber = m_frame;
		
		
		#pragma mark Frame End callback
		[commandBuffer addCompletedHandler:^(id <MTLCommandBuffer> completedCommandBuffer)
		{
#if TARGET_OS_IPHONE
            m_occlusionQueries[tempQueryIndex].m_frame = tempFrameNumber;
            m_occlusionQueries[tempQueryIndex].m_queriesRetired = 1;
#endif
            
			m_dynamicDataBufferPool->MarkSlotUnused(slot);
		}];
        
#if !TARGET_OS_IPHONE
		[commandBuffer commit];

		// separated into another command buffer because we don't want to stall the next frame
		// ie, we want to begin encoding without waiting on the pageoff of these occlusion queries
		commandBuffer = [m_commandQueue commandBufferWithUnretainedReferences];
#endif
        
        #ifdef ISSUE_OCCLUSION_QUERIES
        {
            CloseQueryBuffers(commandBuffer) ;
        }
        #endif
        
        
#if !TARGET_OS_IPHONE
		[commandBuffer addCompletedHandler:^(id<MTLCommandBuffer> _Nonnull) {
		    m_occlusionQueries[tempQueryIndex].m_frame = tempFrameNumber;
		    m_occlusionQueries[tempQueryIndex].m_queriesRetired = 1;
		}];
#endif

		[commandBuffer commit];
		

		uint64_t endFrameTime = mach_absolute_time();
		// Enxure that the command buffer submission has not errored
		uint64_t frameDuration = endFrameTime - startFrameTime;
		m_totalDuration += frameDuration;
		
    } // autoreleasepool
}

#pragma mark Shader Loading
KCL::KCL_Status MTL_Scene_30::reloadShaders()
{
	assert(m_test_id != "");
	
	std::string lighting_fs_shader;
	if ( (m_test_id == "metal_manhattan311_wqhd_off") ||
		 (m_test_id == "metal_manhattan311_fixed_wqhd_off") )
	{
		lighting_fs_shader = "lighting_wqhd.fs";
	}
	else
	{
		lighting_fs_shader = "lighting.fs";
	}
	printf("use lighting shader: %s\n",lighting_fs_shader.c_str());
	
    KCL::KCL_Status result = MTL_Scene_Base::reloadShaders();
    
    if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
    
    assert( (m_scene_version == KCL::SV_30) || (m_scene_version == KCL::SV_31) );

    if( (m_scene_version == KCL::SV_30) || (m_scene_version == KCL::SV_31) )
    {
        std::set< std::string> default_defines;
        default_defines.insert( GetVersionStr() ) ;
        
        std::set< std::string> defines = default_defines;
        
        
        //
        //  Lighting Vertex Layout
        //
        
        int lightingBufferStride = 3 * sizeof(float);
        MTLVertexDescriptor* lightingVertexDesc = [[MTLVertexDescriptor alloc] init];
        
        
        lightingVertexDesc.attributes[0].format = MTLVertexFormatFloat3;
        lightingVertexDesc.attributes[0].bufferIndex = 0;
        lightingVertexDesc.attributes[0].offset = 0;
        
        lightingVertexDesc.layouts[0].stride = lightingBufferStride ;
        lightingVertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        
        
        //
        //  Fog Vertex Layout
        //
        
        int fogBufferStride = 3 * sizeof(float);
        MTLVertexDescriptor* fogVertexDesc = [[MTLVertexDescriptor alloc] init];
        
        
        fogVertexDesc.attributes[0].format = MTLVertexFormatFloat3;
        fogVertexDesc.attributes[0].bufferIndex = 0;
        fogVertexDesc.attributes[0].offset = 0;
        
        fogVertexDesc.layouts[0].stride = fogBufferStride ;
        fogVertexDesc.layouts[0].stepFunction = MTLVertexStepFunctionPerVertex;
        

        defines = default_defines;
        defines.insert( "POINT_LIGHT");
        defines.insert( "NEED_HIGHP");
        if (m_scene_version == KCL::SV_31)
        {
            defines.insert("INSTANCING") ;
        }
        
        MetalRender::Pipeline::GFXPipelineDescriptor lighting_pipeline_desc(MetalRender::Pipeline::UNKNOWN,
                                                                            m_default_shader_type,
                                                                            lightingVertexDesc,
                                                                            true,
                                                                            m_force_highp);

        lighting_pipeline_desc.blendType = MetalRender::Pipeline::ONE_X_ONE ;
        m_lighting_pipelines[LIGHT_POINT] = MetalRender::Pipeline::CreatePipeline("lighting.vs", lighting_fs_shader.c_str(), &defines, lighting_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
        
        if (m_scene_version == KCL::SV_30)
        {
        lighting_pipeline_desc.blendType = MetalRender::Pipeline::DISABLED;
        m_lighting_pipelines[LIGHT_POINT_NOBLEND] = MetalRender::Pipeline::CreatePipeline("lighting.vs", lighting_fs_shader.c_str(), &defines, lighting_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
        }
        
        defines = default_defines;
        defines.insert( "SPOT_LIGHT");
        defines.insert( "NEED_HIGHP");
        if (m_scene_version == KCL::SV_31)
        {
            defines.insert("INSTANCING") ;
        }
        
        lighting_pipeline_desc.blendType = MetalRender::Pipeline::ONE_X_ONE ;
        m_lighting_pipelines[LIGHT_SPOT] = MetalRender::Pipeline::CreatePipeline("lighting.vs", lighting_fs_shader.c_str(), &defines, lighting_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;

        
        if (m_scene_version == KCL::SV_30)
        {
        lighting_pipeline_desc.blendType = MetalRender::Pipeline::DISABLED;
        m_lighting_pipelines[LIGHT_SPOT_NOBLEND] = MetalRender::Pipeline::CreatePipeline("lighting.vs", lighting_fs_shader.c_str(), &defines, lighting_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
        }

        defines = default_defines;
        lighting_pipeline_desc.blendType = MetalRender::Pipeline::COLOR_ALPHA_X_ZERO ;
        m_lighting_pipelines[LIGHT_SHADOW_DECAL] = MetalRender::Pipeline::CreatePipeline("lighting.vs", "shadow_decal.fs", &defines, lighting_pipeline_desc, result);
        
        releaseObj(lightingVertexDesc) ;
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;

        MTLDepthStencilDescriptor *lightDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        lightDepthStateDesc.depthWriteEnabled = NO;
        lightDepthStateDesc.depthCompareFunction = MTLCompareFunctionGreaterEqual;

        m_lightDepthState = [m_Device newDepthStencilStateWithDescriptor:lightDepthStateDesc];

        releaseObj(lightDepthStateDesc);

        defines = default_defines;
        
        MetalRender::Pipeline::GFXPipelineDescriptor reflection_emission_pipeline_desc(MetalRender::Pipeline::DISABLED,m_default_shader_type,
                                                                                       m_quadBufferVertexLayout, true, m_force_highp);
        m_reflection_emission_pipeline =
            MetalRender::Pipeline::CreatePipeline("pp.vs", "re.fs", &defines, reflection_emission_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;

        MTLDepthStencilDescriptor *lightShaftDepthStateDesc = [[MTLDepthStencilDescriptor alloc] init];
        lightShaftDepthStateDesc.depthWriteEnabled = NO;
        lightShaftDepthStateDesc.depthCompareFunction = MTLCompareFunctionLess;
        
        m_lightShaftDepthState = [m_Device newDepthStencilStateWithDescriptor:lightShaftDepthStateDesc];

        MetalRender::Pipeline::GFXPipelineDescriptor fog_pipeline_desc(MetalRender::Pipeline::ALPHA_X_ONE_MINUS_SOURCE_ALPHA,
                                                                       m_default_shader_type,
                                                                       fogVertexDesc, true, m_force_highp) ;
        
        m_fog_pipeline =
            MetalRender::Pipeline::CreatePipeline("fog.vs", "fog.fs", &defines, fog_pipeline_desc, result);
        releaseObj(fogVertexDesc) ;
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;

        releaseObj(lightShaftDepthStateDesc);

        MetalRender::Pipeline::GFXPipelineDescriptor blurFilter_pipeline_desc(MetalRender::Pipeline::DISABLED, MetalRender::kShaderTypeSingleRGBA8Default,
                                                                              m_quadBufferVertexLayout, false, m_force_highp) ;
        m_blurFilter_pipeline = MetalRender::Pipeline::CreatePipeline("color_blur.vs", "color_blur.fs", &defines, blurFilter_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;

        MetalRender::Pipeline::GFXPipelineDescriptor subFilter_pipeline_desc(MetalRender::Pipeline::DISABLED, MetalRender::kShaderTypeSingleRGBA8Default,
                                                                             m_quadBufferVertexLayout, false, m_force_highp);
        m_subFilter_pipeline = MetalRender::Pipeline::CreatePipeline("pp.vs", "sub.fs", &defines, subFilter_pipeline_desc, result);
        
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
		
        MetalRender::Pipeline::GFXPipelineDescriptor particleAdvect_pipeline_desc(MetalRender::Pipeline::DISABLED, MetalRender::kShaderTypeTransformFeedback,
                                                                                  nullptr, false, m_force_highp);
		m_particleAdvect_pipeline = MetalRender::Pipeline::CreatePipeline( "particleAdvect.vs", "particleAdvect.fs", &defines, particleAdvect_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;

        MetalRender::Pipeline::GFXPipelineDescriptor occlusion_query_pipeline_desc(MetalRender::Pipeline::NO_COLOR_WRITES,MetalRender::kShaderTypeNoColorAttachment,
                                                                                   nullptr, true, m_force_highp);
		m_occlusion_query_pipeline = MetalRender::Pipeline::CreatePipeline( "oc.vs", "oc.fs", &defines, occlusion_query_pipeline_desc, result);
        
        if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
    }

    return result;
}


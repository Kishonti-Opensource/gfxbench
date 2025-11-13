/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_scene_31.h"
#include "metal/mtl_pipeline_builder.h"
#include "property.h"

typedef KCL::Vector4D hfloat4;

#include "pp2.h"
#include "pp_hdr.h"
#include "hdr_consts.h"

using namespace KCL ;


#define CAMERA_CONSTS_BFR_SLOT 12


KRL_Scene* MetalRender::CreateMTLScene31(const GlobalTestEnvironment* const gte)
{
    return new MTL_Scene_31(gte);
}


MTL_Scene_31::MTL_Scene_31(const GlobalTestEnvironment* const gte) : MTL_Scene_30(gte)
{
    m_compute_lightning = nullptr ;
    m_InstancedLightRenderer = nullptr ;
    m_hdr_pipeline = nullptr ;
    m_dof_blur = nullptr;
    m_compute_hdr = nullptr;
    
    m_ldr_color_buffer = nil ;
    
    MTLPipeLineBuilder::SetScene(KCL::SV_31, this);
}


MTL_Scene_31::~MTL_Scene_31()
{
    MTLPipeLineBuilder::SetScene(KCL::SV_INVALID, nullptr);
    
    m_hdr_pipeline = nullptr ;
    releaseObj(m_ldr_color_buffer) ;
    
    delete m_compute_lightning ;
    delete m_InstancedLightRenderer ;
    delete m_dof_blur;
    delete m_compute_hdr;
}


KCL::KCL_Status MTL_Scene_31::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
    PropertyLoader prr;
    std::string path = "manhattan31/scene_31.prop";
    std::vector<SerializeEntry> entries = prr.DeSerialize(m_ubo_frame, path);
    
    MTLTextureDescriptor *ldr_tex_desc = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatRGBA8Unorm
                                                                                        width:m_viewport_width
                                                                                       height:m_viewport_height
                                                                                    mipmapped:false];

    ldr_tex_desc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
	ldr_tex_desc.storageMode = MTLStorageModePrivate;
    
    m_ldr_color_buffer = [m_Device newTextureWithDescriptor:ldr_tex_desc];
    
    return MTL_Scene_30::Process_GL(color_mode, depth_mode, samples) ;
}


void MTL_Scene_31::Animate()
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
        if(delta_time == 0)
        {
            delta_time = 40; //40 ms to simulate 25 fps if animation is stopped
        }
    }
    m_last_animation_time = m_animation_time;
    
    m_ubo_frame.time_dt_pad2.x = m_animation_time * 0.001f;
    m_ubo_frame.time_dt_pad2.y = delta_time * 0.001f; // to seconds
    
    MTL_Scene_30::Animate();
}


void MTL_Scene_31::DoLightingPass(id <MTLRenderCommandEncoder> render_encoder)
{
    m_framebuffer->SetRenderedTexturesForLightPass(render_encoder);
    
    m_InstancedLightRenderer->m_num_lightning_lights = m_compute_lightning->GetLightCount();
    m_InstancedLightRenderer->m_num_omni_lights = 0;
    m_InstancedLightRenderer->m_num_spot_lights = 0;
    
    for( size_t i=0; i<m_visible_lights.size(); i++)
    {
        KCL::Light *l = m_visible_lights[i];
        float fov = KCL::Math::Rad( l->m_spotAngle);
        
        MTLInstancedLightRenderer::_instanced_light *cl;
        
        if( l->m_light_type == KCL::Light::OMNI)
        {
            cl = &m_InstancedLightRenderer->m_omni_lights[m_InstancedLightRenderer->m_num_omni_lights];
            m_InstancedLightRenderer->m_num_omni_lights++;
            
            cl->model.identity();
            cl->model.translate( Vector3D( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14]));
            cl->model.scale( Vector3D( l->m_radius * 1.25f, l->m_radius * 1.25f, l->m_radius * 1.25f) );
            
        }
        else
        {
            cl = &m_InstancedLightRenderer->m_spot_lights[m_InstancedLightRenderer->m_num_spot_lights];
            m_InstancedLightRenderer->m_num_spot_lights++;
            
            float halfSpotAngle = Math::Rad(0.5f * l->m_spotAngle);
            
            float scalingFactorX = KCL::Vector3D(l->m_world_pom.v[0], l->m_world_pom.v[1], l->m_world_pom.v[2]).length();
            float scalingFactorY = KCL::Vector3D(l->m_world_pom.v[4], l->m_world_pom.v[5], l->m_world_pom.v[6]).length();
            float scalingFactorZ = KCL::Vector3D(l->m_world_pom.v[8], l->m_world_pom.v[9], l->m_world_pom.v[10]).length();
            
            assert(fabs(scalingFactorX - scalingFactorY) < 0.001f);
            assert(fabs(scalingFactorY - scalingFactorZ) < 0.001f);
            assert(fabs(scalingFactorX - scalingFactorZ) < 0.001f);
            
            cl->model.zero();
            cl->model.v33 = l->m_radius * (1.0f / scalingFactorZ);
            cl->model.v11 = cl->model.v22 = cl->model.v33 * tanf(halfSpotAngle) * 1.2f; //1.2 is: extra opening to counter low tess-factor of the cone
            cl->model.v43 = -cl->model.v33;	// Translate so the top is at the origo
            cl->model.v44 = 1;
            cl->model *= l->m_world_pom;
        }
        
        
        cl->color.set( l->m_diffuse_color.x, l->m_diffuse_color.y, l->m_diffuse_color.z, 1.0f);
        cl->position.set( l->m_world_pom.v[12], l->m_world_pom.v[13], l->m_world_pom.v[14], (l->m_light_type == KCL::Light::OMNI ? 0 : 1));
        cl->atten_parameters.set( -1.0f / (l->m_radius * l->m_radius), 0,0,0);
        cl->dir.set( -l->m_world_pom.v[8], -l->m_world_pom.v[9], -l->m_world_pom.v[10], l->m_world_pom.v[14]);
        cl->spot.set( cosf( fov * 0.5), 1.0f / ( 1.0f - cosf( fov * 0.5f)), 1, 1);
    }
    
    //  Set before the lighting pass in the scene cpp. See m_lightSkyFrameBuffer
    //  glClearColor( 0.0f, 0.0f, 0.0f, 0.0f);
    //  glClear( GL_COLOR_BUFFER_BIT);
    
    //  Upload Camera Consts
    [render_encoder setVertexBytes:  &m_active_camera->GetViewProjection()[0] length:sizeof(KCL::Matrix4x4) atIndex:CAMERA_CONSTS_BFR_SLOT] ;
    [render_encoder setFragmentBytes:&m_active_camera->GetViewProjection()[0] length:sizeof(KCL::Matrix4x4) atIndex:CAMERA_CONSTS_BFR_SLOT] ;
    
    m_InstancedLightRenderer->Draw(render_encoder, m_active_camera, m_lbos, m_lighting_pipelines);
}


void MTL_Scene_31::RunLightningEffectPass1(id <MTLCommandBuffer> commandBuffer)
{
    for(unsigned int i=0; i<m_actors.size(); ++i)
    {
        if(m_actors[i]->m_name.find("robot") != std::string::npos)
        {
            m_compute_lightning->RunPass1(m_animation_time,  m_actors[i], commandBuffer, m_dynamicDataBuffer);
        }
    }
}


void MTL_Scene_31::RunLightningEffectPass2(id <MTLCommandBuffer> commandBuffer)
{
    m_compute_lightning->RunPass2(m_active_camera, commandBuffer, m_dynamicDataBuffer) ;
}


void MTL_Scene_31::RenderLightningEffect(id <MTLRenderCommandEncoder> render_encoder)
{
    m_compute_lightning->Draw(m_active_camera, render_encoder);
}


void MTL_Scene_31::RunPostProcess(id <MTLCommandBuffer> commandBuffer)
{
    //
    //  Reduction, bloom pass
    //
    m_compute_hdr->SetInputTexture( m_framebuffer->GetMainColorBuffer() ); //GL_RGB10_A2 is fed to the compute shader
    m_compute_hdr->Execute(commandBuffer, m_ubo_frame);
    
    
    //
    //  HDR pass
    //
    
    MTLRenderPassDescriptor* hdrPassDescriptor = [[MTLRenderPassDescriptor alloc] init];
    
    hdrPassDescriptor.colorAttachments[0].texture = m_ldr_color_buffer ;
    hdrPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionDontCare ;
    hdrPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore ;
    
    id <MTLRenderCommandEncoder> render_encoder = [commandBuffer renderCommandEncoderWithDescriptor:hdrPassDescriptor] ;
    releaseObj(hdrPassDescriptor) ;
    
    m_hdr_pipeline->Set(render_encoder) ;
    
    HDRConsts hdr_consts ;
    hdr_consts.ABCD = m_ubo_frame.ABCD ;
    hdr_consts.EFW_tau = m_ubo_frame.EFW_tau ;
    hdr_consts.exposure_bloomthreshold_minmax_lum = m_ubo_frame.exposure_bloomthreshold_pad2 ;
    
    [render_encoder setFragmentBytes:&hdr_consts length:sizeof(HDRConsts) atIndex:PP_HDR_HDRCONTS_BFR_SLOT] ;
    [render_encoder setFragmentBuffer:m_compute_hdr->GetLuminanceBuffer() offset:0 atIndex:PP_HDR_LUMINANCE_BFR_SLOT] ;
    
    [render_encoder setFragmentTexture:m_framebuffer->GetMainColorBuffer() atIndex:PP_HDR_HDR_COLOR_TEXTURE_SLOT] ;
    [render_encoder setFragmentTexture:m_compute_hdr->GetBloomTexture()    atIndex:PP_HDR_BLOOM_TEXTURE_SLOT] ;
    [render_encoder setFragmentSamplerState:m_compute_hdr->GetBloomSampler() atIndex:PP_HDR_BLOOM_SAMPLER_SLOT] ;
    
    m_finalBlitQuadBuffer->Draw(render_encoder);
    
    [render_encoder endEncoding];
    
    
    //
    //  Fragment blur for the DoF effect
    //
    m_dof_blur->SetInputTexture( m_ldr_color_buffer );
    m_dof_blur->Execute(commandBuffer);
}


void MTL_Scene_31::RenderPostProcess(id <MTLCommandBuffer> commandBuffer)
{
    //
    //  DoF effect pass
    //
    id <MTLRenderCommandEncoder> renderEncoder = m_framebuffer->SetFinalBufferAsTargetAndGetEncoder(commandBuffer);
    
    WriteAndSetFrameConsts<true, true>(m_active_camera, renderEncoder);
    
    float depthOfFieldConstants[2] = { m_active_camera->GetFov(), m_camera_focus_distance };
    
    m_dynamicDataBuffer->WriteAndSetData<true, true>(renderEncoder, DEPTH_OF_FIELD_CONST_INDEX, depthOfFieldConstants, sizeof(depthOfFieldConstants));
    
    [renderEncoder setFragmentTexture:m_ldr_color_buffer atIndex:PP2_MAIN_COLOR_TEX_SLOT] ;
    [renderEncoder setFragmentTexture:m_framebuffer->GetDepthBuffer()     atIndex:PP2_DEPTH_TEX_SLOT] ;
    [renderEncoder setFragmentTexture:m_dof_blur->GetOutputTexture()      atIndex:PP2_BLURED_COLOR_TEX_SLOT] ;
    
    [renderEncoder setDepthStencilState:m_blitDepthState];
    
    m_depthOfField_pipeline->Set(renderEncoder);
    
    m_finalBlitQuadBuffer->Draw(renderEncoder);
    
    [renderEncoder endEncoding];
}


KCL::KCL_Status MTL_Scene_31::reloadShaders()
{
    KCL::KCL_Status result = MTL_Scene_30::reloadShaders();
    
    if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
    
    std::set< std::string> defines;
    
    MetalRender::Pipeline::GFXPipelineDescriptor depthOfField_pipeline_desc(MetalRender::Pipeline::DISABLED,MetalRender::kShaderTypeSingleBGRA8,
                                                                            m_quadBufferVertexLayout, false, m_force_highp);
    
    m_depthOfField_pipeline = MetalRender::Pipeline::CreatePipeline("pp2.vs", "pp2.fs", &defines, depthOfField_pipeline_desc, result);
    
    if (result != KCL::KCL_TESTERROR_NOERROR) return result ;
    
    MTLPipeLineBuilder sb ;
    sb.ShaderFile("pp_hdr.shader") ;
    sb.SetVertexLayout(m_quadBufferVertexLayout) ;
    m_hdr_pipeline = sb.Build(result) ;
    
    {
        delete m_InstancedLightRenderer;
        m_InstancedLightRenderer = new MTLInstancedLightRenderer(m_Device, m_dynamicDataBuffer);
        m_InstancedLightRenderer->Init();
    }
    
    {
        delete m_compute_lightning ;
        m_compute_lightning = new MTLComputeLightning(m_Device) ;
        m_compute_lightning->Init(m_InstancedLightRenderer->GetLightningInstanceDataVBO(), m_default_shader_type) ;
    }
    
    {
        int dof_blur_strength = 9;
        
        // on iPhone the rendertarget is in portrait mode
        KCL::uint32 m_dim = KCL::Min(m_viewport_height,m_viewport_width);
        dof_blur_strength = (dof_blur_strength*m_dim) / 1080; // normalize for actual resolution
        
        delete m_dof_blur;
        m_dof_blur = new MetalRender::FragmentBlur(m_Device);
        m_dof_blur->Init(m_viewport_width, m_viewport_height, dof_blur_strength, MTLPixelFormatRGBA8Unorm, 1);
    }
    
    {
        delete m_compute_hdr;
        m_compute_hdr = new ComputeHDR31(m_Device);
        m_compute_hdr->Init(m_viewport_width,m_viewport_height, MTLPixelFormatRGBA8Unorm, this);
    }
    
    return result;
}



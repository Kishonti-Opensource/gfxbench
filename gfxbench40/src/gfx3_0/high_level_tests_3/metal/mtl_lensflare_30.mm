/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "mtl_scene_30.h"
#include "mtl_dynamic_data_buffer.h"
#include "mtl_shader_constant_layouts_30.h"
#include "mtl_pipeline.h"

#include "mtl_light.h"

//NOTE:
//
//	Currently the boolean occlusion query does NOT set 0 if it fails
//	All it does is write a non-zero value in the slot if it passes
//	So we MUST reset it by hand


void MTL_Scene_30::IssueQueries(id <MTLCommandBuffer> cmdBuf)
{
    // __builtin_printf("frame %u query index %u\n", m_frame, m_currentQueryBufferIndex);

#if !TARGET_OS_IPHONE
	//reset all occlusion queries - must be one with blit so it's inlined
    {
        id <MTLBlitCommandEncoder> blitEnc = [cmdBuf blitCommandEncoder];
        [blitEnc fillBuffer:m_occlusionQueries[m_currentQueryBufferIndex].m_queryResults range:NSMakeRange(0, m_occlusionQueries[m_currentQueryBufferIndex].m_size) value:0x0];
        [blitEnc endEncoding];
    }
    m_occlusionQueries[m_currentQueryBufferIndex].m_queriesRetired = 0;
#endif

    id <MTLRenderCommandEncoder> enc = m_framebuffer->SetOcclusionQueryBufferAndGetEncoder(cmdBuf, m_currentQueryBufferIndex);
    
    //don't want to stomp this data
    assert(m_frame >= m_occlusionQueries[m_currentQueryBufferIndex].m_frame);
    
#if TARGET_OS_IPHONE
    m_occlusionQueries[m_currentQueryBufferIndex].m_queriesRetired = NO;
    
    //reset all occlusion queries
    bzero([m_occlusionQueries[m_currentQueryBufferIndex].m_queryResults contents], m_occlusionQueries[m_currentQueryBufferIndex].m_size);
#endif
    
    meshConsts meshConstants;
    meshConstants.mvp = m_active_camera->GetViewProjection();
    m_lensFlareDataBuffer->WriteAndSetData<true, false>(enc, MESH_CONST_INDEX, &meshConstants, sizeof(meshConsts));
    
    //pipeline turns color writes off, but keeps rasterization on
    m_occlusion_query_pipeline->Set(enc);
    //depth writes off, depth test ON
    [enc setDepthStencilState:m_particleDepthStencilState];
    
    for (int i = 0; i < m_visible_lights.size(); i++)
    {
        MetalRender::Light* l = dynamic_cast<MetalRender::Light*>(m_visible_lights[i]);
        
        if( !l->m_has_lensflare)
        {
            continue;
        }
        
        if( l->m_light_type == KCL::Light::AMBIENT || l->m_light_type == KCL::Light::DIRECTIONAL || l->m_diffuse_color == KCL::Vector3D() )
        {
            continue;
        }
        
        KCL::Vector4D pos_buffer(l->m_world_pom.v[12],l->m_world_pom.v[13],l->m_world_pom.v[14],1.0) ;
        m_lensFlareDataBuffer->WriteAndSetData<true, false>(enc, 0, &pos_buffer, sizeof(KCL::Vector4D));
        
        //the offset was calculated when the scene was loaded
#if TARGET_OS_IPHONE
        [enc setVisibilityResultMode:MTLVisibilityResultModeBoolean offset:l->m_query_objects[0]*sizeof(uint64_t)];
#else
        [enc setVisibilityResultMode:MTLVisibilityResultModeCounting offset:l->m_query_objects[0]*sizeof(uint64_t)];
#endif
        [enc drawPrimitives:MTLPrimitiveTypePoint vertexStart:VERTEX_DATA_INDEX vertexCount:1];
    }
    
    [enc endEncoding];
}


void MTL_Scene_30::RenderLensFlares(id<MTLRenderCommandEncoder> enc)
{
    uint64_t* results = NULL;
    //want last frame's queries. maybe add more?
    while(!m_occlusionQueries[m_lastQueryBufferIndex].m_queriesRetired)
    {
        //spin while the queries aren't ready
        //printf("PERFORMANCE WARNING: lensflare draw busy wait!!!\n");
    }
    
    results = reinterpret_cast<uint64_t*>([m_occlusionQueries[m_lastQueryBufferIndex].m_queryResults contents]);
    
    assert(results);
    
    for( KCL::uint32 i=0; i<m_visible_lights.size(); i++)
    {
        MetalRender::Light *l = dynamic_cast<MetalRender::Light*>(m_visible_lights[i]);
        
        if (l->m_has_lensflare)
        {
            uint64_t visible = results[l->m_query_objects[0]];
            
            if(visible)
            {
                assert(l->visible_meshes[0].size() == 0);
                l->visible_meshes[0].push_back( m_lens_flare_mesh);
                
                RenderWithCamera(m_active_camera, enc, l->visible_meshes[0], 0, 0, 0, l, 0);
                
                l->visible_meshes[0].clear();
            }
        }
    }
}


void MTL_Scene_30::CloseQueryBuffers(id <MTLCommandBuffer> commandBuffer)
{
#if !TARGET_OS_IPHONE
    //page data back so the CPU can read the occlusion results in a frame or two
    {
        id <MTLBlitCommandEncoder> pageOffBlitEnc = [commandBuffer blitCommandEncoder];
        [pageOffBlitEnc synchronizeResource:m_occlusionQueries[m_currentQueryBufferIndex].m_queryResults];
        [pageOffBlitEnc endEncoding];
    }
#endif

    //switch occlusion query
    m_lastQueryBufferIndex = (m_lastQueryBufferIndex + 1) % MetalRender::Light::QUERY_COUNT;
    m_currentQueryBufferIndex = (m_currentQueryBufferIndex + 1) % MetalRender::Light::QUERY_COUNT;
}





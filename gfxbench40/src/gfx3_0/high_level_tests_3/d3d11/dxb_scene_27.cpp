/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_scene.h"
#include "dxb_scene_27.h"

#include "dxb_planarmap.h"
#include "d3d11/dxb_image.h"
#include "dxb_material.h"
#include "dxb_mesh.h"
#include "d3d11/vbopool.h"
#include "d3d11/shader.h"
#include "d3d11/fbo3.h"

void DXB_Scene_27::Render()
{
	std::vector<KCL::MeshInstanceOwner*>::iterator mioi = m_mios.begin();

	while( mioi != m_mios.end())
	{
		KCL::MeshInstanceOwner *mio = *mioi;

		if( mio->IsNeedUpdate() )
		{
			for( KCL::uint32 j = 0; j < 2; j++)
			{
				IndexBufferPool::Instance()->SubData( 
					mio->m_current_vertex_indices[j].size() * 2,
					&mio->m_current_vertex_indices[j][0],
					mio->m_mesh->m_mesh->m_ebo[j].m_buffer,
					(KCL::uint32)mio->m_mesh->m_mesh->m_ebo[j].m_offset
					);
			}
		}

		mioi++;
	}




	//DX

    DX::getContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_constantBuffer->bind(0);
	m_constantBufferParticle->bind(1);
	m_constantBufferMBlur->bind(2);

	DX::getContext()->PSSetSamplers(
        0,
        1,
        m_linearSampler.GetAddressOf()
        );
	DX::getContext()->PSSetSamplers(
        1,
        1,
        m_linearSamplerClamp.GetAddressOf()
        );
	
	//----------------------------------------------

	m_num_draw_calls = 0;
	m_num_triangles = 0;
	m_num_vertices = 0;

	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		if( m_global_shadowmaps[i])
		{
			RenderShadow( ((ShadowMap*)m_global_shadowmaps[i]));
		}
	}

	m_constantBuffer->bind(0);

	for( KCL::uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
		RenderPlanar( m_visible_planar_maps[i]);
	}

	m_constantBuffer->bind(0);

	GLB::FBO* main_fbo = m_main_fbo;
	if (m_mblur_enabled)
	{
		GLB::FBO::bind(main_fbo);
		GLB::FBO::discard(main_fbo);
	} 
	else
	{
		//FBO::discard(NULL);
		GLB::FBO::clear(NULL,0,0,0);
	}
	
	if(UseZPrePass())
		RenderPrepass(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0);
	else
		DXB_Scene::Render(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0);

	DXB_Scene::Render(m_active_camera, m_visible_meshes[1], 0, 0, 0, 0);

	for( KCL::uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
		dynamic_cast<DXB::Material*>(m_planarReflectionMaterial)->setPlanarMap(dynamic_cast<DXB::PlanarMap*>(m_visible_planar_maps[i])); 
		m_planarReflectionMaterial->m_transparency = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_reflect_intensity;
		m_planarReflectionMaterial->m_textures[2] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[2];
		m_planarReflectionMaterial->m_textures[3] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[3];

		DXB_Scene::Render(m_active_camera, m_visible_planar_maps[i]->m_receiver_meshes, m_planarReflectionMaterial, 0, 0, 0, 0);

		m_planarReflectionMaterial->m_textures[2] = 0;
		m_planarReflectionMaterial->m_textures[3] = 0;
	}

	if( m_num_shadow_maps )
	{
		DXB_Scene::Render(m_active_camera, m_shadowStaticReceiverMeshes, m_shadowStaticReceiverMaterial, 0, 0, 0, 0);
	}
	
	if (m_mblur_enabled)
	{
		GLB::FBO::discardDepth(NULL);
		GLB::FBO* mblur_fbo = m_mblur_fbo;
		GLB::FBO::bind(mblur_fbo);
		GLB::FBO::clear(mblur_fbo, 0.5f, 0.5f, 0.0f, 1.0f);

		DXB_Scene::Render(m_active_camera, m_motion_blurred_meshes, m_mblurMaterial, 0, 0, 0);

		GLB::FBO::discardDepth(mblur_fbo);
		GLB::FBO::bind(NULL);
		GLB::FBO::clear(NULL,0,0,0);
		//FBO::discard(NULL);

		m_blur_shader->Bind();
	
		ID3D11ShaderResourceView* textureView0 = main_fbo->Get();
		DX::getContext()->PSSetShaderResources(0, 1, &textureView0);
	
		ID3D11ShaderResourceView* textureView1 = mblur_fbo->Get();
		DX::getContext()->PSSetShaderResources(1, 1, &textureView1);

		VboPool::Instance()->BindBuffer(m_fullscreen_vbo);
		IndexBufferPool::Instance()->BindBuffer(m_fullscreen_ebo);

		DX::getContext()->IASetPrimitiveTopology( D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DX::getContext()->IASetInputLayout( m_fullscreen_inputLayout.Get());
		
		DX::getStateManager()->ApplyStates();
		DX::getContext()->DrawIndexed( 6, 0, 0);
	
		ID3D11ShaderResourceView*const textureView[1] = {NULL};
		DX::getContext()->PSSetShaderResources(0, 1, textureView);
		DX::getContext()->PSSetShaderResources(1, 1, textureView);
	}
}
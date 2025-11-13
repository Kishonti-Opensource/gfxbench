/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_material.h"

//#include "glb_mesh.h"
#include "d3d11/dxb_image.h"
#include "d3d11/shader.h"
#include "platform.h"


using namespace std;

Shader* DXB::Material::m_last_shader = 0;

void DXB::Material::postInit()
{
	DX::StateManager* manager = DX::getStateManager();
	manager->SetDepthEnabled(false);
	manager->SetDepthFunction(D3D11_COMPARISON_LESS);
	manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ALL);
	manager->SetViewportDepthrange();
	manager->SetBlendEnabled(false);
	manager->SetRasterCullMode(D3D11_CULL_NONE);
}

void DXB::Material::preInit(KCL::uint32 &texture_num, int type, int pass_type)
{
	/* force use of shader[2] for depth prepass */
	int shader_bank = (pass_type == -1) ? 1 : 0;
	int i = 8;

	m_last_shader = m_shaders[shader_bank][type];
	m_last_shader->Bind();

	while( i--)
	{
		if (!i && m_videoTexture)
		{
			m_videoTexture->bind(i);
			continue;
		}

		if( m_textures[i] )
		{
			m_textures[i]->bind(i);
			texture_num++;
		}
	}	

	if (m_texture_array)
	{
		m_texture_array->bind(m_textureArraySlot);
	}
	
	DX::StateManager* manager = DX::getStateManager();

	switch( m_material_type)
	{
	case SKY:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthFunction(D3D11_COMPARISON_LESS_EQUAL);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetViewportDepthrange(1.0f, 1.0f);
			break;
		}
	case WATER:
		{
			manager->SetDepthEnabled(true);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	case GLASS:
		{
			manager->SetDepthEnabled(true);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	case LIGHTSHAFT:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
			break;
		}
	case FOLIAGE:
		{
			manager->SetDepthEnabled(true);
			break;
		}
	case FLAME:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_OP_ADD);
			break;
		}
	case FIRE:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	case STEAM:
		{			
			manager->SetDepthEnabled(true);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	case SMOKE:
		{			
			manager->SetDepthEnabled(true);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	case DECAL:
		{			
			manager->SetDepthEnabled(true);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	case OMNILIGHT:
		{
			break;
		}
	case SHADOWCASTER0:
		{
			break;
		}
	case SHADOWRECEIVER0:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthFunction(D3D11_COMPARISON_EQUAL);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
			break;
		}
	case SHADOWCASTER1:
		{
			manager->SetDepthEnabled(true);
			manager->SetRasterCullMode(D3D11_CULL_BACK);
			break;
		}
	case SHADOWRECEIVER1:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthFunction(D3D11_COMPARISON_EQUAL);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ZERO, D3D11_BLEND_OP_ADD);
			break;
		}
	case PLANAR_REFLECTION:
		{
			manager->SetDepthEnabled(true);
			manager->SetDepthFunction(D3D11_COMPARISON_LESS_EQUAL);
			manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
			manager->SetBlendEnabled(true);
			manager->SetBlendFunction(D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_OP_ADD);
			break;
		}
	default:
		{
			manager->SetDepthEnabled(true);
			manager->SetRasterCullMode(D3D11_CULL_BACK);
		}
	}

	/* overrride for prepass rendered meshes */
	switch (pass_type)
	{
		/* depth prepass */
		case -1:
			/* NOTE: colour writes are disabled outside the state manager! */
			switch(m_material_type)
			{
				case FOLIAGE:
					manager->SetDepthEnabled(true);
					manager->SetDepthFunction(D3D11_COMPARISON_LESS);
					manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ALL);
					manager->SetRasterCullMode(D3D11_CULL_NONE);
				break;

				case DEFAULT:
					manager->SetDepthEnabled(true);
					manager->SetDepthFunction(D3D11_COMPARISON_LESS);
					manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ALL);
					manager->SetRasterCullMode(D3D11_CULL_BACK);
				break;
			}
		break;

		/* shade pass */
		case +1:
			switch(m_material_type)
			{
				case FOLIAGE:
					manager->SetDepthEnabled(true);
					manager->SetDepthFunction(D3D11_COMPARISON_EQUAL);
					manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
					manager->SetRasterCullMode(D3D11_CULL_NONE);
				break;

				case DEFAULT:
					manager->SetDepthEnabled(true);
					manager->SetDepthFunction(D3D11_COMPARISON_LESS_EQUAL);
					manager->SetDepthMask(D3D11_DEPTH_WRITE_MASK_ZERO);
					manager->SetRasterCullMode(D3D11_CULL_BACK);
				break;
			}
		break;

		default:
		break;
	}

}

void DXB::Material::LoadVideo( const char *filename)
{
	if (m_videoTexture)
	{
		delete m_videoTexture;
		m_videoTexture = NULL;
	}

	if (m_video)
	{
		delete m_video;
		m_video = NULL;
	}

	m_video = new _ogg_decoder(filename);
	m_videoTexture = new DXB::DXBTexture(m_video);
	m_videoTexture->commit();
}

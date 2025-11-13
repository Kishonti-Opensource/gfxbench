/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_material.h"

#include "glb_mesh.h"
#include "opengl/glb_image.h"
#include "opengl/shader.h"
#include "platform.h"
#include "opengl/glb_opengl_state_manager.h"

#include "xiph_decode/glb_ogg_decoder.h"


#if (defined __glew_h__ || (defined(__MACH__) && defined(__APPLE__))) && !defined IPHONE
#undef glDepthRangef
#define glDepthRangef glDepthRange
#endif

using namespace std;

#ifdef TEXTURE_COUNTING
static std::set<KCL::uint32> *textureCounterPtr = 0;
void GLB::Material::TextureCounter(std::set<KCL::uint32> &textureCounter)
{
	textureCounterPtr = &textureCounter;
}

void GLB::Material::NullTextureCounter()
{
	textureCounterPtr = 0;
}
#endif

void GLB::Material::preInit( KCL::uint32 &texture_num, int type, int pass_type)
{
	/* force use of shader[2] for depth prepass */
	int shader_bank = (pass_type == -1) ? 1 : 0;
	int i = 8;


	if(Shader::m_last_shader != m_shaders[shader_bank][type])
	{
		OpenGLStateManager::GlUseProgram( m_shaders[shader_bank][type]->m_p);
		Shader::m_last_shader = m_shaders[shader_bank][type];
	}

	while( i--)
	{
		if( m_shaders[shader_bank][type]->m_uniform_locations[uniforms::texture_unit0 + i] > -1 && m_textures[i] )
		{
            m_textures[i]->bind(texture_num);
			glUniform1i(m_shaders[shader_bank][type]->m_uniform_locations[uniforms::texture_unit0 + i], texture_num);
			texture_num++;

			#ifdef TEXTURE_COUNTING
			if(textureCounterPtr)
			{
				textureCounterPtr->insert( ((GLBTexture*)m_textures[i])->textureObject() );
			}
			#endif
		}

		if( m_ogg_decoder && i == 0)
		{
			glBindTexture( GL_TEXTURE_2D, m_ogg_decoder->m_tbo );

			#ifdef TEXTURE_COUNTING
			if(textureCounterPtr)
			{
				textureCounterPtr->insert( m_ogg_decoder->m_tbo );
			}
			#endif
		}
	}	

#if defined HAVE_GLES3 || defined __glew_h__
	i = 8;
	if (m_shaders[shader_bank][type]->m_uniform_locations[uniforms::texture_unit0 + i] > -1 && m_texture_array)
	{
        m_texture_array->bind(texture_num);

		glUniform1i(m_shaders[shader_bank][type]->m_uniform_locations[uniforms::texture_unit0 + i], texture_num);
		texture_num++;

		#ifdef TEXTURE_COUNTING
		if(textureCounterPtr)
		{
			textureCounterPtr->insert( ((GLBTexture*)m_texture_array)->textureObject() );
		}
		#endif
	}
#endif


	switch( m_material_type)
	{
	case SKY:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlDepthFunc( GL_LEQUAL);
			OpenGLStateManager::GlDepthMask( 0);
			glDepthRangef( 1, 1);
			break;
		}
	case WATER:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case GLASS:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			OpenGLStateManager::GlDepthMask( 0);
			break;
		}
	case LIGHTSHAFT:
		{
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlBlendFunc( 1, 1);
			break;
		}
	case FOLIAGE:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			break;
		}
	case FLAME:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( 1, 1);
			break;
		}
	case FIRE:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( 1, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case SMOKE:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case STEAM:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( 1, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case DECAL:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, GL_SRC_COLOR);
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
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlDepthFunc( GL_EQUAL);
			OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, 0);
			break;
		}
	case SHADOWCASTER1:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable(GL_CULL_FACE);
			OpenGLStateManager::GlCullFace( GL_BACK);
			break;
		}
	case SHADOWRECEIVER1:
		{			
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlDepthFunc( GL_EQUAL);
			OpenGLStateManager::GlBlendFunc( GL_DST_COLOR, 0);
			break;
		}
	case PLANAR_REFLECTION:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlDepthMask( 0);
			OpenGLStateManager::GlDepthFunc( GL_EQUAL);
			OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			break;
		}
	case XRAY:
		{
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			OpenGLStateManager::GlDisable( GL_CULL_FACE);
			OpenGLStateManager::GlEnable( GL_BLEND);
			OpenGLStateManager::GlBlendFunc( GL_ONE_MINUS_DST_COLOR, 1);
			OpenGLStateManager::GlDepthMask( 0);
			break;
		}
	default:
		{
			OpenGLStateManager::GlEnable( GL_CULL_FACE);
			OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
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
					OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
					OpenGLStateManager::GlDepthFunc(GL_LESS);
				break;

				case DEFAULT:
					OpenGLStateManager::GlEnable(GL_CULL_FACE);
					OpenGLStateManager::GlCullFace(GL_BACK);
					OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
					OpenGLStateManager::GlDepthFunc(GL_LESS);
				break;
                default:
                break;
			}
		break;

		/* shade pass */
		case +1:
			switch(m_material_type)
			{
				case FOLIAGE:
					OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
					OpenGLStateManager::GlDepthMask(0);
					OpenGLStateManager::GlDepthFunc(GL_EQUAL);
				break;

				case DEFAULT:
					OpenGLStateManager::GlEnable(GL_CULL_FACE);
					OpenGLStateManager::GlCullFace(GL_BACK);
					OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
					OpenGLStateManager::GlDepthMask(0);
					OpenGLStateManager::GlDepthFunc(GL_LEQUAL);
                break;
                default:
                break;
			}
		break;

		default:
		break;
	}
}

void GLB::Material::postInit()
{
	OpenGLStateManager::GlDepthFunc( GL_LESS);
	OpenGLStateManager::GlDepthMask( 1);
	OpenGLStateManager::GlCullFace( GL_BACK);
	OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	OpenGLStateManager::GlDisable( GL_CULL_FACE);
	OpenGLStateManager::GlDisable( GL_BLEND);
	glDepthRangef( 0, 1);
}


Shader* GLB::Material::CreateShader(const char* vsfile, const char* fsfile, const std::set<std::string> *defines, KCL::KCL_Status& error)
{
	return Shader::CreateShader(vsfile, fsfile, defines, error);
}

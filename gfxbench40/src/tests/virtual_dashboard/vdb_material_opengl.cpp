/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "vdb_material.h"

#include "glb_mesh.h"
#include "opengl/glb_image.h"
#include "opengl/shader.h"
#include "platform.h"
#include "opengl/glb_opengl_state_manager.h"

#if (defined __glew_h__ || (defined(__MACH__) && defined(__APPLE__))) && !defined IPHONE
#undef glDepthRangef
#define glDepthRangef glDepthRange
#endif

using namespace std;

#ifdef TEXTURE_COUNTING
static std::set<KCL::uint32> *textureCounterPtr = 0;
void VDB::Material::TextureCounter(std::set<KCL::uint32> &textureCounter)
{
	textureCounterPtr = &textureCounter;
}

void VDB::Material::NullTextureCounter()
{
	textureCounterPtr = 0;
}
#endif

void VDB::Material::preInit( KCL::uint32 &texture_num, int type, int pass_type)
{
	/* force use of shader[2] for depth prepass */
	int shader_bank = (pass_type == -1) ? 1 : 0;
	int i = 8;


	if(Shader::m_last_shader != m_shaders[shader_bank][type])
	{
		GLB::OpenGLStateManager::GlUseProgram( m_shaders[shader_bank][type]->m_p);
		Shader::m_last_shader = m_shaders[shader_bank][type];
	}

	while( i--)
	{
		if( m_shaders[shader_bank][type]->m_uniform_locations[100 + i] > -1 && m_textures[i] )
		{
            m_textures[i]->bind(texture_num);
			glUniform1i( m_shaders[shader_bank][type]->m_uniform_locations[100 + i], texture_num);
			texture_num++;

			#ifdef TEXTURE_COUNTING
			if(textureCounterPtr)
			{
				textureCounterPtr->insert( ((GLBTexture*)m_textures[i])->textureObject() );
			}
			#endif
		}

	}	

#if defined HAVE_GLES3 || defined __glew_h__
	i = 8;
	if( m_shaders[shader_bank][type]->m_uniform_locations[100 + i] > -1 && m_texture_array )
	{
        m_texture_array->bind(texture_num);

		glUniform1i( m_shaders[shader_bank][type]->m_uniform_locations[100 + i], texture_num);
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
	case XRAY:
		{
			GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
			GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
			GLB::OpenGLStateManager::GlEnable( GL_BLEND);
			GLB::OpenGLStateManager::GlBlendFunc( GL_ONE_MINUS_DST_COLOR, 1);
			GLB::OpenGLStateManager::GlDepthMask( 0);
			break;
		}
	default:
		{
			GLB::OpenGLStateManager::GlEnable( GL_CULL_FACE);
			GLB::OpenGLStateManager::GlEnable( GL_DEPTH_TEST);
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
				case DEFAULT:
					GLB::OpenGLStateManager::GlEnable(GL_CULL_FACE);
					GLB::OpenGLStateManager::GlCullFace(GL_BACK);
					GLB::OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
					GLB::OpenGLStateManager::GlDepthFunc(GL_LESS);
				break;
                default:
                break;
			}
		break;

		/* shade pass */
		case +1:
			switch(m_material_type)
			{
				case DEFAULT:
					GLB::OpenGLStateManager::GlEnable(GL_CULL_FACE);
					GLB::OpenGLStateManager::GlCullFace(GL_BACK);
					GLB::OpenGLStateManager::GlEnable(GL_DEPTH_TEST);
					GLB::OpenGLStateManager::GlDepthMask(0);
					GLB::OpenGLStateManager::GlDepthFunc(GL_LEQUAL);
                break;
                default:
                break;
			}
		break;

		default:
		break;
	}
}

void VDB::Material::postInit()
{
	GLB::OpenGLStateManager::GlDepthFunc( GL_LESS);
	GLB::OpenGLStateManager::GlDepthMask( 1);
	GLB::OpenGLStateManager::GlCullFace( GL_BACK);
	GLB::OpenGLStateManager::GlDisable( GL_DEPTH_TEST);
	GLB::OpenGLStateManager::GlDisable( GL_CULL_FACE);
	GLB::OpenGLStateManager::GlDisable( GL_BLEND);
	glDepthRangef( 0, 1);
}


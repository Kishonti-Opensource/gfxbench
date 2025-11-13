/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if 1
#include "glb_scene.h"

#include <kcl_light2.h>
#include <kcl_planarmap.h>
#include <kcl_room.h>
#include <kcl_actor.h>
#include <kcl_animation4.h>
#include <kcl_particlesystem2.h>

#include "glb_kcl_adapter.h"
#include "opengl/glb_image.h"
#include "glb_mesh.h"
#include "glb_material.h"

#include "opengl/shader.h"
#include "platform.h"
#include "cubemap.h"
#include "shadowmap.h"

#include "opengl/misc2_opengl.h"
#include "opengl/fbo.h"
#include "opengl/glbshader.h"
#include "opengl/texture.h"
#include "opengl/vbopool.h"

#include "misc2.h"
#include "kcl_io.h"

#include <cmath>

#include "opengl/glb_opengl_state_manager.h"

#include "cubemap.h"

using namespace GLB;

//#define STRIP_REDUNDANT_CLEARS

void GLB_Scene_ES2::Release_GLResources()
{
	if( m_particles_vbo)
	{
		glDeleteBuffers(1, &m_particles_vbo);
		m_particles_vbo = 0;
	}

	if ( m_fullscreen_quad_vbo )
	{
		glDeleteBuffers(1, &m_fullscreen_quad_vbo) ;
		m_fullscreen_quad_vbo = 0 ;
	}

#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
	//destroy dummy fbo stuff
	{
		delete m_dummyFbo;
		m_dummyFbo = 0;

		glDeleteProgram(m_dummy_program);
		m_dummy_program = 0;

		glDeleteBuffers(1, &m_dummy_vbo);
		glDeleteBuffers(1, &m_dummy_ebo);

		m_dummy_vbo = 0;
		m_dummy_ebo = 0;
		m_dummy_texture_unif_loc = -1;
	}
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
	delete m_glGLSamplesPassedQuery;
	m_glGLSamplesPassedQuery = 0;
#endif
	delete m_mblur_fbo;
	delete m_main_fbo;
}

bool GLB_Scene_ES2::UseEnvmapMipmaps()
{
	return false;
}

KCL::KCL_Status GLB_Scene_ES2::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
    IncrementProgressTo(0.5f);

	std::string required_render_api;

	if( m_scene_version == KCL::SV_30)
	{
		required_render_api = "es3";
	}
	else if ( m_scene_version == KCL::SV_31)
	{
		required_render_api = "es31";
	}
	else
	{
		required_render_api = "es2";
	}

	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;

    Shader::SetForceHIGHP(m_force_highp);
    IncrementProgressTo(0.51f);

	Shader::InitShaders( required_render_api, true);
    IncrementProgressTo(0.52f);

	if( m_mblur_enabled)
	{
		try
		{
			m_main_fbo = new GLB::FBO(m_viewport_width, m_viewport_height, samples, color_mode, depth_mode, "motion blur b0");
			m_mblur_fbo = new GLB::FBO( m_viewport_width, m_viewport_height, 0, RGB888_Linear, GLB::DEPTH_16_RB, "motion blur b1");
		}
		catch (...)
		{
			return KCL::KCL_TESTERROR_MOTIONBLUR_WITH_MSAA_NOT_SUPPORTED;
		}
	}
    IncrementProgressTo(0.53f);

	if( !m_fullscreen_quad_vbo)
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
	}
    IncrementProgressTo(0.54f);

	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		bool need_immutable = (m_scene_version == KCL::SV_30 || m_scene_version == KCL::SV_31);
		m_global_shadowmaps[i] = ShadowMap::Create(m_fboShadowMap_size, m_shadow_method_str, m_fullscreen_quad_vbo, true,need_immutable );
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
						mio->m_mesh= GetMeshFactory().Create("instance_owner", 0, 0);
						mio->m_mesh->m_material = room->m_meshes[j]->m_material;
						mio->m_mesh->m_mesh = m;

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
				if( room->m_meshes[j])
				{
					room->m_meshes[j]->DeleteUnusedAttribs();
				}
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
				INFO("Can not load texture: %s", newName.c_str());
                assert(0);
				return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
                
//				KCL::AssetFile combinedImage(newName);
//			
//				CombineData cd;
//                KCL::uint32 w = instances[0]->m_material->m_textures[1]->getImage()->getWidth();
//				KCL::uint32 h = instances[0]->m_material->m_textures[1]->getImage()->getWidth();
//				int wm, hm;
//				KCL::uint32 num_channels = instances[0]->m_material->m_textures[1]->getImage()->getBpp() / 8;
//
//				assert( num_channels > 0);
//
//				(*mio)->GetLightMapSizeMultiplier(wm, hm);
//
//				int newImageSizeW = w * wm;
//				int newImageSizeH = h * hm;
//
//				cd.Create( newImageSizeW, newImageSizeH, num_channels);
//				
//				for(unsigned int k = 0 ;k < instances.size();k++)
//				{
//                    KCL::Texture* image = KCL::Texture::CreateAndSetup( KCL::Texture_2D,  std::string(instances[k]->m_material->m_textures[1]->getImage()->getName()).c_str() );
//					
//                    for(unsigned int y = 0; y < h; y++)
//					{
//						for(unsigned int x = 0; x < w; x++)
//						{
//							float u = x;
//							float v = y;
//							(*mio)->ConvertLightmapUV(k,w,h,u,v);
//
//							for( KCL::uint32 k=0; k<num_channels; k++)
//							{
//                                cd.Put(u,newImageSizeH-v - 1,k,( (KCL::uint8*)(image->getImage()->getData()) )[(y * w + x ) * num_channels + k]);
//							}
//						}
//					}
//					delete image;
//				}
//
//				savePng(newName.c_str(), newImageSizeW , newImageSizeH, (unsigned char*&)cd.GetData(), 0);
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
			(*mio)->m_mesh->DeleteUnusedAttribs();
			mio++;
		}
		IncrementProgressTo(0.58f);

	}
	else
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

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	for( uint32 i=0; i<m_sky_mesh.size(); i++)
	{
		m_sky_mesh[i]->DeleteUnusedAttribs();
	}

	for(size_t i = 0; i < m_meshes.size(); ++i)
	{
		if( KCL::g_os->LoadingCallback( 0) != KCL::KCL_TESTERROR_NOERROR) break;
		m_meshes[i]->InitVertexAttribs();
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
		m_materials[i]->InitImages();
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
	bool use_envmap_mipmaps = UseEnvmapMipmaps();
	for( uint32 i = 0; i < m_envmap_descriptors.size(); i++)
	{
		CubeEnvMap *cem = CreateEnvMap( m_envmap_descriptors[i].m_position, i, use_envmap_mipmaps);
		m_cubemaps.push_back(cem);
	}

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	GeneratePVS();

#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
	//creating dummy stuff
	{
		try
		{
			m_dummyFbo = new GLB::FBO(64, 64, 0, GLB::RGB565_Linear, GLB::DEPTH_None, "dummy fbo");
		}
		catch (...)
		{
			delete m_dummyFbo;
			m_dummyFbo = 0;
			return KCL::KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED;
		}

		bool useES = g_extension->getGraphicsContext()->type() == GraphicsContext::GLES ;

        std::string srcVert;
        std::string srcFrag;
        if(useES || (m_scene_version < KCL::SV_30))
        {
		srcVert =
			"\
			#ifdef GL_ES\n\
			precision highp float;\n\
			#endif\n\
			attribute  vec2 myVertex;\n\
			attribute  vec2 myTexCoord;\n\
			varying  vec2 vTexCoord;\n\
			void main()\n\
			{\n\
			gl_Position = vec4(myVertex.x, myVertex.y, 0.0, 1.0);\n\
			vTexCoord = myTexCoord;\n\
			}\n\
			";

		srcFrag =
			"\
			#ifdef GL_ES\n\
			precision mediump float;\n\
			#endif\n\
			uniform sampler2D texChars;\n\
			varying  vec2 vTexCoord;\n\
			void main()\n\
			{\n\
			gl_FragColor = texture2D(texChars, vTexCoord);\n\
			}\n\
			";
        }
        else //SV_30 Manhattan uses core profile
        {
		srcVert =
			"\
			#version 400 core\n\
			in vec2 myVertex;\n\
			in vec2 myTexCoord;\n\
			out vec2 vTexCoord;\n\
			void main()\n\
			{\n\
				gl_Position = vec4(myVertex.x, myVertex.y, 0.0, 1.0);\n\
				vTexCoord = myTexCoord;\n\
			}\n\
			";

		srcFrag =
			"\
			#version 400 core\n\
			uniform sampler2D texChars;\n\
			in vec2 vTexCoord;\n\
			out vec4 frag_color;\n\
			void main()\n\
			{\n\
				frag_color = texture(texChars, vTexCoord);\n\
			}\n\
			";
        }

		uint32 dummy_vertex_shader   = GLB::initShader(GL_VERTEX_SHADER,   srcVert.c_str());
		uint32 dummy_fragment_shader = GLB::initShader(GL_FRAGMENT_SHADER, srcFrag.c_str());

		m_dummy_program = glCreateProgram();

		glAttachShader(m_dummy_program, dummy_vertex_shader  );
		glAttachShader(m_dummy_program, dummy_fragment_shader);

		glBindAttribLocation(m_dummy_program, 0, "myVertex");
		glBindAttribLocation(m_dummy_program, 1, "myTexCoord");

		glLinkProgram(m_dummy_program);

		glDeleteShader(dummy_vertex_shader  );
		glDeleteShader(dummy_fragment_shader);

		m_dummy_texture_unif_loc = glGetUniformLocation( m_dummy_program, "texChars");

		if ( 0 == glsl_log(m_dummy_program, GL_LINK_STATUS, "link"))
		{
			delete m_dummyFbo;
			m_dummyFbo = 0;

			glDeleteProgram(m_dummy_program);
			m_dummy_program = 0;

			return KCL::KCL_TESTERROR_SHADER_ERROR;
		}



		float verticesTmp[] =
		{
			-1.0f,  1.0f,  0.0f,  1.0f,
			-1.0f, -1.0f,  0.0f,  0.0f,
			1.0f,  1.0f,  1.0f,  1.0f,
			1.0f, -1.0f,  1.0f,  0.0f
		};
		uint8 indicesTmp[] = { 0, 1, 2, 3 };

		glGenBuffers(1, &m_dummy_vbo);
		glGenBuffers(1, &m_dummy_ebo);


		glBindBuffer(GL_ARRAY_BUFFER, m_dummy_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(verticesTmp), (const void*)verticesTmp, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);


		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_dummy_ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicesTmp), (const void*)indicesTmp, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
#endif


#ifdef OCCLUSION_QUERY_BASED_STAT
	m_glGLSamplesPassedQuery = new GLSamplesPassedQuery;
#endif

    IncrementProgressTo(0.9f);

	return result;
}


void GLB_Scene_ES2::RenderPlanar( KCL::PlanarMap* pm_)
{
	GLB::PlanarMap *pm = dynamic_cast<GLB::PlanarMap*>(pm_);
	pm->Bind();

#if defined STRIP_REDUNDANT_CLEARS
	DiscardColorAttachment();
	glClear( GL_DEPTH_BUFFER_BIT);
#else
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

	glViewport( 0, 0, pm->m_width, pm->m_height);

	glFrontFace( GL_CW);

	if(UseZPrePass())
		RenderPrepass(&pm->m_camera, pm->m_visible_meshes[0], 0, pm, 1, 0);
	else
		Render( &pm->m_camera, pm->m_visible_meshes[0], 0, pm, 1, 0);

	Render( &pm->m_camera, pm->m_visible_meshes[1], 0, pm, 1, 0);
	glFrontFace( GL_CCW);

	pm->Unbind();

#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
	//dummy render to flush parameter buffer
	{
		Shader::m_last_shader = 0;

		OpenGLStateManager::GlUseProgram(m_dummy_program);

		glBindFramebuffer(GL_FRAMEBUFFER, m_dummyFbo->getName());		

		glViewport(0, 0, m_dummyFbo->getWidth(), m_dummyFbo->getHeight());
#if defined STRIP_REDUNDANT_CLEARS
		DiscardColorAttachment();
		glClear( GL_DEPTH_BUFFER_BIT);
#else
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

		OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pm->GetTextureId());

		glUniform1i( m_dummy_texture_unif_loc, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_dummy_vbo);	
		OpenGLStateManager::GlEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_dummy_ebo);		
		OpenGLStateManager::GlEnableVertexAttribArray(1);

		GLB::OpenGLStateManager::Commit();

		glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, 0, 2*sizeof(float), (const void*)(2*sizeof(float)));

		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		OpenGLStateManager::GlDisableVertexAttribArray(0);
		OpenGLStateManager::GlDisableVertexAttribArray(1);

		OpenGLStateManager::GlUseProgram(0);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName() );
	}
#endif
}


#ifdef DUMMY_RENDER_FOR_PLANAR_FLUSH
void Scene::RenderPlanar( PlanarMap* pm, PlanarMap* pm2)
{
	pm->Bind();

	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport( 0, 0, pm->m_width, pm->m_height);

	glFrontFace( GL_CW);
	Render( &pm->m_camera, 0, pm->m_visible_meshes[0][0], 0, pm, 1);
	Render( &pm->m_camera, 1, pm->m_visible_meshes[0][1], 0, pm, 1);
	Render( &pm->m_camera, 0, pm->m_visible_meshes[1][0], 0, pm, 1);
	Render( &pm->m_camera, 1, pm->m_visible_meshes[1][1], 0, pm, 1);
	//glFrontFace( GL_CCW);

	pm->Unbind();


	//dummy render to flush parameter buffer
	{
		Material::m_last_shader = 0;

		OpenGLStateManager::GlUseProgram(m_dummy_program);

		//glBindFramebuffer(GL_FRAMEBUFFER, m_dummyFbo->getName());

		//glViewport(0, 0, m_dummyFbo->getWidth(), m_dummyFbo->getHeight());
		//glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
		//glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if(pm2)
		{
			pm2->Bind();
		}
		else
		{
			glBindFramebuffer(GL_FRAMEBUFFER, KCL::g_os->GetGlobalFBO() );
		}

		OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, pm->GetTextureId());

		glUniform1i( m_dummy_texture_unif_loc, 0);

		glBindBuffer(GL_ARRAY_BUFFER, m_dummy_vbo);	
		OpenGLStateManager::GlEnableVertexAttribArray(0);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_dummy_ebo);		
		OpenGLStateManager::GlEnableVertexAttribArray(1);
		GLB::OpenGLStateManager::Commit();

		glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
		glVertexAttribPointer(1, 2, GL_FLOAT, 0, 2*sizeof(float), (const void*)(2*sizeof(float)));

		glDrawElements(GL_TRIANGLE_STRIP, 4, GL_UNSIGNED_BYTE, 0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		OpenGLStateManager::GlDisableVertexAttribArray(0);
		OpenGLStateManager::GlDisableVertexAttribArray(1);

		OpenGLStateManager::GlUseProgram(0);

		if(pm2)
		{
			//glBindFramebuffer(GL_FRAMEBUFFER, KCL::g_os->GetGlobalFBO() );
		}		
	}
	glFrontFace( GL_CCW);
}
#endif


void GLB_Scene_ES2::RenderShadow( KRL_ShadowMap* sm)
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

	Render(&sm->m_camera, visible_meshes[0], m_shadowCasterMaterial, 0, 1, 0);
	Render(&sm->m_camera, visible_meshes[1], m_shadowCasterMaterial, 0, 1, 0);

#if 0
	m_shadowCasterMaterial->preInit( 0);

	OpenGLStateManager::GlEnableVertexAttribArray( m_shadowCasterMaterial->m_shaders[0]->m_attrib_locations[attribs::in_position]);

	glUniformMatrix4fv( m_shadowCasterMaterial->m_shaders[0]->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, sm->m_camera.GetViewProjection().v);

	glLineWidth( 100);


	DrawAABB( dab, m_shadowCasterMaterial->m_shaders[0]->m_attrib_locations[attribs::in_position]);
	glLineWidth( 4);
	OpenGLStateManager::GlDisableVertexAttribArray( m_shadowCasterMaterial->m_shaders[0]->m_attrib_locations[attribs::in_position]);

	m_shadowCasterMaterial->postInit();
#endif

	glViewport( 0, 0, m_viewport_width, m_viewport_height);

	m_shadowStaticReceiverMeshes = sm->m_receiver_meshes[0];

	sm->Unbind();
}

void Render_Frustum_Of_Camera_From_Eye(const Camera2& camera, const Camera2& eye, Shader *s);


void GLB_Scene_ES2::Render()
{
#ifdef LOG_SHADERS

#ifdef PER_FRAME_ALU_INFO
	Shader::Push_New_FrameAluInfo();
#endif

#endif

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
					(size_t)mio->m_mesh->m_mesh->m_ebo[j].m_offset
					);
			}
		}

		mioi++;
	}



	m_num_draw_calls = 0;
	m_num_triangles = 0;
	m_num_vertices = 0;

#ifdef TEXTURE_COUNTING
	m_textureCounter.clear();
	Material::TextureCounter(m_textureCounter);
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
	m_num_samples_passed = 0;
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
    m_pixel_coverage_sampleCount = 0;
    m_pixel_coverage_primCount = 0;
#endif
#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT
	m_num_instruction = 0;
#endif
#endif


	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		if( m_global_shadowmaps[i])
		{
			RenderShadow( m_global_shadowmaps[i]);
		}
	}
	
	for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
#ifndef DUMMY_RENDER_FOR_PLANAR_FLUSH
		RenderPlanar( m_visible_planar_maps[i]);
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

	if( m_mblur_enabled)
	{
		glBindFramebuffer( GL_FRAMEBUFFER, m_main_fbo->getName());
	}

	glViewport( 0, 0, m_viewport_width, m_viewport_height);

#if defined STRIP_REDUNDANT_CLEARS
	DiscardColorAttachment();
	glClear( GL_DEPTH_BUFFER_BIT);
#else
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif

	if(UseZPrePass())
	{
        RenderPrepass(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0);
    }
	else
	{
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = true;
#endif
        Render(m_active_camera, m_visible_meshes[0], 0, 0, 0, 0);
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = false;
#endif
    }

	Render(m_active_camera, m_visible_meshes[1], 0, 0, 0, 0);

	for( uint32 i=0; i<m_visible_planar_maps.size(); i++)
	{
		dynamic_cast<GLB::Material*>(m_planarReflectionMaterial)->m_planar_map = dynamic_cast<GLB::PlanarMap*>(m_visible_planar_maps[i]);
		m_planarReflectionMaterial->m_transparency = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_reflect_intensity;
		m_planarReflectionMaterial->m_textures[2] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[2];
		m_planarReflectionMaterial->m_textures[3] = m_visible_planar_maps[i]->m_receiver_meshes[0]->m_material->m_textures[3];

		GLB_Scene_ES2_::Render(m_active_camera, m_visible_planar_maps[i]->m_receiver_meshes, m_planarReflectionMaterial, 0, 0, 0, 0);

		m_planarReflectionMaterial->m_textures[2] = 0;
		m_planarReflectionMaterial->m_textures[3] = 0;
	}

	if( m_num_shadow_maps )
	{
		GLB_Scene_ES2_::Render(m_active_camera, m_shadowStaticReceiverMeshes, m_shadowStaticReceiverMaterial, 0, 0, 0, 0);
	}

	if( m_mblur_enabled)
	{
		DiscardDepthAttachment();
		glBindFramebuffer( GL_FRAMEBUFFER, m_mblur_fbo->getName());
		
		glClearColor( 0.5, 0.5, 0.0, 1);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport( 0, 0, m_mblur_fbo->getWidth(), m_mblur_fbo->getHeight());

		Render(m_active_camera, m_motion_blurred_meshes, m_mblurMaterial, 0, 0, 0);

		DiscardDepthAttachment();
		glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName() );

#if defined STRIP_REDUNDANT_CLEARS
		DiscardColorAttachment();
#else
		glClearColor( 0.0f, 0.0f, 0.0f, 1.0f);
		glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
		glViewport( 0, 0, m_viewport_width, m_viewport_height);

		OpenGLStateManager::GlUseProgram( m_blur_shader->m_p);

		if (m_blur_shader->m_uniform_locations[GLB::uniforms::inv_resolution]>-1)
		{
			glUniform2f(m_blur_shader->m_uniform_locations[GLB::uniforms::inv_resolution], 1.0f / m_viewport_width, 1.0f / m_viewport_height);
		}
		
		OpenGLStateManager::GlActiveTexture(GL_TEXTURE1);
		glBindTexture( GL_TEXTURE_2D, m_mblur_fbo->getTextureName() );
		glUniform1i(m_blur_shader->m_uniform_locations[GLB::uniforms::texture_unit1], 1);

		OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
		glBindTexture( GL_TEXTURE_2D, m_main_fbo->getTextureName() );
		glUniform1i(m_blur_shader->m_uniform_locations[GLB::uniforms::texture_unit0], 0);

#ifdef TEXTURE_COUNTING
		m_textureCounter.insert( m_mblur_fbo->getTextureName() );
		m_textureCounter.insert( m_main_fbo->getTextureName() );
#endif


		glBindBuffer( GL_ARRAY_BUFFER, m_fullscreen_quad_vbo);
		OpenGLStateManager::GlEnableVertexAttribArray(m_blur_shader->m_attrib_locations[attribs::in_position]);
		OpenGLStateManager::Commit();
		glVertexAttribPointer(m_blur_shader->m_attrib_locations[attribs::in_position], 2, GL_FLOAT, GL_FALSE, sizeof(Vector4D), 0);


#ifdef OCCLUSION_QUERY_BASED_STAT
		m_glGLSamplesPassedQuery->Begin();
#endif
		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4);

		OpenGLStateManager::GlDisableVertexAttribArray(m_blur_shader->m_attrib_locations[attribs::in_position]);
		glBindBuffer( GL_ARRAY_BUFFER, 0);

		m_num_draw_calls++;
		m_num_triangles += 2;
	    m_num_vertices += 4;

#ifdef OCCLUSION_QUERY_BASED_STAT
		m_glGLSamplesPassedQuery->End();
		m_num_samples_passed += m_glGLSamplesPassedQuery->Result();

#ifdef CALCULATE_SHADER_INSTRUCTION_COUNT		
		m_num_instruction += m_blur_shader->m_instruction_count_v * 4 + m_blur_shader->m_instruction_count_f * m_glGLSamplesPassedQuery->Result();
#endif
#endif
	}

	Shader::m_last_shader = 0;

#ifdef TEXTURE_COUNTING
	Material::NullTextureCounter();
	m_num_used_texture = m_textureCounter.size();
#endif

#if 0
	OpenGLStateManager::GlEnable(GL_DEPTH_TEST);

	OpenGLStateManager::GlDepthFunc( GL_LEQUAL);
	OpenGLStateManager::GlUseProgram( m_shader->m_p);
	glUniformMatrix4fv( m_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);

	glLineWidth( 4);


	OpenGLStateManager::GlEnableVertexAttribArray( m_shader->m_attrib_locations[attribs::in_position]);

	XRoom *my_room = SearchMyRoom( m_active_camera->GetEye());

	if( my_room )
	{
		Camera2* camera = m_active_camera;
		std::vector<XRoom*> visible_rooms;
		std::vector<KCL::PlanarMap*> visible_planar_maps;
		std::vector<Mesh*> visible_meshes[2];
		Vector4D p[] = 
		{
			-camera->GetCullPlane(0),
			-camera->GetCullPlane(1),
			-camera->GetCullPlane(2),
			-camera->GetCullPlane(3),
			-camera->GetCullPlane(4),
			-camera->GetCullPlane(5),
		};

		p[5].w = - Vector3D::dot( Vector3D( p[5].x, p[5].y, p[5].z), camera->GetEye());

		XRoom::FrustumCull( visible_rooms, visible_planar_maps, visible_meshes, my_room, camera, p, 4, my_room, m_pvs);

		for(size_t i=0; i<visible_rooms.size(); ++i)
		{
			XRoom *r = visible_rooms[i];

			if( i == 0)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, 0, 0);
			}
			else if( i == 1)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0, 1, 0);
			}
			else if( i == 2)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0, 0, 1);
			}
			else if( i == 3)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0, 1, 1);
			}
			else if( i == 4)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, 0, 1);
			}
			else if( i == 5)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, 1, 0);
			}
			else if( i == 6)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0.5, 0, 0);
			}
			else if( i == 7)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0, 0.5, 0);
			}
			else if( i == 8)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0, 0, 0.5);
			}
			else if( i == 9)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0, 0.5, 0.5);
			}
			else if( i == 10)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0.5, 0, 0.5);
			}
			else if( i == 11)
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 0.5, 0.5, 0);
			}
			else
			{
				glUniform3f( m_shader->m_uniform_locations[GLB::uniforms::background_color], 1, 1, 1);
			}

			for(size_t j=0; j<r->m_meshes.size(); ++j)
			{
				Mesh* sm = r->m_meshes[j];

				if( m_active_camera->IsVisible( &sm->m_aabb))
				{
					Matrix4x4 mvp = sm->m_world_pom * m_active_camera->GetViewProjection();
					glUniformMatrix4fv( m_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, mvp.v);

					VboPool::Instance()->BindBuffer(sm->m_mesh->m_vbo);
					IndexBufferPool::Instance()->BindBuffer(sm->m_mesh->m_ebo[0].m_buffer);

					GLB::OpenGLStateManager::Commit();
					glVertexAttribPointer( 
						m_shader->m_attrib_locations[attribs::in_position], 
						sm->m_mesh->m_vertex_attribs[0].m_size,
						sm->m_mesh->m_vertex_attribs[0].m_type,
						sm->m_mesh->m_vertex_attribs[0].m_normalized, 
						sm->m_mesh->m_vertex_attribs[0].m_stride, 
						sm->m_mesh->m_vertex_attribs[0].m_data
						);
					glDrawElements(GL_TRIANGLES, sm->m_mesh->m_vertex_indices[0].size(), GL_UNSIGNED_SHORT, sm->m_mesh->m_ebo[0].m_offset);
				}
			}
		}
	}
	OpenGLStateManager::GlDisableVertexAttribArray( m_shader->m_attrib_locations[attribs::in_position]);
#endif
#if 0
	OpenGLStateManager::GlEnable(GL_DEPTH_TEST);

	glLineWidth( 4);

	OpenGLStateManager::GlEnable( GL_BLEND);
	OpenGLStateManager::GlEnable( GL_CULL_FACE);
	OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	OpenGLStateManager::GlUseProgram( m_shader->m_p);
	glUniformMatrix4fv( m_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, m_active_camera->GetViewProjection().v);

	for( uint32 i=0; i<m_actors.size(); i++)
	{
		renderSkeleton( m_actors[i]->m_root);
	}

	RenderVisibleAABBs( true, false, false);

	OpenGLStateManager::GlDisable( GL_CULL_FACE);
	OpenGLStateManager::GlBlendFunc( 1, 1);

	RenderPortals();

	glLineWidth( 1);
	OpenGLStateManager::GlDisable( GL_BLEND);
	OpenGLStateManager::GlDisable(GL_DEPTH_TEST);

#endif
#if	0
	//OpenGLStateManager::GlUseProgram( 0);

	//OpenGLStateManager::GlEnable( GL_TEXTURE_2D);

	glUseProgram( 0);

	glEnable( GL_TEXTURE_2D);
	glBindTexture( GL_TEXTURE_2D, m_global_shadowmaps[0]->GetTextureId());
	glBegin(GL_QUADS);

	glTexCoord2f( 0, 0);
	glVertex2f( -1,-1);
	glTexCoord2f( 1, 0);
	glVertex2f(  0,-1);
	glTexCoord2f( 1, 1);
	glVertex2f(  0, 0);
	glTexCoord2f( 0, 1);
	glVertex2f( -1, 0);

	glEnd();
#endif

#if 0
	OpenGLStateManager::GlEnable(GL_DEPTH_TEST);

	glLineWidth( 4);

	OpenGLStateManager::GlEnable( GL_BLEND);
	OpenGLStateManager::GlEnable( GL_CULL_FACE);
	OpenGLStateManager::GlBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	OpenGLStateManager::GlUseProgram( m_shader->m_p);

	Render_Frustum_Of_Camera_From_Eye(*m_animated_camera, *m_active_camera, m_shader);
	Render_Frustum_Of_Camera_From_Eye( m_global_shadowmap->m_camera , *m_active_camera, m_shader);


	glLineWidth( 1);
	OpenGLStateManager::GlDisable( GL_CULL_FACE);
	OpenGLStateManager::GlDisable( GL_BLEND);

	OpenGLStateManager::GlDisable(GL_DEPTH_TEST);

	//return;
#endif

	//printf(" %d, %d\n", m_num_draw_calls, m_num_triangles);

	//log_file = fopen( "log2.txt", "at");
	//fprintf( log_file, "%d, %d\n", m_animation_time, m_num_draw_calls);
	//fclose( log_file);


	/*
	static KCL::uint32 max_draw_calls = m_num_draw_calls;
	static KCL::uint32 max_draw_calls_at_animation_time = m_animation_time;
	static KCL::uint32 max_triangle_count = m_num_triangles;
	static KCL::uint32 max_triangle_count_at_animation_time = m_animation_time;

	if( max_draw_calls < m_num_draw_calls )
	{
	max_draw_calls = m_num_draw_calls;
	max_draw_calls_at_animation_time = m_animation_time;
	}

	if( max_triangle_count < m_num_triangles )
	{
	max_triangle_count = m_num_triangles;
	max_triangle_count_at_animation_time = m_animation_time;
	}

	INFO(" Max draw calls: %d at animation time: %d\n", max_draw_calls, max_draw_calls_at_animation_time);
	INFO(" Max triangle count: %d at animation time: %d\n", max_triangle_count, max_triangle_count_at_animation_time);
	//*/
}


void GLB_Scene_ES2::Render( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light)
{
	Render( camera, visible_meshes, _override_material, pm, lod, light, 0);
}

void GLB_Scene_ES2::RenderPrepass( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light)
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

#ifdef OCCLUSION_QUERY_BASED_STAT
	//do_not_skip_samples_passed = false;
#endif

	/* render opaque_visible_meshes to depth only */
	glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
	GLB_Scene_ES2_::Render( camera, default_visible_meshes, _override_material, pm, lod, light, -1);
	GLB_Scene_ES2_::Render( camera, foliage_visible_meshes, _override_material, pm, lod, light, -1);

	/* dispatch shadepass */
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = true;
#endif
	GLB_Scene_ES2_::Render( camera, visible_meshes, _override_material, pm, lod, light, +1);
#ifdef MEASURE_PIXEL_PER_PRIMITIVE_COVERAGE
	m_measurePixelCoverage = false;
#endif

#ifdef OCCLUSION_QUERY_BASED_STAT
	//do_not_skip_samples_passed = true;
#endif
}


const Vector3D refs[6] =
{
	Vector3D(1, 0, 0),
	Vector3D(-1, 0, 0),
	Vector3D(0, 1, 0),
	Vector3D(0, -1, 0),
	Vector3D(0, 0, 1),
	Vector3D(0, 0, -1)
};
const Vector3D ups[6] =
{
	Vector3D(0, -1, 0),
	Vector3D(0, -1, 0),
	Vector3D(0, 0, 1),
	Vector3D(0, 0, -1),
	Vector3D(0, -1, 0),
	Vector3D(0, -1, 0)
};


CubeEnvMap *GLB_Scene_ES2::CreateEnvMap( const GLB::Vector3D &pos, uint32 idx, bool use_mipmaps)
{
	std::string envmap_path(EnvmapsDirectory());
	CubeEnvMap *cubemap = CubeEnvMap::Load( idx, envmap_path.c_str(), use_mipmaps);

	if( !cubemap)
	{
		INFO("Generate environment  maps...");
		std::vector<Mesh*> visible_meshes[2];
		std::vector<KCL::PlanarMap*> visible_planar_maps;
		std::vector<KCL::Mesh*> meshes_to_blur;
		std::vector< std::vector<KCL::Mesh*> > visible_instances;

		FboEnvMap* m_fboEnvMap = new FboEnvMap( m_fboEnvMap_size);
		Camera2 camera;

		cubemap = m_fboEnvMap->CreateCubeEnvMapRGBA();

		camera.Perspective(90.0f, 1, 1, 0.01f, 1000.0f);

		glViewport( 0, 0, m_fboEnvMap->GetSize(), m_fboEnvMap->GetSize());

		m_fboEnvMap->Bind();
		for(size_t i = 0; i < 6; ++i)
		{
			m_fboEnvMap->AttachCubemap(cubemap, i);
			camera.LookAt(pos, pos+refs[i], ups[i]);
			camera.Update();

			visible_meshes[0].clear();
			visible_meshes[1].clear();

			Vector2D nearfar;

			FrustumCull(&camera, visible_planar_maps, visible_meshes, visible_instances, meshes_to_blur, m_pvs, nearfar, 0, true, false);

			glClearColor( m_background_color.x, m_background_color.y, m_background_color.z, 1.0f);
			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


			if(UseZPrePass())
				RenderPrepass( &camera, visible_meshes[0], 0, 0, 0, 0);
			else
				Render( &camera, visible_meshes[0], 0, 0, 0, 0);

			Render( &camera, visible_meshes[1], 0, 0, 0, 0);

			cubemap->Save( idx, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_fboEnvMap->GetSize(), envmap_path.c_str());
		}

		m_fboEnvMap->Unbind();
		glViewport( 0, 0, m_viewport_width, m_viewport_height);	

		delete m_fboEnvMap;
	}

	cubemap->SetPosition( pos);

	return cubemap;
}


void GLB_Scene_ES2::GeneratePVS()
{
#if 0
	return;
#ifndef __glew_h__
	m_bsp_tree->ResetPVS();

	for( uint32 i=0; i<m_bsp_tree->m_num_nodes0; i++)
	{
		for( uint32 j=0; j<m_bsp_tree->m_num_nodes0; j++)
		{
			m_bsp_tree->m_pvs[i][j] = true;
		}
	}
#else
	const int w = 480;
	uint32 t = 0;
	uint32 cicc;
	Camera2 camera;

	if( w > m_viewport_width || w > m_viewport_height)
	{
		printf("!warning: PVS generating will be corrupted.\n");
	}

	OpenGLStateManager::GlUseProgram( m_shader->m_p);
	glGenQueries( 1, &cicc);

	camera.Perspective(90.0f, w, w, 0.01f, 1000.0f);

	glViewport( 0, 0, w, w);
	OpenGLStateManager::GlEnable( GL_DEPTH_TEST);

	OpenGLStateManager::GlDepthFunc( GL_LESS);

	OpenGLStateManager::GlEnableVertexAttribArray( m_shader->m_attrib_locations[attribs::in_position]);

	while( 1)
	{
		Matrix4x4 m = AnimateCamera( t);

		Vector3D pos( m.v[12], m.v[13], m.v[14]);

		for(size_t i=0; i<6; ++i)
		{
			std::vector<XRoom*> visible_rooms;

			camera.LookAt( pos, pos+refs[i], ups[i]);

			camera.Update();

			glUniformMatrix4fv( m_shader->m_uniform_locations[GLB::uniforms::mvp], 1, GL_FALSE, camera.GetViewProjection().v);

			XRoom *my_room = SearchMyRoom( &camera);
			if( !my_room)
			{
				printf("!warning: camera outside of scene.\n");
				continue;
			}

			XRoom::FrustumCull( visible_rooms, &camera, my_room, 0, 0);

			glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for( uint32 j=0; j<visible_rooms.size(); j++)
			{
				XRoom* n = visible_rooms[j];
				uint32 ready = 0;
				uint32 num_samples = 0;
				bool query = false;

				if( !m_pvs[my_room->m_pvs_index][n->m_pvs_index])
				{
					query = true;
				}

				if( query)
				{
					glBeginQuery( GL_SAMPLES_PASSED, cicc);
				}

				for( uint32 k=0; k<n->m_meshes.size(); k++)
				{
					GLB::OpenGLStateManager::Commit();

					glBindBuffer( GL_ARRAY_BUFFER, n->m_meshes[k]->m_mesh->m_vbo);

					glVertexAttribPointer( m_shader->m_attrib_locations[attribs::in_position], 3, n->m_meshes[k]->m_mesh->m_vertex_attribs[0].m_type, 0, n->m_meshes[k]->m_mesh->m_vertex_attribs[0].m_stride, n->m_meshes[k]->m_mesh->m_vertex_attribs[0].m_data);

					glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, n->m_meshes[k]->m_mesh->m_ebo);

					glDrawElements( GL_TRIANGLES, n->m_meshes[k]->m_mesh->m_vertex_indices.size(), GL_UNSIGNED_SHORT, 0);
				}

				glFinish();

				if( query)
				{
					glEndQuery( GL_SAMPLES_PASSED);

					while( !ready)
					{
						glGetQueryObjectuiv( cicc, GL_QUERY_RESULT_AVAILABLE, &ready);
					}

					glGetQueryObjectuiv( cicc, GL_QUERY_RESULT, &num_samples);

					float t = 100.0f * (float)num_samples / (float)(w * w);
					if( t > 0.1f)
					{
						m_pvs[my_room->m_pvs_index][n->m_pvs_index] = true;
					}
				}
			}
		}

		t += 100;

		if( t >= 113000)
		{
			break;
		}
	}


	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer( GL_ARRAY_BUFFER, 0);
	OpenGLStateManager::GlDisableVertexAttribArray( m_shader->m_attrib_locations[attribs::in_position]);

	OpenGLStateManager::GlDisable( GL_DEPTH_TEST);

	glViewport( 0, 0, m_viewport_width, m_viewport_height);

	//m_bsp_tree->SavePVS( m_path.c_str());
#endif
#endif
}

#endif

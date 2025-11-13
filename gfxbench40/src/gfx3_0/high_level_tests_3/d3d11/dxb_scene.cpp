/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_scene.h"

#include "dxb_planarmap.h"
#include "d3d11/dxb_image.h"
#include "dxb_material.h"
#include "dxb_mesh.h"
#include "d3d11/vbopool.h"
#include "d3d11/shader.h"
#include <kcl_particlesystem2.h>

#include "krl_mesh.h"
#include "cubemap.h"
#include "shadowmap.h"

using namespace DirectX;

void DXB_Scene::InitFactories()
{
	m_materialFactory = new DXB::MaterialFactory(m_scene_version);
	m_factory.RegisterFactory(m_materialFactory, KCL::MATERIAL);
}


DXB_Scene::DXB_Scene() : KRL_Scene(), m_main_fbo( 0), m_mblur_fbo( 0), m_particles_vbo(0)
{
	m_factory.RegisterFactory(&m_animated_factory, KCL::EMITTER1);
	m_factory.RegisterFactory(&m_emitter_factory, KCL::EMITTER2);
	m_factory.RegisterFactory(&m_light_factory, KCL::LIGHT);
}

DXB_Scene::~DXB_Scene()
{
	Release_GLResources();
}

void DXB_Scene::Release_GLResources()
{
	delete m_main_fbo;
	delete m_mblur_fbo;
	m_main_fbo=0;
	m_mblur_fbo=0;
	m_linearSampler = nullptr;
	m_linearSamplerClamp = nullptr;

    m_pointSamplerClamp = nullptr;
    m_shadowCmpSampler = nullptr;

	m_fullscreen_inputLayout = nullptr;
}

KCL::KCL_Status DXB_Scene::reloadShaders()
{
	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;
	char max_joint_num_per_mesh_str[64];

	sprintf( max_joint_num_per_mesh_str, " %d ", m_max_joint_num_per_mesh);

    std::set< std::string> def;
	m_shader = Shader::CreateShader( "debug.vs", "debug.fs", &def, result);

	if (m_shader == NULL)
	{
		return KCL::KCL_TESTERROR_SHADER_ERROR;
	}

	Shader::m_default_shader = m_shader;

	
	def.insert("DEP_TEXTURING");	/* force mediump */

	m_blur_shader = Shader::CreateShader( "pp.vs", "mblur_final.fs", &def, result);

	for(size_t i=0; i<m_materials.size(); ++i)
	{
		std::string s;

		if( m_shadow_method_str == "depth map(depth)")
		{
			s += "shadow_depth_map_depth ";
		}
		else if( m_shadow_method_str == "depth map(color)")
		{
			s += "shadow_depth_map_color ";
		}
		else if( m_shadow_method_str == "simple projective")
		{
			s += "shadow_simple_projective ";
		}
		if( m_soft_shadow_enabled)
		{
			s += "soft_shadow ";
		}
		s += GetVersionStr() + " ";

		result = dynamic_cast<KRL::Material*>(m_materials[i])->InitShaders( s.c_str(), max_joint_num_per_mesh_str);
		if(result != KCL::KCL_TESTERROR_NOERROR)
		{
			return result;
		}
	}

	if( m_scene_version == KCL::SV_30)
	{
		std::set< std::string> defines;


        defines.insert("SV_30");
		defines.insert( "SPECIAL_DIFFUSE_CLAMP");
		defines.insert( "NEED_HIGHP");
		m_lighting_shaders[0] = Shader::CreateShader( "lighting.vs", "lighting.fs", &defines, result);
		defines.insert( "SHADOW_MAP");
		m_lighting_shaders[10] = Shader::CreateShader( "lighting.vs", "lighting.fs", &defines, result);

		defines.clear();
        defines.insert("SV_30");
		defines.insert( "POINT_LIGHT");
		defines.insert( "NEED_HIGHP");
		m_lighting_shaders[1] = Shader::CreateShader( "lighting.vs", "lighting.fs", &defines, result);
		defines.insert( "SHADOW_MAP");
		m_lighting_shaders[11] = Shader::CreateShader( "lighting.vs", "lighting.fs", &defines, result);

		defines.clear();
        defines.insert("SV_30");
		defines.insert( "SPOT_LIGHT");
		defines.insert( "NEED_HIGHP");
		m_lighting_shaders[2] = Shader::CreateShader( "lighting.vs", "lighting.fs", &defines, result);
		defines.insert( "SHADOW_MAP");
		m_lighting_shaders[12] = Shader::CreateShader( "lighting.vs", "lighting.fs", &defines, result);

		defines.clear();
        defines.insert("SV_30");
		m_lighting_shaders[15] = Shader::CreateShader( "lighting.vs", "shadow_decal.fs", &defines, result);

		defines.clear();
        defines.insert("SV_30");
		defines.insert( "NEED_HIGHP");

		m_lighting_shader = Shader::CreateShader( "lighting.vs", "lighting2.fs", &defines, result);

		defines.clear();
        defines.insert("SV_30");
		m_pp_shaders[2] = Shader::CreateShader( "pp.vs", "sub.fs", &defines, result);
		m_pp_shaders[1] = Shader::CreateShader( "color_blur.vs", "color_blur.fs", &defines, result);
		m_pp_shaders[0] = Shader::CreateShader( "pp.vs", "pp.fs", &defines, result);

		m_reflection_emission_shader = Shader::CreateShader( "pp.vs", "re.fs", &defines, result);
		
		m_hud_shader = Shader::CreateShader( "hud.vs", "hud.fs", &defines, result);

		m_occlusion_query_shader = Shader::CreateShader( "oc.vs", "oc.fs", &defines, result);

		m_fog_shader = Shader::CreateShader( "fog.vs", "fog.fs", &defines, result);

		m_camera_fog_shader = Shader::CreateShader( "fog.vs", "fog_camera.fs", &defines, result);

		m_particleAdvect_shader = Shader::CreateShader( "particleAdvect.vs", "particleAdvect.fs", &defines, result, Shader::ShaderTypes::TransformFeedback);
	}

	return result;
}



KCL::KCL_Status DXB_Scene::Process_GL( GLB::FBO_COLORMODE color_mode, GLB::FBO_DEPTHMODE depth_mode, int samples)
{
	Shader::InitShaders( (m_scene_version==KCL::SceneVersion::SV_30)?"es3":"es2", true);
	//--------------------------------------------------------------
	
    D3D11_SAMPLER_DESC samplerDesc;
    ZeroMemory(&samplerDesc, sizeof(samplerDesc));
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MipLODBias = 0.0f;
    samplerDesc.MaxAnisotropy = 8;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.BorderColor[0] = 0.0f;
    samplerDesc.BorderColor[1] = 0.0f;
    samplerDesc.BorderColor[2] = 0.0f;
    samplerDesc.BorderColor[3] = 0.0f;
    samplerDesc.MinLOD = 0.0f;
    samplerDesc.MaxLOD = FLT_MAX;

    DX_THROW_IF_FAILED(
        DX::getDevice()->CreateSamplerState(
            &samplerDesc,
            &m_linearSampler
            )
        );
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    DX_THROW_IF_FAILED(
        DX::getDevice()->CreateSamplerState(
            &samplerDesc,
            &m_linearSamplerClamp
            )
        );

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    DX_THROW_IF_FAILED(
        DX::getDevice()->CreateSamplerState(
            &samplerDesc,
            &m_pointSamplerClamp
            )
        );

    samplerDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    DX_THROW_IF_FAILED(
        DX::getDevice()->CreateSamplerState(
            &samplerDesc,
            &m_shadowCmpSampler
            )
        );

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

	for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
	{
		delete m_global_shadowmaps[i];
		m_global_shadowmaps[i] = ShadowMap::Create(m_fboShadowMap_size, m_shadow_method_str, m_fullscreen_quad_vbo, true);
	}

	DX::getStateManager()->SetRasterFrontClockwise(false);

	//-------------------------------

	KCL::KCL_Status result = KCL::KCL_TESTERROR_NOERROR;

	
	if( m_do_instancing)
	{
		std::map<KCL::Mesh3*, KCL::MeshInstanceOwner*> mios_map;

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
					std::map<KCL::Mesh3*,KCL::MeshInstanceOwner*>::iterator f = mios_map.find( m);

					if( f == mios_map.end())
					{
						mio = new KCL::MeshInstanceOwner;
						mio->m_mesh= new KCL::Mesh;
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
		
			
			for(unsigned int k = 0 ;k < instances.size();k++)
			{
				KCL::Texture* tex = instances[k]->m_material->m_textures[1];
				instances[k]->m_material->m_textures[1] = 0;
				for (size_t i=0;i<m_textures.size();i++)
				{
					if (tex==m_textures[i])
						m_textures[i] = 0;
				}
				delete tex;
			}

			KCL::Texture *texture = TextureFactory().CreateAndSetup(KCL::Texture_2D, newName.c_str(),KCL::TC_Commit);

			(*mio)->m_mesh->m_material->m_textures[1] = texture;
			m_textures.push_back(texture);

			(*mio)->Instance();
			(*mio)->m_mesh->DeleteUnusedAttribs();
			mio++;
		}
	}
	else
	{
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
			KCL::AnimatedEmitter *emitter = (KCL::AnimatedEmitter*)actor->m_emitters[j];

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
		dynamic_cast<DXB::Mesh3*>(m_meshes[i])->InitVertexAttribs(); 	
	}

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

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

    //NOTE: Manhattan uses instancing, which does not work currently with
    //      multiple cubemaps - this reduces the cubes to always select the
    //      same 2 - which will make sure image data is consistent between
    //      consequtive runs
    if(m_scene_version == KCL::SV_30)
    {
		std::vector<KCL::CubeEnvMapDescriptor> envmap_fixup;
		envmap_fixup.push_back(m_envmap_descriptors[0]);
		envmap_fixup.push_back(m_envmap_descriptors[1]);

		m_envmap_descriptors = envmap_fixup;
	}

	for( uint32 i = 0; i < m_envmap_descriptors.size(); i++)
	{
		CubeEnvMap *cem = CreateEnvMap( m_envmap_descriptors[i].m_position, i);
		if (cem == NULL)
		{
			return KCL::KCL_TESTERROR_FILE_NOT_FOUND;
		}

		m_cubemaps.push_back(cem);
	}

	switch (KCL::KCL_Status status = KCL::g_os->LoadingCallback(0))
	{
	case KCL::KCL_TESTERROR_NOERROR:
		break;
	default:
		return status;
	}

	if (m_mblur_enabled) {
		delete m_main_fbo;
		delete m_mblur_fbo;
		FBO* main_fbo = new FBO();
		FBO* mblur_fbo = new FBO();
		m_main_fbo = main_fbo;
		m_mblur_fbo = mblur_fbo;
		
		bool b1 = main_fbo->init( m_viewport_width, m_viewport_height, color_mode, depth_mode, samples ? 4 : 1);
		bool b0 = mblur_fbo->init( m_viewport_width, m_viewport_height, RGB888_Linear, GLB::DEPTH_16_RB);
		
		if( !b0 || !b1)
		{
			return KCL::KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED;
		}
	}

	{
		float screenAlignedQuad[] =
		{
			-1, -1, 0,
			1, -1, 0,
			1,  1, 0,
			-1,  1, 0,
		};

		const uint16 idx[] = {0, 1, 2, 2, 3, 0}; //6

		uint32 offset;
		VboPool::Instance()->AddData(4*3*sizeof(float), (const void*)screenAlignedQuad, m_fullscreen_vbo, offset, 3*sizeof(float));
		IndexBufferPool::Instance()->AddData(6*sizeof(uint16), (const void*)idx, m_fullscreen_ebo, offset);

		std::vector<D3D11_INPUT_ELEMENT_DESC> vertexDesc;
		
		const D3D11_INPUT_ELEMENT_DESC vertexLayout[] =
		{
			{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0 , D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		if( m_shader)
		{
			DX_THROW_IF_FAILED(
				DX::getDevice()->CreateInputLayout(
				vertexLayout,
                _countof(vertexLayout),
				&m_shader->m_vs.m_bytes[0],
				m_shader->m_vs.m_bytes.size(),
				&m_fullscreen_inputLayout
				)
				);
		}
	}

	m_constantBuffer = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBuffer>();
	m_constantBuffer->commit();

	m_constantBufferParticle = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBufferParticle>();
	m_constantBufferParticle->commit();

	m_constantBufferMBlur = KCL::ConstantBuffer::factory->CreateBuffer<ConstantBufferMBlur>();
	m_constantBufferMBlur->commit();


	GeneratePVS();

	return result;
}


void DXB_Scene::RenderPlanar( KCL::PlanarMap* kclpm)
{
	ID3D11ShaderResourceView*const textureView[1] = {NULL};
	DX::getContext()->PSSetShaderResources(11, 1, textureView);

	DXB::PlanarMap* pm = dynamic_cast<DXB::PlanarMap*>(kclpm);

	pm->Bind();

	pm->Clear();
	
	DX::getStateManager()->SetRasterFrontClockwise(true);

	if(UseZPrePass())
		RenderPrepass(&pm->m_camera, pm->m_visible_meshes[0], 0, pm, 1, 0);
	else
		Render( &pm->m_camera, pm->m_visible_meshes[0], 0, pm, 1, 0);
	
	Render( &pm->m_camera, pm->m_visible_meshes[1], 0, pm, 1, 0);

	DX::getStateManager()->SetRasterFrontClockwise(false);

	pm->Unbind();
}


void DXB_Scene::RenderShadow( ShadowMap* sm)
{
	ID3D11ShaderResourceView* nullResource = NULL;
	for( KCL::uint32 i=0; i < m_num_shadow_maps; i++)
	{
		DX::getContext()->PSSetShaderResources(10+i, 1, &nullResource);
	}

	sm->Bind();

	sm->Clear();

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

	m_shadowStaticReceiverMeshes = sm->m_receiver_meshes[0];

	sm->Unbind();
}

void DXB_Scene::Render( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light, int pass_type)
{
	DXB::Material *override_material = dynamic_cast<DXB::Material*>(_override_material);
	DXB::Material *last_material = NULL;
	int last_mesh_type = -1;

	float normalized_time = (float) (m_animation_time % 100000) / 100000.0f;

	for( uint32 j=0; j<visible_meshes.size(); j++)
	{
		Mesh* sm = (Mesh*)visible_meshes[j];

		if( !sm->m_mesh)
		{
			continue;
		}

		int mesh_type = sm->m_mesh->m_vertex_matrix_indices.size() != 0;

        KRL::Mesh3 *krl_mesh  = (KRL::Mesh3 *)sm->m_mesh;
        if( krl_mesh->m_instances[sm->m_material].size() > 1)
		{
			std::set<KCL::Material*>::iterator iter = krl_mesh->m_is_rendered.find( sm->m_material);

			if( iter != krl_mesh->m_is_rendered.end())
			{
				continue;
			}
			
			mesh_type = 2;
		}

		DXB::Material *material = dynamic_cast<DXB::Material*>(sm->m_material);

		if( override_material)
		{
			material = override_material;
		}

		/* force use of shader[1][?] for depth prepass */
		int shader_bank = (pass_type == -1) ? 1 : 0;

		Shader *s = material->m_shaders[shader_bank][mesh_type];
		KRL_CubeEnvMap *envmaps[2];
		float envmaps_interpolator = 0.0f;
		Matrix4x4 mvp;
		Matrix4x4 mv;
		Matrix4x4 model;
		Matrix4x4 inv_model;
		Vector3D pos( sm->m_world_pom.v[12], sm->m_world_pom.v[13], sm->m_world_pom.v[14]);

		KCL::uint32 texture_num_from_material = 0;
		if( last_material != material || last_mesh_type != mesh_type)
		{
			if( last_material)
			{
				last_material->postInit();
			}

			material->PlayVideo( m_animation_time * m_animation_multiplier / 1000.0f);
			material->preInit(texture_num_from_material, mesh_type, pass_type);
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
				model = sm->m_world_pom;
				inv_model = Matrix4x4::Invert4x3( sm->m_world_pom);
				break;
			}
        case 2:
		case 1:
			{
				mvp = camera->GetViewProjection();
				mv = camera->GetView();
				model.identity();
				inv_model.identity();
				break;
			}
		}

		bool isParticle = m_particle_geometry == sm->m_mesh;

		if (isParticle)
		{
			ConstantBufferParticle *buf= (ConstantBufferParticle*)m_constantBufferParticle->map();
			//	mat4 	mvp
			buf->mvp=DX::Float4x4toXMFloat4x4ATransposed(mvp.v);
			//	mat4 	mv
			buf->mv = DX::Float4x4toXMFloat4x4ATransposed(mv.v);
			//	mat4 	model
			buf->model = DX::Float4x4toXMFloat4x4ATransposed(model.v);
			//	mat4 	world_fit
			buf->world_fit = DX::Float4x4toXMFloat4x4ATransposed(m_world_fit_matrix.v);
			
			//m_constantBufferParticleData.color = XMFLOAT4(sm->m_color.x, sm->m_color.y, sm->m_color.z, 1.0f);
			
			for (int i=0;i<MAX_PARTICLE_PER_MESH;i++)
			{
				buf->particle_data[i] = XMFLOAT4(m_particle_data[sm->m_offset1+i].v);
				buf->particle_color[i] = XMFLOAT4(m_particle_color[sm->m_offset1+i].v);
			}
			
			m_constantBufferParticle->unmap();
		}
		else if (m_mblurMaterial!=0 && m_mblurMaterial==override_material)
		{
			ConstantBufferMBlur *buf= (ConstantBufferMBlur*)m_constantBufferMBlur->map();
			//	mat4 	mvp
			buf->mvp = DX::Float4x4toXMFloat4x4ATransposed(mvp.v);
			
			switch( mesh_type)
			{
			case 0:
				{
					KCL::Matrix4x4 mvp2;
					mvp2 = sm->m_prev_world_pom * m_prev_vp;
					//	mat4 	mvp2
					buf->mvp2 = DX::Float4x4toXMFloat4x4ATransposed(mvp2.v);
					break;
				}
			case 1:
				{
					//	mat4 	mvp2
					buf->mvp2 = DX::Float4x4toXMFloat4x4ATransposed(m_prev_vp.v);
					break;
				}
			}
			if( sm->m_mesh->m_vertex_matrix_indices.size())
			{
				for (int i = 0;i < 32; i++)
				{
					
					buf->bones[i] = DirectX::XMFLOAT4X3( sm->m_mesh->m_node_matrices.data() + i * 4 * 3 );
					buf->prev_bones[i] = DirectX::XMFLOAT4X3( sm->m_mesh->m_prev_node_matrices.data() + i * 4 * 3 );
				}
			}
			
			m_constantBufferMBlur->unmap();
		}
		else
		{
			ConstantBuffer *buf = (ConstantBuffer*)m_constantBuffer->map();

			//	mat4 	mvp
			buf->mvp = DX::Float4x4toXMFloat4x4ATransposed(mvp.v);
			//	mat4 	mv
			buf->mv = DX::Float4x4toXMFloat4x4ATransposed(mv.v);

			//	mat4 	bone_orientations[32]
			//	vec4 	bone_positions[32]
			if (sm->m_mesh->m_vertex_matrix_indices.size()) 
			{
				for (int i=0;i<32;i++)
				{
					buf->bones[i] = DirectX::XMFLOAT4X3( sm->m_mesh->m_node_matrices.data() + i * 4 * 3);
				}
			}

			//	mat4 	model
			buf->model = DX::Float4x4toXMFloat4x4ATransposed(model.v);
			//	mat4 	inv_model
			buf->inv_model = DirectX::XMFLOAT4X4A(inv_model.v);

			//Matrix4x4 inv_mv = Matrix4x4::Invert4x3(mv);
            Matrix4x4 inv_mv;
            Matrix4x4::InvertModelView(mv, inv_mv);
			buf->inv_mv = DirectX::XMFLOAT4X4A(inv_mv.v); //DX::Float4x4toXMFloat4x4ATransposed(inv_mv.v);

			//	vec3 	global_light_dir
			buf->global_light_dir = XMFLOAT4A(m_light_dir.x, m_light_dir.y, m_light_dir.z, 0.0f);
			//	vec3 	global_light_color
			buf->global_light_color = XMFLOAT4A(m_light_color.x, m_light_color.x, m_light_color.z, 0.0f);

			//	vec3 	view_dir
			buf->view_dir = XMFLOAT4A(-camera->GetView().v[2], -camera->GetView().v[6], -camera->GetView().v[10], 0.0f);
			//	vec3 	view_pos
			buf->view_pos = XMFLOAT4A(camera->GetEye().x, camera->GetEye().y, camera->GetEye().z, 0.0f);

			//	float	time
			buf->time = normalized_time;

			//	vec3 	background_color
			buf->background_color = XMFLOAT4A(m_background_color.x, m_background_color.y, m_background_color.z, 0.0f);
			//	float	fog_density
			buf->fog_density = m_fog_density;

			//	float	diffuse_intensity
			buf->diffuse_intensity = material->m_diffuse_intensity;
			//	float	specular_intensity
			buf->specular_intensity = material->m_specular_intensity;
			//	float	reflect_intensity
			buf->reflect_intensity = material->m_reflect_intensity;
			//	float	specular_exponent
			buf->specular_exponent = material->m_specular_exponent;
			//	float	transparency
			buf->transparency = material->m_transparency;
			buf->fresnel_params = XMFLOAT4A(material->m_fresnel_params.v);
		
			//	float	envmaps_interpolator
			if( m_cubemaps.size())
			{
				Get2Closest( pos, envmaps[0], envmaps[1], envmaps_interpolator);
			
				buf->envmaps_interpolator = envmaps_interpolator;

			
				ID3D11ShaderResourceView* envmap0 = ((CubeEnvMap*)envmaps[0])->getD3D11Id().Get();
				ID3D11ShaderResourceView* envmap1 = ((CubeEnvMap*)envmaps[1])->getD3D11Id().Get();

				DX::getContext()->PSSetShaderResources(8, 1, &envmap0);
				DX::getContext()->PSSetShaderResources(9, 1, &envmap1);

			}

			//	vec3 	light_pos
			buf->light_pos = XMFLOAT4A(0,0,0,0);
            buf->light_color = XMFLOAT4A(0,0,0,0);
            if(light)
            {
                Vector3D light_pos(light->m_world_pom.v[12], light->m_world_pom.v[13], light->m_world_pom.v[14]);
		        buf->light_pos = DirectX::XMFLOAT4(light_pos.x, light_pos.y, light_pos.z, 1);

                float i = 1.0f;
	            if (light->m_intensity_track)
	            {
		            Vector4D v;
		            light->t = m_animation_time / 1000.0f;
		            _key_node::Get( v, light->m_intensity_track, light->t, light->tb, true);
		            i = v.x / light->m_intensity;
	            }

	            buf->light_color = DirectX::XMFLOAT4(
		            light->m_diffuse_color.x * i, 
		            light->m_diffuse_color.y * i, 
		            light->m_diffuse_color.z * i, 1);
            }
			
			ID3D11ShaderResourceView*const textureView[1] = {NULL};
			for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
			{
				DX::getContext()->PSSetShaderResources(10+i, 1, textureView);
			}
			//	mat4 	
			if( m_num_shadow_maps && override_material != m_shadowCasterMaterial)
			{
				for( KCL::uint32 i=0; i<m_num_shadow_maps; i++)
				{
					if (m_global_shadowmaps[i])
					{
						ID3D11ShaderResourceView* textureView = ((ShadowMap*)m_global_shadowmaps[i])->GetD3DTextureId();
						DX::getContext()->PSSetShaderResources(10 + i, 1, &textureView);
					}
				}
				//TODO dinamikusan ezt is
				if (m_global_shadowmaps[0])
					buf->shadow_matrix0 = DX::Float4x4toXMFloat4x4ATransposed(m_global_shadowmaps[0]->m_matrix.v);
				if (m_global_shadowmaps[1])
					buf->shadow_matrix1 = DX::Float4x4toXMFloat4x4ATransposed(m_global_shadowmaps[1]->m_matrix.v);
				
			}

			//	vec2 	offset_2d
			buf->offset_2d = XMFLOAT4A();
			//	vec2 	inv_resolution
			//m_constantBufferData.inv_resolution = XMFLOAT4A(1.0f / m_viewport_width, 1.0f / m_viewport_height, 0.0f, 0.0f);
			//	float 	screen_width
			//m_constantBufferData.screen_width = m_viewport_width;
			//	float 	screen_height
			//m_constantBufferData.screen_height = m_viewport_height;
			//	vec3 	color
			buf->color = XMFLOAT4A(sm->m_color.x, sm->m_color.y, sm->m_color.z, 0.0f);

		
			//	vec2 	translate_uv
			buf->screen_resolution = XMFLOAT2A(m_viewport_width, m_viewport_height);
			buf->inv_screen_resolution = XMFLOAT2A(1.0f / m_viewport_width, 1.0f / m_viewport_height);
			buf->depth_parameters = XMFLOAT4A(m_active_camera->m_depth_linearize_factors.v);

			if( 1 )
			{
				if( sm->m_material->m_frame_when_animated != m_frame)
				{
                    if (sm->m_material->m_animation_time > m_animation_time / 1000.0f)
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

						_key_node::Get(r, sm->m_material->m_translate_v_track, sm->m_material->m_animation_time, sm->m_material->m_animation_time_base, true);

						sm->m_material->m_uv_offset.y = -r.x;
					}

					sm->m_material->m_frame_when_animated = m_frame;
				}			
			
				buf->translate_uv = XMFLOAT4A(sm->m_material->m_uv_offset.x, sm->m_material->m_uv_offset.y, 0.0f, 0.0f);
			}
		
			//	float	camera_focus
			buf->camera_focus = m_camera_focus_distance;

			buf->alpha_threshold = sm->m_alpha;

			buf->mblur_mask = sm->m_is_motion_blurred ? 1.0f : 0.0f;

			//TODO: bind textures

			if( override_material != 0 && override_material->getPlanarMap())
			{
				ID3D11ShaderResourceView* textureView = override_material->getPlanarMap()->m_FBO->Get();
				DX::getContext()->PSSetShaderResources(11, 1, &textureView);
			}

			m_constantBuffer->unmap();
		}

		VboPool::Instance()->BindBuffer(sm->m_mesh->m_vbo);
		IndexBufferPool::Instance()->BindBuffer(sm->m_mesh->m_ebo[lod].m_buffer);

		
		DX::getStateManager()->ApplyStates();

        DXB::Mesh3* sm_mesh = (DXB::Mesh3*)sm->m_mesh;

        if(m_scene_version == KCL::SV_30)
        {
            if( mesh_type < 2) //skinned or basic
            {
                //TODO: override_material inputlayout fix
                if(mesh_type == 0)
		        {
                    sm_mesh->BindLayout30( material->m_shaders[shader_bank][mesh_type]);
                }
                else
                {
                    sm_mesh->BindLayout30_Skinned( material->m_shaders[shader_bank][mesh_type]);
                }

		        DX::getContext()->DrawIndexed(
			        ( sm->m_primitive_count ? sm->m_primitive_count : sm->m_mesh->getIndexCount(lod)),
			        0,
			        0
			        );
            }
            else //use instancing, never skinned
            {
                krl_mesh->m_is_rendered.insert( sm->m_material);

                sm_mesh->BindInstanceVBO();

                sm_mesh->BindLayout30_Instanced( material->m_shaders[shader_bank][mesh_type]);

                DX::getContext()->DrawIndexedInstanced(
			        ( sm->m_primitive_count ? sm->m_primitive_count : sm->m_mesh->getIndexCount(lod)),
                    krl_mesh->m_instances[sm->m_material].size(),
			        0,
			        0,
			        0
                    );
            }
        }
        else //use 2 texcoords, can be skinned or basic
        {
                //TODO: override_material inputlayout fix
                if(mesh_type == 0)
		        {
                    sm_mesh->BindLayout27( material->m_shaders[shader_bank][mesh_type]);
                }
                else
                {
                    sm_mesh->BindLayout27_Skinned( material->m_shaders[shader_bank][mesh_type]);
                }

		        DX::getContext()->DrawIndexed(
			        ( sm->m_primitive_count ? sm->m_primitive_count : sm->m_mesh->getIndexCount(lod)),
			        0,
			        0
			        );
        }


		m_num_draw_calls++;
		m_num_triangles += sm->m_mesh->getIndexCount(lod) / 3;
		m_num_vertices += sm->m_mesh->getIndexCount(lod);
	}

	if(last_material)
	{
		last_material->postInit();
	}

#ifdef USE_VBO
	VboPool::Instance()->BindBuffer(0);
	IndexBufferPool::Instance()->BindBuffer(0);
#endif

}

void DXB_Scene::Render( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light)
{
	DXB_Scene::Render( camera, visible_meshes, _override_material, pm, lod, light, 0);
}

void DXB_Scene::RenderPrepass( Camera2* camera, std::vector<Mesh*> &visible_meshes, KCL::Material *_override_material, KCL::PlanarMap* pm, KCL::uint32 lod, KCL::Light* light)
{
	std::vector<Mesh *> default_visible_meshes;
	std::vector<Mesh *> foliage_visible_meshes;
	size_t i;

	/* filter opaque meshes (DEFAULT|FOLIAGE) for prepass */
	for (i = 0; i < visible_meshes.size(); i++)
	{
		if (visible_meshes[i]->m_material->m_material_type == KCL::Material::DEFAULT)
			default_visible_meshes.push_back(visible_meshes[i]);

		if (visible_meshes[i]->m_material->m_material_type == KCL::Material::FOLIAGE)
			foliage_visible_meshes.push_back(visible_meshes[i]);
	}

	/* render opaque_visible_meshes to depth only */
	DX::getStateManager()->SetBlendColorMask(0);
	Render( camera, default_visible_meshes, _override_material, pm, lod, light, -1);
	Render( camera, foliage_visible_meshes, _override_material, pm, lod, light, -1);


	/* dispatch shadepass */
	DX::getStateManager()->SetBlendColorMask(D3D11_COLOR_WRITE_ENABLE_ALL);
	DXB_Scene::Render( camera, visible_meshes, _override_material, pm, lod, light, +1);
}

CubeEnvMap *DXB_Scene::CreateEnvMap( const GLB::Vector3D &pos, uint32 idx)
{
	std::string envmap_path = EnvmapsDirectory();
	CubeEnvMap *cubemap = CubeEnvMap::Load( idx, envmap_path.c_str());

	if( !cubemap)
	{
#ifndef HAVE_DX
		INFO("Generate environment  maps...");
		std::vector<Mesh*> visible_meshes[2];
		std::vector<KCL::PlanarMap*> visible_planar_maps;
		std::vector<KCL::Mesh*> meshes_to_blur;

		FboEnvMap* m_fboEnvMap = new FboEnvMap( m_fboEnvMap_size);
		Camera2 camera;

		cubemap = m_fboEnvMap->CreateCubeEnvMap( pos);

		camera.Perspective(90.0f, 1, 1, 0.01f, 1000.0f);

		glViewport( 0, 0, m_fboEnvMap->GetSize(), m_fboEnvMap->GetSize());

		m_fboEnvMap->Bind();
		for(size_t i = 0; i < 6; ++i)
		{
			m_fboEnvMap->AttachCubemap(cubemap, i);
			camera.LookAt(cubemap->GetPosition(), cubemap->GetPosition()+refs[i], ups[i]);
			camera.Update();

			visible_meshes[0].clear();
			visible_meshes[1].clear();

			Vector2D nearfar;

			FrustumCull(&camera, visible_planar_maps, visible_meshes, meshes_to_blur, m_pvs, nearfar, 0, true);

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
#endif
	}

	if (cubemap)
	{
		cubemap->SetPosition(pos);
	}

	return cubemap;
}

void DXB_Scene::OcclusionCullLights()
{

}

void DXB_Scene::GeneratePVS()
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

	glUseProgram( m_shader->m_p);
	glGenQueries( 1, &cicc);

	camera.Perspective(90.0f, w, w, 0.01f, 1000.0f);

	glViewport( 0, 0, w, w);
	glEnable( GL_DEPTH_TEST);

	glDepthFunc( GL_LESS);

	glEnableVertexAttribArray( m_shader->m_attrib_locations[0]);

	while( 1)
	{
		Matrix4x4 m = AnimateCamera( t);

		Vector3D pos( m.v[12], m.v[13], m.v[14]);

		for(size_t i=0; i<6; ++i)
		{
			std::vector<XRoom*> visible_rooms;

			camera.LookAt( pos, pos+refs[i], ups[i]);

			camera.Update();

			glUniformMatrix4fv( m_shader->m_uniform_locations[0], 1, GL_FALSE, camera.GetViewProjection().v);

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
					glBindBuffer( GL_ARRAY_BUFFER, n->m_meshes[k]->m_mesh->m_vbo);

					glVertexAttribPointer( m_shader->m_attrib_locations[0], 3, n->m_meshes[k]->m_mesh->m_vertex_attribs[0].m_type, 0, n->m_meshes[k]->m_mesh->m_vertex_attribs[0].m_stride, n->m_meshes[k]->m_mesh->m_vertex_attribs[0].m_data);

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
	glDisableVertexAttribArray( m_shader->m_attrib_locations[0]);

	glDisable( GL_DEPTH_TEST);

	glViewport( 0, 0, m_viewport_width, m_viewport_height);

	//m_bsp_tree->SavePVS( m_path.c_str());
#endif
#endif
}

KCL::uint32 Create3DTexture( std::vector<DXB::Image2D*> &images, bool clamp)
{
	return 0u;
}

KCL::uint32 LoadTexture3D( const std::string &name, bool clamp)
{
	int i = 1;
	char filename[512];
	std::vector<DXB::Image2D*> images;

	while( 1)
	{
		sprintf( filename, "%s_%04d.png", name.c_str(), i);
		DXB::Image2D *img = new DXB::Image2D();

		bool b = img->load( filename);
		if( !b)
		{
			delete img;
			break;
		}
		else
		{
			images.push_back( img);
		}
		i++;
	}

	KCL::uint32 texid = Create3DTexture( images, clamp);

	for( KCL::uint32 i=0; i<images.size(); i++)
	{
		delete images[i];
	}

	return texid;
}

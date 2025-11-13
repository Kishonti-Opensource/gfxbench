/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_debug_renderer.h"
#include "gfxb_shapes.h"
#include "gfxb_shader.h"
#include "gfxb_light.h"
#include "gfxb_particlesystem.h"

#include "gfxb_mesh_shape.h"
#include "gfxb_material.h"
#include "gfxb_texture.h"
#include "gfxb_shadow_map.h"

#include <kcl_scene_handler.h>
#include <kcl_camera2.h>
#include <kcl_mesh.h>
#include <kcl_actor.h>

#include <sstream>

using namespace GFXB;

const float DebugRenderer::SOLID = 10.0f;

DebugRenderer::DebugRenderer()
{
	m_width = 0;
	m_height = 0;

	m_shapes = nullptr;
	m_dir_light_shape = nullptr;

	m_sphere_shader = 0;
	m_cube_shader = 0;
	m_cone_shader = 0;

	m_blit_gamma_correction = true;
	m_blit_linearize_depth = false;
	m_blit_texture = 0;
	m_blit_shader = 0;
	m_blit_render = 0;

	m_debug_job = 0;

	m_mesh_shader = 0;
	m_mesh_alpha_test_shader = 0;
	m_mesh_skeletal_shader = 0;
	m_mesh_skeletal_alpha_test_shader = 0;
	m_line_shader = 0;

	SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
	SetPerimeterThreshold(SOLID);
	m_selected_meshes.reserve(32);
}


DebugRenderer::~DebugRenderer()
{
	delete m_dir_light_shape;
}


void DebugRenderer::Init(KCL::SceneHandler *scene, KCL::uint32 viewport_width, KCL::uint32 viewport_height, Shapes *shapes, KCL::uint32 color_texture, KCL::uint32 depth_texture)
{
	m_width = viewport_width;
	m_height = viewport_height;

	m_shapes = shapes;

	m_color_texture = color_texture;
	m_depth_texture = depth_texture;

	m_sphere_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_sphere.vert", "debug/debug_sphere.frag"));
	m_cube_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_cube.vert", "debug/debug_cube.frag"));
	m_cone_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_cone.vert", "debug/debug_cone.frag"));
	m_line_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_line.vert", "debug/debug_line.frag"));

	m_blit_texture = color_texture;
	m_blit_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_blit.vert", "debug/debug_blit.frag"));
	{
		NGL_job_descriptor rrd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = 0;
			ad.m_attachment_load_op = NGL_LOAD_OP_CLEAR;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			rrd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_name = "debug_blit";
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			rrd.m_subpasses.push_back(sp);
		}

		rrd.m_load_shader_callback = LoadShader;
		m_blit_render = nglGenJob(rrd);

		int32_t viewport[4] =
		{
			0, 0, (int32_t)viewport_width, (int32_t)viewport_height
		};

		nglViewportScissor(m_blit_render, viewport, viewport);
	}
	{
		NGL_job_descriptor jd;
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_color_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_attachment_descriptor ad;
			ad.m_attachment.m_idx = m_depth_texture;
			ad.m_attachment_load_op = NGL_LOAD_OP_LOAD;
			ad.m_attachment_store_op = NGL_STORE_OP_STORE;
			jd.m_attachments.push_back(ad);
		}
		{
			NGL_subpass sp;
			sp.m_usages.push_back(NGL_COLOR_ATTACHMENT);
			sp.m_usages.push_back(NGL_READ_ONLY_DEPTH_ATTACHMENT);
			jd.m_subpasses.push_back(sp);
		}

		jd.m_load_shader_callback = LoadShader;
		m_debug_job = nglGenJob(jd);
	}

	// Load the mesh shaders
	{
		m_mesh_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_mesh.vert", "debug/debug_mesh.frag"));
		m_mesh_alpha_test_shader = ShaderFactory::GetInstance()->AddDescriptor(ShaderDescriptor("debug/debug_mesh.vert", "debug/debug_mesh.frag").AddDefine("ALPHA_TEST"));

		// Skeletal mesh
		ShaderDescriptor skeletal_desc("debug/debug_mesh.vert", "debug/debug_mesh.frag");
		skeletal_desc.AddDefineInt("SKELETAL", 1);
		skeletal_desc.AddDefineInt("MAX_BONES", 3 * KCL::Mesh3::MAX_BONES); // One bone needs 3 vec4
		m_mesh_skeletal_shader = ShaderFactory::GetInstance()->AddDescriptor(skeletal_desc);

		skeletal_desc.AddDefine("ALPHA_TEST");
		m_mesh_skeletal_alpha_test_shader = ShaderFactory::GetInstance()->AddDescriptor(skeletal_desc);
	}


	// Load the debug mesh for the directional light shape
	{
		std::string filename = "meshes/light_dir_shapeShape";
		KCL::AssetFile file(filename);
		if (file.Opened())
		{
			m_dir_light_shape = (Mesh3*)scene->Mesh3Factory().Create(filename.c_str());
			scene->ReadMeshGeometry(m_dir_light_shape, nullptr, file);
			m_dir_light_shape->UploadMesh();
		}
		else
		{
			INFO("DebugRenderer: Can not load mesh: %s", filename.c_str());
			m_dir_light_shape = nullptr;
		}
	}
}


void DebugRenderer::UpdateViewport(KCL::uint32 viewport_x, KCL::uint32 viewport_y, KCL::uint32 viewport_width, KCL::uint32 viewport_height)
{
	m_width = viewport_width;
	m_height = viewport_height;

	// Offscreen renderer
	KCL::int32 vp[4] = { 0, 0, (int32_t)m_width, (int32_t)m_height };
	nglViewportScissor(m_debug_job, vp, vp);

	// Onscreen renderer
	vp[0] = (int32_t)viewport_x;
	vp[1] = (int32_t)viewport_y;
	nglViewportScissor(m_blit_render, vp, vp);
}


void DebugRenderer::DeleteRenderers()
{
	nglDeletePipelines(m_debug_job);
	nglDeletePipelines(m_blit_render);
}


DebugRenderer &DebugRenderer::DrawLine(const KCL::Vector3D &pos0, const KCL::Vector3D &pos1)
{
	m_state.m_mesh_type = LINE;
	m_state.m_position0 = KCL::Vector4D(pos0, 1.0f);
	m_state.m_position1 = KCL::Vector4D(pos1, 1.0f);

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawLine(const KCL::Vector3D &pos0, const KCL::Vector3D &color0, const KCL::Vector3D &pos1, const KCL::Vector3D &color1)
{
	m_state.m_mesh_type = LINE;
	m_state.m_position0 = KCL::Vector4D(pos0, 1.0f);
	m_state.m_position1 = KCL::Vector4D(pos1, 1.0f);
	m_state.m_color0 = KCL::Vector4D(color0, 1.0f);
	m_state.m_color1 = KCL::Vector4D(color1, 1.0f);

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawSphere(const KCL::Vector3D &center, float radius)
{
	m_state.m_model.identity();
	m_state.m_mesh_type = SPHERE;
	m_state.m_position0 = KCL::Vector4D(center, 1.0f);
	m_state.m_scale = KCL::Vector4D(radius, radius, radius, 1.0f);

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawCone(const KCL::Matrix4x4 &model)
{
	m_state.m_mesh_type = CONE;
	m_state.m_model = model;

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawAABB(const KCL::Vector3D &center, const KCL::Vector3D &size)
{
	m_state.m_model.identity();
	m_state.m_mesh_type = AABB;
	m_state.m_position0 = KCL::Vector4D(center, 1.0f);
	m_state.m_scale = KCL::Vector4D(size, 1.0f);

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawAABB(const KCL::AABB &aabb)
{
	KCL::Vector3D half_extent;
	KCL::Vector3D center;
	aabb.CalculateHalfExtentCenter(half_extent, center);
	return DrawAABB(center, half_extent);
}


DebugRenderer &DebugRenderer::DrawAABB(const KCL::Mesh *mesh)
{
	return DrawAABB(mesh->m_aabb);
}


DebugRenderer &DebugRenderer::DrawCube(const KCL::Matrix4x4 &model)
{
	m_state.m_model = model;
	m_state.m_mesh_type = CUBE;
	m_state.m_position0.set(0.0f, 0.0f, 0.0f, 1.0f);
	m_state.m_scale.set(1.0f, 1.0f, 1.0f, 1.0f);

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawMesh(const KCL::Matrix4x4 &model, KCL::Mesh3 *mesh3)
{
	m_state.m_model = model;
	m_state.m_mesh_type = MESH_SHAPE;
	m_state.m_mesh3 = (GFXB::Mesh3*)mesh3;

	Commit();
	return *this;
}


DebugRenderer &DebugRenderer::DrawMesh(const KCL::Vector3D &center, KCL::Mesh3 *mesh3)
{
	KCL::Matrix4x4 model;
	model.identity();
	model.translate(center);
	return DrawMesh(model, mesh3);
}


DebugRenderer &DebugRenderer::DrawMesh(KCL::Mesh *mesh)
{
	m_selected_meshes.push_back(mesh);
	return *this;
}


DebugRenderer &DebugRenderer::DrawCamera(const KCL::Camera2 *camera)
{
	m_state.m_position0.set(0.0f, 0.0f, 0.0f, 0.0f);
	m_state.m_scale.set(1.0f, 1.0f, 1.0f, 1.0f);

	m_state.m_mesh_type = AABB;
	KCL::Matrix4x4::Invert4x4(camera->GetViewProjection(), m_state.m_model);

	m_state.m_color0.w = 0.2f;
	SetDepthTestMode(NGL_DEPTH_DISABLED);
	SetBlendMode(NGL_BLEND_ALFA);
	SetCullMode(NGL_TWO_SIDED);

	m_state.m_perimeter_threshold = 0.02f;
	Commit();

	m_state.m_mesh_type = AABB;
	KCL::Matrix4x4::Invert4x4(camera->GetViewProjection(), m_state.m_model);

	m_state.m_color0.w = 1.0f;
	SetDepthTestMode(NGL_DEPTH_LESS);
	SetBlendMode(NGL_BLEND_DISABLED);
	SetCullMode(NGL_TWO_SIDED);

	m_state.m_perimeter_threshold = 0.02f;
	Commit();

	return *this;
}


void DebugRenderer::DrawDirectionalLight(const KCL::Matrix4x4 &model)
{
	SetCullMode(NGL_BACK_SIDED);
	SetBlendMode(NGL_BLEND_ALFA);
	SetDepthTestMode(NGL_DEPTH_GREATER);
	SetColor(KCL::Vector4D(1.0f, 1.0f, 0.0f, 0.5f));
	DrawMesh(model, m_dir_light_shape);

	SetCullMode(NGL_FRONT_SIDED);
	SetBlendMode(NGL_BLEND_DISABLED);
	SetDepthTestMode(NGL_DEPTH_LESS);
	DrawMesh(model, m_dir_light_shape);
}


DebugRenderer &DebugRenderer::DrawLight(const KCL::Light *light, const KCL::Vector3D &camera_pos)
{
	Light *gfxb_light = (Light*)light;

	switch (light->m_light_shape->m_light_type)
	{
		case KCL::LightShape::OMNI:
		{
			KCL::Vector3D center = KCL::Vector3D(light->m_world_pom.v41, light->m_world_pom.v42, light->m_world_pom.v43);
			// Draw center
			SetPerimeterThreshold(0.4f);
			SetBlendMode(NGL_BLEND_ALFA);
			SetDepthTestMode(NGL_DEPTH_DISABLED);
			SetColor(KCL::Vector4D(1.0f, 1.0f, 0.0f, 0.2f));
			DrawSphere(center, 0.4f);

			SetBlendMode(NGL_BLEND_DISABLED);
			SetDepthTestMode(NGL_DEPTH_LESS);
			SetColor(KCL::Vector4D(1.0f, 1.0f, 0.0f, 1.0f));
			DrawSphere(center, 0.4f);

			// Draw radius
			SetPerimeterThreshold(0.2f);

			SetBlendMode(NGL_BLEND_ALFA);
			SetDepthTestMode(NGL_DEPTH_DISABLED);
			SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 0.2f));
			DrawSphere(center, light->m_light_shape->m_radius);

			SetBlendMode(NGL_BLEND_DISABLED);
			SetDepthTestMode(NGL_DEPTH_LESS);
			SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
			DrawSphere(center, light->m_light_shape->m_radius);

			if (light->m_light_shape->m_is_shadow_caster)
			{
				KCL::Vector3D shadow_center = KCL::Vector3D(gfxb_light->GetShadowMap()->GetLight()->m_uniform_pos.v);
				float shadow_near = gfxb_light->GetShadowMap()->GetLight()->m_shadow_near;
				float shadow_far = gfxb_light->GetShadowMap()->GetLight()->m_shadow_far;

				// Shadow near
				SetPerimeterThreshold(0.3f);
				SetBlendMode(NGL_BLEND_ALFA);
				SetDepthTestMode(NGL_DEPTH_DISABLED);
				SetColor(KCL::Vector4D(0.0f, 0.0f, 1.0f, 0.2f));
				DrawSphere(shadow_center, shadow_near);

				SetBlendMode(NGL_BLEND_DISABLED);
				SetDepthTestMode(NGL_DEPTH_LESS);
				SetColor(KCL::Vector4D(0.0f, 0.0f, 1.0f, 1.0f));
				DrawSphere(shadow_center, shadow_near);

				// Shadow far
				SetPerimeterThreshold(0.2f);
				SetBlendMode(NGL_BLEND_ALFA);
				SetDepthTestMode(NGL_DEPTH_DISABLED);
				SetColor(KCL::Vector4D(0.0f, 0.0f, 1.0f, 0.2f));
				DrawSphere(shadow_center, shadow_far);

				SetBlendMode(NGL_BLEND_DISABLED);
				SetDepthTestMode(NGL_DEPTH_LESS);
				SetColor(KCL::Vector4D(0.0f, 0.0f, 1.0f, 1.0f));
				DrawSphere(shadow_center, shadow_far);
			}
			break;
		}

		case KCL::LightShape::SPOT:
		{
			SetPerimeterThreshold(0.2f);
			SetCullMode(NGL_BACK_SIDED);
			SetBlendMode(NGL_BLEND_ALFA);
			SetDepthTestMode(NGL_DEPTH_LESS);
			SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 0.3f));
			DrawCone(gfxb_light->m_uniform_shape_world_pom);
			break;
		}
		case KCL::LightShape::DIRECTIONAL:
			if (gfxb_light->m_light_shape->m_box_light)
			{
				SetCullMode(NGL_TWO_SIDED);

				SetPerimeterThreshold(0.01f);

				SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 0.2f));
				SetDepthTestMode(NGL_DEPTH_DISABLED);
				SetBlendMode(NGL_BLEND_ALFA);
				DrawCube(gfxb_light->m_uniform_shape_world_pom);

				SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 1.0f));
				SetDepthTestMode(NGL_DEPTH_LESS);
				SetBlendMode(NGL_BLEND_DISABLED);
				DrawCube(gfxb_light->m_uniform_shape_world_pom);

				//KCL::Matrix4x4 &m = gfxb_light->m_uniform_shape_world_pom;
			}
			else if (m_dir_light_shape != nullptr)
			{
				KCL::Vector3D light_dir = KCL::Vector3D(light->m_world_pom.v[8], light->m_world_pom.v[9], light->m_world_pom.v[10]);
				light_dir.normalize();

				KCL::Matrix4x4 model;

				KCL::Vector3D t = KCL::Vector3D(light->m_world_pom.v[0], light->m_world_pom.v[1], light->m_world_pom.v[2]).normalize();
				KCL::Vector3D b = KCL::Vector3D(light->m_world_pom.v[4], light->m_world_pom.v[5], light->m_world_pom.v[6]).normalize();
				KCL::Vector3D n = KCL::Vector3D(light->m_world_pom.v[8], light->m_world_pom.v[9], light->m_world_pom.v[10]).normalize();

				model.v[0] = t.x; model.v[1] = t.y; model.v[2] = t.z;
				model.v[4] = b.x; model.v[5] = b.y; model.v[6] = b.z;
				model.v[8] = n.x; model.v[9] = n.y;	model.v[10] = n.z;

				const float arrow_dist = 3.0;
				for (int y = -1; y <= 1; y++)
				{
					for (int x = -3; x <= 3; x++)
					{
						for (int z = -3; z <= 3; z++)
						{
							KCL::Vector3D pos = camera_pos + KCL::Vector3D(float(x), float(y), float(z)) * arrow_dist;
							model.v[12] = pos.x; model.v[13] = pos.y; model.v[14] = pos.z;
							DrawDirectionalLight(model);
						}
					}
				}
			}
			break;

		default:
			break;
	}

	return *this;
}

DebugRenderer &DebugRenderer::DrawParticleEmitter(const GFXB::ParticleEmitter *particle_emitter)
{
	using namespace KCL;

	SetColor(KCL::Vector4D(1.0f, 1.0f, 1.0f, 0.5f));
	SetPerimeterThreshold(0.02f);
	SetDepthTestMode(NGL_DEPTH_DISABLED);
	SetBlendMode(NGL_BLEND_ALFA);
	DrawAABB(reinterpret_cast<const KCL::Vector3D&>(particle_emitter->m_emitter.m_pom.v41), KCL::Vector3D(1, 1, 1) * 0.3f);

	Matrix4x4 pom = particle_emitter->m_world_pom;//particle_emitter->m_emitter.pom;
	float focus_distance = particle_emitter->m_emitter.m_focus_distance;
	Vector3D emitter_pos = reinterpret_cast<Vector3D&>(pom.v41);
	Vector3D ap = particle_emitter->m_emitter.m_aperture * 0.5f;
	Vector3D dir_x = Vector3D::normalize(reinterpret_cast<KCL::Vector3D&>(pom.v11)) * ap.x;
	Vector3D dir_y = Vector3D::normalize(reinterpret_cast<KCL::Vector3D&>(pom.v21)) * ap.y;
	Vector3D dir_z = Vector3D::normalize(reinterpret_cast<KCL::Vector3D&>(pom.v31)) * ap.z;

	// aperture box

	DrawLine(
		-dir_x + dir_y + dir_z + emitter_pos,
		dir_x + dir_y + dir_z + emitter_pos
	);
	DrawLine(
		dir_x + -dir_y + dir_z + emitter_pos,
		dir_x + dir_y + dir_z + emitter_pos
	);
	DrawLine(
		dir_x + dir_y + -dir_z + emitter_pos,
		dir_x + dir_y + dir_z + emitter_pos
	);

	DrawLine(
		-dir_x + -dir_y + dir_z + emitter_pos,
		-dir_x + dir_y + dir_z + emitter_pos
	);
	DrawLine(
		-dir_x + dir_y + -dir_z + emitter_pos,
		-dir_x + dir_y + dir_z + emitter_pos
	);

	DrawLine(
		-dir_x + -dir_y + dir_z + emitter_pos,
		dir_x + -dir_y + dir_z + emitter_pos
	);
	DrawLine(
		dir_x + -dir_y + -dir_z + emitter_pos,
		dir_x + -dir_y + dir_z + emitter_pos
	);

	DrawLine(
		-dir_x + dir_y + -dir_z + emitter_pos,
		dir_x + dir_y +- dir_z + emitter_pos
	);
	DrawLine(
		dir_x + -dir_y + -dir_z + emitter_pos,
		dir_x + dir_y + -dir_z + emitter_pos
	);

	DrawLine(
		-dir_x + -dir_y + dir_z + emitter_pos,
		-dir_x + -dir_y + -dir_z + emitter_pos
	);
	DrawLine(
		dir_x + -dir_y + -dir_z + emitter_pos,
		-dir_x + -dir_y + -dir_z + emitter_pos
	);
	DrawLine(
		-dir_x + dir_y + -dir_z + emitter_pos,
		-dir_x + -dir_y + -dir_z + emitter_pos
	);

	// focus line

	Vector3D focal_point = Vector3D::normalize(reinterpret_cast<KCL::Vector3D&>(pom.v21)) * focus_distance + emitter_pos;
	DrawLine(emitter_pos, focal_point);
	SetPerimeterThreshold(0.00f);
	DrawSphere(focal_point, 0.1f);

	return *this;
}


void DebugRenderer::Commit()
{
	m_commands.push_back(m_state);
//	m_state.clear();
}


void DebugRenderer::ClearCommands()
{
	m_commands.clear();
}


void DebugRenderer::Render(KCL::uint32 command_buffer, const KCL::Camera2 *camera)
{
	KCL::Matrix4x4 vp = camera->GetViewProjection();
	KCL::Vector4D view_pos = KCL::Vector4D(camera->GetEye());
	KCL::Matrix4x4 mvp;

	const void *p[UNIFORM_MAX];
	memset(p, 0, sizeof(p));

	p[UNIFORM_VP] = vp.v;
	p[UNIFORM_VIEW_POS] = view_pos.v;

	p[UNIFORM_MVP] = mvp.v;

	int32_t viewport[4] =
	{
		0, 0, (int32_t)m_width, (int32_t)m_height
	};

	nglViewportScissor(m_debug_job, viewport, viewport);

	nglBegin(m_debug_job, command_buffer);
	for (size_t i = 0; i < m_commands.size(); ++i)
	{
		draw_command &cmd = m_commands[i];

		nglBlendStateAll(m_debug_job, cmd.m_blend_mode);
		nglDepthState(m_debug_job, cmd.m_depth_test_mode, false);

		mvp = cmd.m_model * vp;

		p[UNIFORM_MODEL] = &cmd.m_model;
		p[UNIFORM_POS0] = cmd.m_position0.v;
		p[UNIFORM_POS1] = cmd.m_position1.v;
		p[UNIFORM_CENTER] = cmd.m_position0.v;
		p[UNIFORM_SCALE] = cmd.m_scale.v;
		p[UNIFORM_COLOR0] = cmd.m_color0.v;
		p[UNIFORM_COLOR1] = cmd.m_color1.v;
		p[UNIFORM_PERIMETER_THRESHOLD] = &cmd.m_perimeter_threshold;

		switch (cmd.m_mesh_type)
		{
			case SPHERE:
				nglDraw(m_debug_job, NGL_TRIANGLES, m_sphere_shader, 1, &m_shapes->m_sphere_ibid, m_shapes->m_sphere_ibid, cmd.m_cull_mode, p);
				break;

			case CYLINDER:
				nglDraw(m_debug_job, NGL_TRIANGLES, m_cone_shader, 1, &m_shapes->m_cylinder_vbid, m_shapes->m_cylinder_ibid, cmd.m_cull_mode, p);
				break;

			case CONE:
				nglDraw(m_debug_job, NGL_TRIANGLES, m_cone_shader, 1, &m_shapes->m_cone_vbid, m_shapes->m_cone_ibid, cmd.m_cull_mode, p);
				break;

			case AABB:
				nglDraw(m_debug_job, NGL_TRIANGLES, m_cube_shader, 1, &m_shapes->m_cube_vbid, m_shapes->m_cube_ibid, cmd.m_cull_mode, p);
				break;

			case CUBE:
				nglDraw(m_debug_job, NGL_TRIANGLES, m_cube_shader, 1, &m_shapes->m_cube_vbid, m_shapes->m_cube_ibid, cmd.m_cull_mode, p);
				break;

			case MESH_SHAPE:
				nglDraw(m_debug_job, NGL_TRIANGLES, m_mesh_shader, 1, &cmd.m_mesh3->m_vbid, cmd.m_mesh3->m_ibid, cmd.m_cull_mode, p);
				break;

			case LINE:
				nglDraw(m_debug_job, NGL_LINES, m_line_shader, 1, &m_shapes->m_line_vbid, m_shapes->m_line_ibid, NGL_TWO_SIDED, p);
				break;

			default:
				break;
		}
	}
	nglEnd(m_debug_job);

	ClearCommands();
}


void DebugRenderer::RenderOnscreen(KCL::uint32 command_buffer)
{
	const void *p[UNIFORM_MAX];
	p[UNIFORM_TEXTURE_UNIT0] = &m_blit_texture;

	nglBegin(m_blit_render, command_buffer);
	nglDrawTwoSided(m_blit_render, m_blit_shader, m_shapes->m_fullscreen_vbid, m_shapes->m_fullscreen_ibid, p);
	nglEnd(m_blit_render);
}


DebugRenderer &DebugRenderer::SetColor(const KCL::Vector4D &c)
{
	m_state.m_color0 = c;
	m_state.m_color1 = c;
	return *this;
}


DebugRenderer &DebugRenderer::SetColor(const KCL::Vector3D &c)
{
	m_state.m_color0.set(c.x, c.y, c.z, 1.0f);
	m_state.m_color1.set(c.x, c.y, c.z, 1.0f);
	return *this;
}


DebugRenderer &DebugRenderer::SetPerimeterThreshold(float threshold)
{
	m_state.m_perimeter_threshold = threshold;
	return *this;
}


DebugRenderer &DebugRenderer::SetCullMode(NGL_cull_mode cull_mode)
{
	m_state.m_cull_mode = cull_mode;
	return *this;
}


DebugRenderer &DebugRenderer::SetBlendMode(NGL_blend_func blend_mode)
{
	m_state.m_blend_mode = blend_mode;
	return *this;
}


DebugRenderer &DebugRenderer::SetDepthTestMode(NGL_depth_func depth_test_mode)
{
	m_state.m_depth_test_mode = depth_test_mode;
	return *this;
}


void DebugRenderer::SetBlitTexture(KCL::uint32 texture)
{
	m_blit_texture = texture;
}


void DebugRenderer::SetBlitGammaCorrection(bool value)
{
	m_blit_gamma_correction = value;
}


void DebugRenderer::SetBlitLinearizeDepth(bool value)
{
	m_blit_linearize_depth = value;
}


KCL::uint32 DebugRenderer::GetColorTexture() const
{
	return m_color_texture;
}


void GFXB::DebugRenderer::RenderSelectedMeshes(KCL::uint32 command_buffer, KCL::Camera2 *active_camera)
{
	nglBegin(m_debug_job, command_buffer);

	KCL::Matrix4x4 vp;
	KCL::Matrix4x4 mvp;
	KCL::Vector4D color(1.0f, 1.0f, 1.0f, 0.5f);

	const void *p[UNIFORM_MAX];
	memset(p, 0, sizeof(p));

	p[UNIFORM_VP] = vp.v;
	p[UNIFORM_MVP] = mvp.v;
	p[UNIFORM_COLOR0] = color.v;
	p[UNIFORM_COLOR1] = color.v;

	for (size_t i = 0; i < m_selected_meshes.size(); i++)
	{
		GFXB::Mesh3 *mesh3 = (Mesh3*)m_selected_meshes[i]->m_mesh;
		GFXB::Material *material = (GFXB::Material*)m_selected_meshes[i]->m_material;

		vp = active_camera->GetViewProjection();
		mvp = m_selected_meshes[i]->m_world_pom * active_camera->GetViewProjection();

		p[UNIFORM_MODEL] = m_selected_meshes[i]->m_world_pom.v;
		p[UNIFORM_COLOR_TEX] = &material->GetTexture(KCL::Material::COLOR)->m_id;
		p[UNIFORM_ALPHA_TEST_THRESHOLD] = &material->m_alpha_test_threshold;
		p[UNIFORM_BONES] = mesh3->m_nodes.empty() ? nullptr : &mesh3->m_node_matrices[0];

		KCL::uint32 shader_code = m_mesh_shader;

		nglBlendStateAll(m_debug_job, NGL_BLEND_ALFA);
		nglDepthState(m_debug_job, NGL_DEPTH_DISABLED, false);

		bool use_alpha_test = m_selected_meshes[i]->m_material->m_opacity_mode == KCL::Material::ALPHA_TEST;

		if (m_selected_meshes[i]->m_mesh->m_vertex_matrix_indices.size())
		{
			shader_code = use_alpha_test ? m_mesh_skeletal_alpha_test_shader : m_mesh_skeletal_shader;
		}
		else if (use_alpha_test)
		{
			shader_code = m_mesh_alpha_test_shader;
		}

		nglDrawFrontSided(m_debug_job, shader_code, mesh3->m_vbid, mesh3->m_ibid, p);
		//nglDraw(m_debug_job, NGL_LINES, shader_code, 1, &mesh3->m_vbid, mesh3->m_ibid, NGL_FRONT_SIDED, p);
	}

	nglEnd(m_debug_job);
	m_selected_meshes.clear();
}

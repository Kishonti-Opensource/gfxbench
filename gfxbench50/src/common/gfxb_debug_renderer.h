/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_DEBUG_RENDERER_H
#define GFXB_DEBUG_RENDERER_H

#include <vector>
#include <map>
#include <kcl_base.h>
#include <kcl_math3d.h>
#include <ngl.h>
#include "gfxb_particlesystem.h"

namespace GFXB
{
class Shapes;
class Mesh3;
class ComputeEmitter;

class DebugRenderer
{
public:
	static const float SOLID;

	DebugRenderer();
	virtual ~DebugRenderer();

	void Init(KCL::SceneHandler *scene, KCL::uint32 viewport_width, KCL::uint32 viewport_height, Shapes *shapes, KCL::uint32 color_texture, KCL::uint32 depth_texture);
	void UpdateViewport(KCL::uint32 viewport_x, KCL::uint32 viewport_y, KCL::uint32 viewport_width, KCL::uint32 viewport_height);
	void DeleteRenderers();

	void ClearCommands();
	void RenderSelectedMeshes(KCL::uint32 command_buffer, KCL::Camera2 *active_camera);
	void Render(KCL::uint32 command_buffer, const KCL::Camera2 *camera);

	void SetBlitTexture(KCL::uint32 texture);
	void SetBlitGammaCorrection(bool value);
	void SetBlitLinearizeDepth(bool value);
	void RenderOnscreen(KCL::uint32 command_buffer);

	DebugRenderer &DrawLine(const KCL::Vector3D &pos0, const KCL::Vector3D &pos1);
	DebugRenderer &DrawLine(const KCL::Vector3D &pos0, const KCL::Vector3D &color0, const KCL::Vector3D &pos1, const KCL::Vector3D &color1);
	DebugRenderer &DrawSphere(const KCL::Vector3D &center, float radius);
	DebugRenderer &DrawCone(const KCL::Matrix4x4 &model);
	DebugRenderer &DrawAABB(const KCL::Vector3D &center, const KCL::Vector3D &size);
	DebugRenderer &DrawAABB(const KCL::AABB &aabb);
	DebugRenderer &DrawAABB(const KCL::Mesh *mesh);
	DebugRenderer &DrawCube(const KCL::Matrix4x4 &model);
	DebugRenderer &DrawMesh(const KCL::Matrix4x4 &model, KCL::Mesh3 *mesh3);
	DebugRenderer &DrawMesh(const KCL::Vector3D &center, KCL::Mesh3 *mesh3);
	DebugRenderer &DrawMesh(KCL::Mesh *mesh);
	DebugRenderer &DrawCamera(const KCL::Camera2 *camera);

	DebugRenderer &DrawLight(const KCL::Light *light, const KCL::Vector3D &camera_pos);
	DebugRenderer &DrawParticleEmitter(const GFXB::ParticleEmitter *particle_emitter);

	DebugRenderer &SetColor(const KCL::Vector4D &c);
	DebugRenderer &SetColor(const KCL::Vector3D &c);
	DebugRenderer &SetPerimeterThreshold(float threshold);
	DebugRenderer &SetCullMode(NGL_cull_mode cull_mode);
	DebugRenderer &SetBlendMode(NGL_blend_func blend_mode);
	DebugRenderer &SetDepthTestMode(NGL_depth_func depth_test_mode);

	KCL::uint32 GetColorTexture() const;

private:
	enum mesh_types
	{
		SPHERE = 0,
		AABB,
		CUBE,
		CYLINDER,
		CONE,
		MESH_SHAPE,
		LINE
	};

	struct draw_command
	{
		mesh_types m_mesh_type;

		KCL::Matrix4x4 m_model;

		KCL::Vector4D m_position0;
		KCL::Vector4D m_position1;
		KCL::Vector4D m_scale;

		KCL::Vector4D m_color0;
		KCL::Vector4D m_color1;
		float m_perimeter_threshold;

		Mesh3 *m_mesh3;

		NGL_cull_mode m_cull_mode;
		NGL_blend_func m_blend_mode;
		NGL_depth_func m_depth_test_mode;

		draw_command()
		{
			clear();
		}

		void clear()
		{
			m_mesh_type = SPHERE;

			m_model.identity();
			m_position0.set(0.0f, 0.0f, 0.0f, 1.0f);
			m_position1.set(0.0f, 0.0f, 0.0f, 1.0f);
			m_scale.set(1.0f, 1.0f, 1.0f, 1.0f);
			m_color0.set(1.0f, 1.0f, 1.0f, 1.0f);
			m_color1.set(1.0f, 1.0f, 1.0f, 1.0f);

			m_perimeter_threshold = 10.0f;

			m_cull_mode = NGL_FRONT_SIDED;
			m_blend_mode = NGL_BLEND_DISABLED;
			m_depth_test_mode = NGL_DEPTH_DISABLED;
		}
	};

	draw_command m_state;

	std::vector<draw_command> m_commands;
	std::vector<KCL::Mesh*> m_selected_meshes;

	KCL::uint32 m_width;
	KCL::uint32 m_height;
	KCL::uint32 m_color_texture;
	KCL::uint32 m_depth_texture;

	KCL::uint32 m_debug_job;

	Shapes *m_shapes;
	Mesh3 *m_dir_light_shape;

	KCL::uint32 m_sphere_shader;
	KCL::uint32 m_cube_shader;
	KCL::uint32 m_cone_shader;
	KCL::uint32 m_line_shader;

	bool m_blit_gamma_correction;
	bool m_blit_linearize_depth;
	KCL::uint32 m_blit_texture;
	KCL::uint32 m_blit_shader;
	KCL::uint32 m_blit_render;

	KCL::uint32 m_mesh_shader;
	KCL::uint32 m_mesh_alpha_test_shader;
	KCL::uint32 m_mesh_skeletal_shader;
	KCL::uint32 m_mesh_skeletal_alpha_test_shader;

	void DrawDirectionalLight(const KCL::Matrix4x4 &model);
	void Commit();
};
}

#endif  // GFXB_DEBUG_RENDERER_H

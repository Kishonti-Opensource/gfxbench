/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB5_SUN_LIGHT_H
#define GFXB5_SUN_LIGHT_H

#include "common/gfxb_scene_base.h"
#include "common/gfxb_factories.h"
#include "common/gfxb_shader.h"

namespace GFXB
{
	class FrustumCull;
	class MeshFilter;
	class ShadowMap;

	class SunLight
	{
	public:
		SunLight(KCL::SceneHandler *scene, MeshFilter *shadow_mesh_filter);
		~SunLight();

		void Update();
		void ShadowRender(KCL::uint32 command_buffer);

		ShadowMap *GetShadowMap();

		KCL::Light *m_light;
		KCL::Vector3D m_light_dir;

	private:
		KCL::SceneHandler *m_scene;
		ShadowMap *m_shadow_map;

		KCL::Light *FindSunLight();
	};
}

#endif
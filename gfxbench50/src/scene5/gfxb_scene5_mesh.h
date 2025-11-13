/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_SCENE5_MESH_H
#define GFXB_SCENE5_MESH_H

#include "common/gfxb_mesh.h"

namespace KCL
{
	class Camera2;
}

namespace GFXB
{
	class Scene5Mesh : public Mesh
	{
	public:
		Scene5Mesh(const std::string &name, KCL::Node *parent, KCL::Object *owner);

		void Animate(KCL::uint32 animation_time, const KCL::Camera2 *camera);

		// Cascaded shadow
		KCL::uint32 m_csm_frame_counter;

		// Animated emissive intensity
		float m_emissive_intensity;

		// Fire
		float m_fire_time;
		KCL::uint32 m_fire_time_offset;

		// Dither value: negative: not dithered , 0...1, dithered, 1... invisible
		float m_dither_value;
	};

	class Scene5MeshFactory : public KCL::MeshFactory
	{
	public:
		virtual KCL::Mesh *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner) override;
	};
}

#endif

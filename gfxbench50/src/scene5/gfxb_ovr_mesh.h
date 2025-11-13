/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef GFXB_OVR_MESH_H
#define GFXB_OVR_MESH_H

#include "common/gfxb_mesh.h"

namespace GFXB
{
	class OvrMesh : public Mesh
	{
	public:
		OvrMesh(const std::string &name, KCL::Node *parent, KCL::Object *owner);

		void Animate(KCL::uint32 animation_time);

		//is part of left eye sky
		int m_is_left_eye_sky;
		int m_is_right_eye_sky;
	};

	class OvrMeshFactory : public KCL::MeshFactory
	{
	public:
		virtual KCL::Mesh *Create(const std::string &name, KCL::Node *parent, KCL::Object *owner) override;
	};
}

#endif

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_mesh.h"
#include "gfxb_frustum_cull.h"

using namespace GFXB;

Mesh::Mesh(const std::string &name, KCL::Node *parent, KCL::Object *owner) : KCL::Mesh(name, parent, owner)
{
	m_cull_frames.resize(FrustumCull::MAX_INSTANCES);

	m_debug_frustum_ids.resize(FrustumCull::MAX_INSTANCES);
}


void Mesh::CalculateCenter()
{
	KCL::Vector3D half_extent;
	m_aabb.CalculateHalfExtentCenter(half_extent, m_center);
}

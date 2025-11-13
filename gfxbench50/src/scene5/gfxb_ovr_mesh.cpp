/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "gfxb_ovr_mesh.h"
#include "common/gfxb_light.h"

using namespace GFXB;

OvrMesh::OvrMesh(const std::string &name, KCL::Node *parent, KCL::Object *owner) : Mesh(name, parent, owner)
{
	m_is_left_eye_sky = name.find( "skyL" ) != std::string::npos;
	m_is_right_eye_sky = name.find( "skyR" ) != std::string::npos;
}


void OvrMesh::Animate(KCL::uint32 animation_time)
{
	
}


KCL::Mesh *OvrMeshFactory::Create(const std::string &name, KCL::Node *parent, KCL::Object *owner)
{
	return new OvrMesh(name, parent, owner);
}
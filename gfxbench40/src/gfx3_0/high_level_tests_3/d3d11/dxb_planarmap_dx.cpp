/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "dxb_planarmap.h"

#include "platform.h"
#include "d3d11/shader.h"

#include <string>

using namespace DXB;
using namespace std;

PlanarMap::PlanarMap(int w, int h, const char* name) : KCL::PlanarMap(w, h, name)
{
	m_FBO = new GLB::FBO;

	m_FBO->init( w, h, GLB::RGB565_Linear, GLB::DEPTH_16_RB);
}


PlanarMap::~PlanarMap()
{
	delete m_FBO;
}


void PlanarMap::Bind()
{
	GLB::FBO::bind( m_FBO);
}
	
void PlanarMap::Unbind()
{
	GLB::FBO::discardDepth(m_FBO);
	GLB::FBO::bind( 0);
}

void PlanarMap::Clear()
{
	GLB::FBO::discard( m_FBO );
}

KCL::PlanarMap* KCL::PlanarMap::Create( int w, int h, const char *name)
{
	return new DXB::PlanarMap( w, h, name);
}

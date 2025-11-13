/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_scene.h"
#include "opengl/shader.h"
#include "krl_material.h"

GLB_Scene_ES2::GLB_Scene_ES2() : m_particles_vbo(0), m_main_fbo( 0), m_mblur_fbo( 0), m_fullscreen_quad_vbo( 0)
{
#ifdef DUMMY_FBO_FOR_PLANAR_FLUSH
	m_dummyFbo = 0;
	m_dummy_texture_unif_loc = -1;
	m_dummy_program = 0;
	m_dummy_vbo = 0;
	m_dummy_ebo = 0;
#endif
}


GLB_Scene_ES2::~GLB_Scene_ES2()
{
	Release_GLResources();
}

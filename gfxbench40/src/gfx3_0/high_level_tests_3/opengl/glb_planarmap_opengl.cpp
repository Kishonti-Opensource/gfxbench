/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "glb_planarmap.h"

#include "platform.h"
#include "opengl/shader.h"
#include "opengl/misc2_opengl.h"
#include "opengl/fbo.h"

#include <string>

using namespace GLB;
using namespace std;

PlanarMap::PlanarMap(int w, int h, const char *name) : KCL::PlanarMap(w, h, name)
{
	glGenFramebuffers( 1, &m_fboid );

	glGenRenderbuffers(1, &m_rboid);

	glBindRenderbuffer(GL_RENDERBUFFER, m_rboid);
	glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);


	glGenTextures(1, &m_tboid);

	glBindTexture(GL_TEXTURE_2D, m_tboid);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);


	Bind();

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tboid, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rboid);

	Unbind();
}


PlanarMap::~PlanarMap()
{
	glDeleteTextures(1, &m_tboid);
	glDeleteFramebuffers(1, &m_fboid);
	glDeleteRenderbuffers(1, &m_rboid);
}


void PlanarMap::Bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, m_fboid);
}
	
void PlanarMap::Unbind()
{
	DiscardDepthAttachment();

	glBindFramebuffer(GL_FRAMEBUFFER, FBO::GetGlobalFBO()->getName() );
}


KCL::PlanarMap* KCL::PlanarMap::Create( int w, int h, const char *name)
{
	return new GLB::PlanarMap( w, h, name);
}

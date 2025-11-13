/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#ifndef MISC2_OPENGL_H
#define MISC2_OPENGL_H

#include "platform.h"
#include <stdio.h>
#include "render_statistics_defines.h"


class MemInfo
{
public:
	MemInfo();
	void Get( GLenum nam);
private:
	int m_res[2][4];
	int m_used_memory;
	int m_used_aux_memory;
	bool m_c;
};

#define CheckOutOfGLMemory()	CheckOutOfGLMemory2(__FILE__, __LINE__)

bool CheckOutOfGLMemory2(const char* file, int line);

void saveBackBuf( int w, int h);

void glBufferData_chunked( KCL::uint32 vboType,KCL::uint32 size,void *data ,KCL::uint32 usage, KCL::uint32 extraspace=0);

void saveFBO(GLint fboID,int isblit=0, int sizeX = 0, int sizeY = 0, int frame = 0);

void GetGLVersion(int & major, int & minor, bool & core, bool & es);

void CheckGLErrors(const char * tag = NULL);

const char *GetGLErrorString(GLenum error_enum);

bool CheckPBBExtCompatibility();

#if defined OCCLUSION_QUERY_BASED_STAT


class GLSamplesPassedQuery
{
public:

	GLSamplesPassedQuery();
	~GLSamplesPassedQuery();

	void Begin();
	void End();
	size_t Result();

private:
	GLSamplesPassedQuery(const GLSamplesPassedQuery&);
	GLSamplesPassedQuery& operator=(const GLSamplesPassedQuery&);

	unsigned int m_qid;

	size_t m_cached_result;
	bool m_dirty;
};
#endif

#endif //MISC2_OPENGL_H

/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "misc2.h"
#include "misc2_opengl.h"
#include "texture.h"
#include "ng/log.h"

#include <string>
#include <algorithm>
#include <string.h>
#include <ctype.h>

#include "kcl_image.h"
#include "kcl_os.h"
#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define NOCOMM
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else

#ifndef BI_RGB
#define BI_RGB        0L

typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef long 				LONG;

struct BITMAPINFOHEADER{
	DWORD      biSize;
	LONG       biWidth;
	LONG       biHeight;
	WORD       biPlanes;
	WORD       biBitCount;
	DWORD      biCompression;
	DWORD      biSizeImage;
	LONG       biXPelsPerMeter;
	LONG       biYPelsPerMeter;
	DWORD      biClrUsed;
	DWORD      biClrImportant;
} __attribute__((packed));

struct BITMAPFILEHEADER {
	WORD    bfType;
	DWORD   bfSize;
	WORD    bfReserved1;
	WORD    bfReserved2;
	DWORD   bfOffBits;
} __attribute__((packed));


#endif
#endif

#ifdef ANDROID
#include <sys/system_properties.h>
#endif

using namespace KCL;
using namespace GLB;

//static int s_max_vertex_attrib = 7;unused vari


MemInfo::MemInfo()
{
	m_c = false;
}


void MemInfo::Get( GLenum nam)
{
	glGetIntegerv( nam, m_res[m_c]);
	if( m_c)
	{
		m_used_memory = m_res[0][0] - m_res[1][0];
		m_used_aux_memory = m_res[0][2] - m_res[1][2];
		INFO(" Used memory: %d %d\n", m_used_memory,m_used_aux_memory);
	}
	m_c = ! m_c;
}


bool CheckOutOfGLMemory2(const char* file, int line)
{
	GLenum error;

	while( (error = glGetError()) != GL_NO_ERROR )
	{
		if( error == GL_OUT_OF_MEMORY)
		{
			return true;
		}

		INFO("glGetError at %s %d: 0x%x", file, line, error);
	}
	return false;
}


void saveBackBuf( int w, int h)
{
	static int cnt = 0;
	static char name[256];

	sprintf(name, "video%05d.tga", cnt++);

	uint8 *outbuf = new uint8[w * h * 3];

	glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, outbuf);

	KCL::Image::saveTga( name, w, h, outbuf, KCL::Image_RGB888);

	delete [] outbuf;
}


#define VBO_CHUNK_SIZE (512*1024)
void glBufferData_chunked (unsigned int vboType, unsigned int size, void *data,unsigned int usage, unsigned int extraspace)
{
	unsigned int chunk_offset = 0;
	char *byte_vertexdata=(char*)data;
	unsigned int num_chunks = size / VBO_CHUNK_SIZE;
	unsigned int remainder =  size - (num_chunks*VBO_CHUNK_SIZE);

	glBufferData (vboType, size+extraspace, 0, usage);
	if(data != 0 )
	{
		for( unsigned int i=0;i<num_chunks;i++)
		{
			glBufferSubData(vboType, chunk_offset, VBO_CHUNK_SIZE, &byte_vertexdata[chunk_offset]);
			chunk_offset += VBO_CHUNK_SIZE;
		}
		if (remainder)
		{
			glBufferSubData(vboType, chunk_offset, remainder, &byte_vertexdata[chunk_offset]);
		}
	}
}

void CheckGLErrors(const char * tag)
{
	GLenum error = glGetError();
	while (error != GL_NO_ERROR)
	{	
		if (tag)
		{		
			NGLOG_ERROR("%s: GL Error: %s - %s", tag, error, GetGLErrorString(error));
		}
		error = glGetError();
	}
}

const char * GetGLErrorString(GLenum error_enum)
{
	switch(error_enum)
	{
	case GL_NO_ERROR:
		return "GL_NO_ERROR";

	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";		

	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";	

	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";

	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";		

	default:
		return "UNKNOWN ERROR CODE";
	}
}

#ifdef FBO_CAPTURE


#ifndef IPHONE
void saveFBO(GLint fboID, int isblit, int sizeX, int sizeY, int frame)
{
	if(isblit==0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER,fboID);
		unsigned char* temp=new unsigned char[sizeX*sizeY*4];
		memset(temp,0xCD,sizeX*sizeY*4);

		glPixelStorei(GL_PACK_ALIGNMENT,1);
		glReadPixels(0,0,sizeX,sizeY,GL_RGBA,GL_UNSIGNED_BYTE,temp);

		convertRGBAtoBGR( temp, sizeX*sizeY);


		char name[256];

		sprintf(name, "%sfbo%05d_%d.tga", GLB::g_os->GetDataDirectory(), frame, fboID);
		saveTga(name, sizeX, sizeY, temp, KCL::Image_RGB888);
		delete []temp;
	}
	else
	{
		uint32 nonFsaaFramebuffer = 0;
		uint32 nonFsaaRenderbuffer = 0;
		uint32 depth = 0;
		if(!nonFsaaFramebuffer)
		{
			glGenFramebuffers(1,&nonFsaaFramebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER,nonFsaaFramebuffer);

			glGenRenderbuffers(1,&nonFsaaRenderbuffer);
			glBindRenderbuffer(GL_RENDERBUFFER,nonFsaaRenderbuffer);

			glRenderbufferStorage(GL_RENDERBUFFER,  GL_RGB, sizeX, sizeY);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, nonFsaaRenderbuffer);

			glGenRenderbuffers (1, &depth);
			glBindRenderbuffer (GL_RENDERBUFFER, depth);

			glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, sizeX, sizeY);
			glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
		}

		glBindFramebuffer(GL_READ_FRAMEBUFFER,fboID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER,nonFsaaFramebuffer);
		glBlitFramebuffer(0, 0, sizeX, sizeY, 0, 0, sizeX,sizeY,GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER,nonFsaaFramebuffer);
		unsigned char* temp=new unsigned char[sizeX*sizeY*3];
		memset(temp,0,sizeX*sizeY*3);

		//glReadPixels(0,0,sizeX,sizeY,GL_BGR,GL_UNSIGNED_BYTE,temp);
		glReadPixels(0,0,sizeX,sizeY,GL_RGB,GL_UNSIGNED_BYTE,temp);

		glBindFramebuffer(GL_FRAMEBUFFER,fboID);

		char name[256];
		static int texID=0;
		sprintf(name, "tex%05d.tga", texID++);
		INFO("nonfsasaveTGA%d",name);
		//saveBufFromBGRsrc(name, sizeX, sizeY, 3, temp);
		//saveBuf2(name, sizeX, sizeY, 3, temp);
		delete []temp;

		glDeleteFramebuffers(1,&nonFsaaFramebuffer);
		glDeleteRenderbuffers(1,&nonFsaaRenderbuffer);
		glDeleteRenderbuffers(1,&depth);
	}
}
#endif//IPHONE


#endif//FBO_CAPTURE

#if defined OCCLUSION_QUERY_BASED_STAT
GLSamplesPassedQuery::GLSamplesPassedQuery() : m_qid(0), m_cached_result(0), m_dirty(true)
{
	glGenQueries(1, &m_qid);
}


GLSamplesPassedQuery::~GLSamplesPassedQuery()
{
	glDeleteQueries(1, &m_qid);
}


void GLSamplesPassedQuery::Begin()
{
#if defined WIN32 && defined __GLEW_H__
	glBeginQuery(GL_SAMPLES_PASSED, m_qid);
	m_dirty = true;
#endif
}


void GLSamplesPassedQuery::End()
{
#if defined WIN32 && defined __GLEW_H__
	glEndQuery(GL_SAMPLES_PASSED);
#endif
}


size_t GLSamplesPassedQuery::Result()
{
#if defined WIN32 && defined __GLEW_H__
	if(m_dirty)
	{
		{
			int available = GL_FALSE;
			while (!available)
			{
				glGetQueryObjectiv(m_qid, GL_QUERY_RESULT_AVAILABLE, &available);
			}
		}

		m_cached_result = 0;
		glGetQueryObjectuiv(m_qid, GL_QUERY_RESULT, &m_cached_result);
		m_dirty = false;
	}
#endif
	return m_cached_result;
}
#endif

void GetGLVersion(int & major, int & minor, bool & core, bool & es)
{
	major = 0;
	minor = 0;
	core = false; // TODO
	es = false;

	const char * str = (const char*) glGetString(GL_VERSION);
	if (str == NULL)
	{
		NGLOG_ERROR("GetGLVersion: Version string is NULL!");
		return;
	}
	
	unsigned int len = strlen(str);
	if (len > 2) // We need at least 3 characters
	{			
		std::string versionStr(str); // Skip the first char, so it can not be a '.'
		std::transform(versionStr.begin(), versionStr.end(), versionStr.begin(), ::tolower);
		char * pch = strtok(const_cast<char*>(versionStr.c_str()), " ");
		while (pch != NULL)
		{	
			if (!strcmp(pch, "es"))
			{
				es = true;					
			} 
			else if (major < 1) // Check if we already found the version
			{
				unsigned int len = strlen(pch);
				if (len >= 3)
				{						
					if (pch[1] == '.' && isdigit(pch[0]) && isdigit(pch[2]))
					{
						major = pch[0] - '0';
						minor = pch[2] - '0';
					}
				}
			}
			pch = strtok (NULL, " ");
		}
	}	
	else
	{
		NGLOG_ERROR("GetGLVersion: Version string is invalid:", str);
	}
}


static bool ContainsString(const char *src, const char *find)
{
	if (!src)
	{
		return false;
	}

	std::string str(src);

	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	return str.find(find) != std::string::npos;
}


bool CheckPBBExtCompatibility()
{
#ifdef ANDROID
	const char *gl_vendor = (const char*)glGetString(GL_VENDOR);
	const char *gl_renderer = (const char*)glGetString(GL_RENDERER);

	bool is_qcom = ContainsString(gl_vendor, "qualcomm");
	bool is_adreno = ContainsString(gl_renderer, "adreno");

	if (!is_qcom && !is_adreno) 
	{
		return true;
	}

	FILE* pipe = popen("getprop ro.build.version.release", "r");
	if (!pipe)
	{
		return true;
	}
	char buffer[1024];

	int version;
	while(!feof(pipe))
	{
		if(fgets(buffer, 1024, pipe) != NULL)
		{
			sscanf(buffer, "%d", &version);
			break;
		}
	}
	pclose(pipe);

	return version > 5;
#else
	return true;
#endif
}
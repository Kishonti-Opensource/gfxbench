/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "offscrman.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>

#include "fbo.h"
#include "opengl/glbshader.h"
#include <kcl_image.h>
#include <kcl_os.h>
#include "misc2.h"
#include "test_descriptor.h"
#include "opengl/ext.h"

#include "kcl_base.h"

#include "opengl/glb_opengl_state_manager.h"

#define SAFE_DELETE(x)	if (NULL != (x)) { delete (x); (x) = NULL; }


using namespace GLB;
using namespace KCL;

OffscreenManager* OffscreenManager::Create(const GlobalTestEnvironment* const gte, int w, int h)
{
	if( GLB::g_extension->hasFeature( GLB::GLBFEATURE_vertex_array_object))
	{
		return new OffscreenManager_CoreProfileVAO( w, h);
	}
	else
	{
		return new OffscreenManager( w, h);
	}
}


OffscreenManager::OffscreenManager( int w, int h) : m_offscreen_default_viewport_width( w), m_offscreen_default_viewport_height( h)
{
	srand( time(0));

	memset(m_sample_times, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);

	m_width = 0;
	m_height = 0;
	m_globalFBO = 0;

    m_virtual_resolution = false;

    m_onscreen_width = 0;
    m_onscreen_height = 0;
	
	m_offscreen_fbo[0] = 0;
	m_offscreen_fbo[1] = 0;
	m_scratch_fbo[0] = 0;
	m_scratch_fbo[1] = 0;
	m_current_fbo_index = 0;
	m_mosaic_fbo = 0;

	m_onscr_mosaic_fbo = 0;
	m_onscr_mosaic_idx = 0;
	m_onscr_mosaic_program[0] = 0;
	m_onscr_mosaic_program[1] = 0;
	m_onscr_mosaic_text_unif_loc[0] = 0;
	m_onscr_mosaic_text_unif_loc[1] = 0;
	init_onscr_mosaic_coords();

	m_offscreen_vertex_shader = 0;
	m_offscreen_fragment_shader = 0;
	m_offscreen_program = 0;
	m_offscreen_vbo = 0;
	m_offscreen_ebo = 0;
	m_offscr_text_unif_loc = 0;
	
	m_next_slot_time_interval = 0;
	m_next_slot_previous_time = 0;
	m_last_refresh_msec = 0;
	m_mosaic_idx = 0;
}


void OffscreenManager::init_onscr_mosaic_coords()
{
	for(KCL::uint32 i=0; i<ONSCR_SAMPLE_C; ++i)
	{
		m_onscr_mosaic_coords_x[i] = (i % ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_W;
		m_onscr_mosaic_coords_y[i] = (i / ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_H;
	}
}


OffscreenManager::~OffscreenManager()
{
	Clear();
}


OffscreenManager_CoreProfileVAO::OffscreenManager_CoreProfileVAO(int w, int h) : OffscreenManager(w, h) {}

//TODO move common code with parent to fn
OffscreenManager_CoreProfileVAO::~OffscreenManager_CoreProfileVAO()
{
#if defined HAVE_GLES3 || defined __glew_h__
    glDeleteVertexArrays(COUNT_OF(m_VAOs), m_VAOs); 
#endif
}

void OffscreenManager_CoreProfileVAO::createOffscrProg()
{
#if defined HAVE_GLES3 || defined __glew_h__
    if(m_offscreen_program)
	{
		return;
	}

    std::string srcVert;
    std::string srcVert2;
    std::string srcFrag;

	if (g_extension->getGraphicsContext()->type() == GraphicsContext::GLES)
	{
		srcVert = "#version 300 es\n";
		srcVert2 = "#version 300 es\n";
		srcFrag = "#version 300 es\n";
	}
	else
	{
		srcVert = "#version 400 core\n";
		srcVert2 = "#version 400 core\n";
		srcFrag = "#version 400 core\n";
	}

	srcVert +=
		"\
		#ifdef GL_ES\n\
		precision highp float;\n\
		#endif\n\
		in vec2 myVertex;\n\
		in vec2 myTexCoord;\n\
		out vec2 vTexCoord;\n\
		void main()\n\
		{\n\
		gl_Position = vec4(myVertex.x, myVertex.y, 0.0, 1.0);\n\
		vTexCoord = myTexCoord;\n\
		}\n\
		";

	srcVert2 +=
		"\
		#ifdef GL_ES\n\
		precision highp float;\n\
		#endif\n\
		in vec2 myVertex;\n\
		in vec2 myTexCoord;\n\
		out vec2 vTexCoord;\n\
		void main()\n\
		{\n\
			gl_Position = vec4(myVertex.x, myVertex.y, 0.0, 1.0);\n\
		vTexCoord = vec2(myTexCoord.y, 1.0-myTexCoord.x);\n\
		}\n\
		";

	srcFrag +=
		"\
		#ifdef GL_ES\n\
		precision mediump float;\n\
		#endif\n\
		uniform sampler2D texChars;\n\
		in  vec2 vTexCoord;\n\
        out vec4 frag_color;\n\
		void main()\n\
		{\n\
			frag_color = texture(texChars, vTexCoord);\n\
		}\n\
		";

    InitCommon(srcVert.c_str(), srcVert2.c_str(), srcFrag.c_str());

    // create the VAOs
    glGenVertexArrays(COUNT_OF(m_VAOs), m_VAOs);
    
    //record the VAOs
    for(int i=0;i<COUNT_OF(m_VAOs); ++i)
    {
        size_t offset = (i + 1) * 8 * sizeof(float); //first 8 float is vert pos, then 64 + 2 sets of UVs

        glBindVertexArray(m_VAOs[i]);

        glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
            
            glEnableVertexAttribArray(0); //position
		    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
        
            glEnableVertexAttribArray(1); //texcoord
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const void*)offset);

        glBindVertexArray(0);
	    glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
#endif
}

void OffscreenManager::createOffscrProg()
{
    const char *srcVert =
		"\
		#ifdef GL_ES\n\
		precision highp float;\n\
		#endif\n\
		attribute  vec2 myVertex;\n\
		attribute  vec2 myTexCoord;\n\
		varying  vec2 vTexCoord;\n\
		void main()\n\
		{\n\
		gl_Position = vec4(myVertex.x, myVertex.y, 0.0, 1.0);\n\
		vTexCoord = myTexCoord;\n\
		}\n\
		";

	const char *srcVert2 =
		"\
		#ifdef GL_ES\n\
		precision highp float;\n\
		#endif\n\
		attribute  vec2 myVertex;\n\
		attribute  vec2 myTexCoord;\n\
		varying  vec2 vTexCoord;\n\
		void main()\n\
		{\n\
		gl_Position = vec4(myVertex.x, myVertex.y, 0.0, 1.0);\n\
		vTexCoord = vec2(myTexCoord.y, 1.0-myTexCoord.x);\n\
		}\n\
		";

	const char *srcFrag =
		"\
		#ifdef GL_ES\n\
		precision mediump float;\n\
		#endif\n\
		uniform sampler2D texChars;\n\
		varying  vec2 vTexCoord;\n\
		void main()\n\
		{\n\
		gl_FragColor = texture2D(texChars, vTexCoord);\n\
		}\n\
		";

    InitCommon(srcVert, srcVert2, srcFrag);
}

void OffscreenManager::InitCommon(const char *srcVert, const char *srcVert2, const char *srcFrag)
{
	if(m_offscreen_program)
	{
		return;
	}

	m_offscreen_vertex_shader = GLB::initShader(GL_VERTEX_SHADER, srcVert);
	m_offscreen_fragment_shader = GLB::initShader(GL_FRAGMENT_SHADER, srcFrag);

	m_offscreen_program = glCreateProgram();

	glAttachShader(m_offscreen_program, m_offscreen_vertex_shader);
	glAttachShader(m_offscreen_program, m_offscreen_fragment_shader);

	glBindAttribLocation(m_offscreen_program, 0, "myVertex");
	glBindAttribLocation(m_offscreen_program, 1, "myTexCoord");

	glLinkProgram(m_offscreen_program);

	m_offscr_text_unif_loc = glGetUniformLocation( m_offscreen_program, "texChars");

	glsl_log(m_offscreen_program, GL_LINK_STATUS, "link");
	
	
	// ONSCREEN MOSAIC PROGRAM CREATION:
	m_onscr_mosaic_program[0] = glCreateProgram();

	glAttachShader(m_onscr_mosaic_program[0], m_offscreen_vertex_shader);
	glAttachShader(m_onscr_mosaic_program[0], m_offscreen_fragment_shader);

	glBindAttribLocation(m_onscr_mosaic_program[0], 0, "myVertex");
	glBindAttribLocation(m_onscr_mosaic_program[0], 1, "myTexCoord");

	glLinkProgram(m_onscr_mosaic_program[0]);

	m_onscr_mosaic_text_unif_loc[0] = glGetUniformLocation( m_onscr_mosaic_program[0], "texChars");

	glsl_log(m_onscr_mosaic_program[0], GL_LINK_STATUS, "link");


	KCL::uint32 vertex_shader2 = GLB::initShader(GL_VERTEX_SHADER, srcVert2);
	m_onscr_mosaic_program[1] = glCreateProgram();

	glAttachShader(m_onscr_mosaic_program[1], vertex_shader2);
	glAttachShader(m_onscr_mosaic_program[1], m_offscreen_fragment_shader);

	glBindAttribLocation(m_onscr_mosaic_program[1], 0, "myVertex");
	glBindAttribLocation(m_onscr_mosaic_program[1], 1, "myTexCoord");

	glLinkProgram(m_onscr_mosaic_program[1]);

	m_onscr_mosaic_text_unif_loc[1] = glGetUniformLocation( m_onscr_mosaic_program[0], "texChars");

	glsl_log(m_onscr_mosaic_program[1], GL_LINK_STATUS, "link");

	
	
	if(m_offscreen_vertex_shader)
	{
		glDeleteShader(m_offscreen_vertex_shader);
		m_offscreen_vertex_shader = 0;
	}
	
	if(vertex_shader2)
	{
		glDeleteShader(vertex_shader2);
	}

	if(m_offscreen_fragment_shader)
	{
		glDeleteShader(m_offscreen_fragment_shader);
		m_offscreen_fragment_shader = 0;
	}

	glGenBuffers(1, &m_offscreen_vbo);
	glGenBuffers(1, &m_offscreen_ebo);

	float* vertices_texcoords = new float[24+8*SAMPLE_C];
	vertices_texcoords[0] = -1.0f; //0
	vertices_texcoords[1] = -1.0f; //0

	vertices_texcoords[2] =  1.0f; //1
	vertices_texcoords[3] = -1.0f; //1

	vertices_texcoords[4] =  1.0f; //2
	vertices_texcoords[5] =  1.0f; //2

	vertices_texcoords[6] = -1.0f; //3
	vertices_texcoords[7] =  1.0f; //3

	float X_L, Y_D, X_R, Y_U;
	for(KCL::uint32 i=1; i<=SAMPLE_C; ++i)
	{
		calcSampleTexCoords(i-1, X_L, Y_D, X_R, Y_U);

		vertices_texcoords[i*8    ] = X_L;
		vertices_texcoords[i*8 + 1] = Y_D;

		vertices_texcoords[i*8 + 2] = X_R;
		vertices_texcoords[i*8 + 3] = Y_D;
		
		vertices_texcoords[i*8 + 4] = X_R;
		vertices_texcoords[i*8 + 5] = Y_U;
		
		vertices_texcoords[i*8 + 6] = X_L;
		vertices_texcoords[i*8 + 7] = Y_U;
		
		KCL::uint32 idx = i-1;
		m_mosaic_coords_x[idx] = (idx % SAMPLE_NUM_X) * SAMPLE_W;
		m_mosaic_coords_y[idx] = (idx / SAMPLE_NUM_X) * SAMPLE_H;
	}

	X_L = 0;
	Y_D = 0;
	X_R = X_L + (float)SAMPLE_W / (float)SCRATCH_WIDTH;
	Y_U = Y_D + (float)SAMPLE_H / (float)SCRATCH_HEIGHT;

	//texture coordinate for scratch
	vertices_texcoords[8+8*SAMPLE_C    ] = X_L;
	vertices_texcoords[8+8*SAMPLE_C + 1] = Y_D;

	vertices_texcoords[8+8*SAMPLE_C + 2] = X_R;
	vertices_texcoords[8+8*SAMPLE_C + 3] = Y_D;

	vertices_texcoords[8+8*SAMPLE_C + 4] = X_R;
	vertices_texcoords[8+8*SAMPLE_C + 5] = Y_U;

	vertices_texcoords[8+8*SAMPLE_C + 6] = X_L;
	vertices_texcoords[8+8*SAMPLE_C + 7] = Y_U;


	//texture coordinate for full view
	vertices_texcoords[16+8*SAMPLE_C    ] =  0.0f; //0
	vertices_texcoords[16+8*SAMPLE_C + 1] =  0.0f; //0

	vertices_texcoords[16+8*SAMPLE_C + 2] =  1.0f; //1
	vertices_texcoords[16+8*SAMPLE_C + 3] =  0.0f; //1

	vertices_texcoords[16+8*SAMPLE_C + 4] =  1.0f; //2
	vertices_texcoords[16+8*SAMPLE_C + 5] =  1.0f; //2

	vertices_texcoords[16+8*SAMPLE_C + 6] =  0.0f; //3
	vertices_texcoords[16+8*SAMPLE_C + 7] =  1.0f; //3


	glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (24+8*SAMPLE_C), (const void*)vertices_texcoords, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	delete[] vertices_texcoords;

	KCL::uint8 indicesTmp[] =
	{
		0, 1, 2,
		0, 2, 3
	};
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(KCL::uint8) * 6, (const void*)indicesTmp, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	m_globalFBO = FBO::GetGlobalFBO();
}	 


void OffscreenManager::deleteOffscrProg()
{
	if(m_offscreen_program == 0)
	{
		return;
	}
	glDeleteProgram(m_offscreen_program);
	m_offscreen_program = 0;

	if(m_onscr_mosaic_program[0])
	{
		glDeleteProgram(m_onscr_mosaic_program[0]);
		m_onscr_mosaic_program[0] = 0;
	}
	if(m_onscr_mosaic_program[1])
	{
		glDeleteProgram(m_onscr_mosaic_program[1]);
		m_onscr_mosaic_program[1] = 0;
	}

	glDeleteBuffers(1, &m_offscreen_vbo);
	glDeleteBuffers(1, &m_offscreen_ebo);
	
	SAFE_DELETE(m_offscreen_fbo[0]);
	SAFE_DELETE(m_offscreen_fbo[1]);
	SAFE_DELETE(m_scratch_fbo[0]);
	SAFE_DELETE(m_scratch_fbo[1]);
	SAFE_DELETE(m_mosaic_fbo);
	SAFE_DELETE(m_onscr_mosaic_fbo);

	m_onscr_mosaic_idx = 0;
}

void OffscreenManager_CoreProfileVAO::renderRectWithTexture(KCL::uint32 textureId, TextureCoordinatesMode textureCoordinatesMode) const
{
#if defined HAVE_GLES3 || defined __glew_h__
    glBindTexture(GL_TEXTURE_2D, textureId);
	
    int VAOidx = 0;
	switch ( textureCoordinatesMode)
	{
	case USE_MOSAIC:
		VAOidx = m_mosaic_idx;
		break;
	case USE_SCRATCH:
		VAOidx = SAMPLE_C;
		break;
	case USE_FULL:
		VAOidx = SAMPLE_C + 1;
		break;
	case USE_FULL_NEAREST:
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			VAOidx = SAMPLE_C + 1;
			break;
		}
	}

    glBindVertexArray(m_VAOs[VAOidx]);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

    glBindVertexArray(0);
#endif
}

void OffscreenManager::renderRectWithTexture(KCL::uint32 textureId, TextureCoordinatesMode textureCoordinatesMode) const
{
	glBindTexture(GL_TEXTURE_2D, textureId);
	
	size_t offset = 0;

	switch ( textureCoordinatesMode)
	{
	case USE_MOSAIC:
		offset = (m_mosaic_idx + 1) * 8 * sizeof(float);
		break;
	case USE_SCRATCH:
		offset = (8 * SAMPLE_C + 8) * sizeof(float);
		break;
	case USE_FULL:
		offset = (8 * SAMPLE_C + 16) * sizeof(float);
		break;
	case USE_FULL_NEAREST:
		{
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			offset = (8 * SAMPLE_C + 16) * sizeof(float);
			break;
		}
	}

	glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, (const void*)offset);
	
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
}


void OffscreenManager::calcSampleTexCoords(size_t idx, float &X_L, float &Y_D, float &X_R, float &Y_U)
{
	KCL::uint32 xui = rand() % (m_width - SAMPLE_W);
	KCL::uint32 yui = rand() % (m_height - SAMPLE_H);

	X_L = (float)xui / (float)m_width;
	Y_D = (float)yui / (float)m_height;
	X_R = X_L + (float)SAMPLE_W / m_width;
	Y_U = Y_D + (float)SAMPLE_H / m_height;

	m_sample_coords_x[idx] = xui;
	m_sample_coords_y[idx] = yui;
}


void OffscreenManager::clear_saved_sample_data()
{
	memset(m_sample_times, 0, sizeof(KCL::uint32)*SAMPLE_C);	
	memset(m_sample_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_sample_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_x, 0, sizeof(KCL::uint32)*SAMPLE_C);
	memset(m_mosaic_coords_y, 0, sizeof(KCL::uint32)*SAMPLE_C);
	m_mosaic_idx = 0;
}


void OffscreenManager::renderToScratch() const
{
	FBO::bind( m_scratch_fbo[m_current_fbo_index] );
#ifndef STRIP_REDUNDANT_CLEARS
	glClear(GL_COLOR_BUFFER_BIT);
#endif
	glViewport( 0, 0, SAMPLE_W, SAMPLE_H);

	renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->getTextureName(), USE_MOSAIC);
}


void OffscreenManager::renderScratchToMosaic() const
{
	FBO::bind( m_mosaic_fbo );
	
	setMosaicViewPort();

	renderRectWithTexture(m_scratch_fbo[m_current_fbo_index]->getTextureName(), USE_SCRATCH);
}


void OffscreenManager::renderScratchToBackScreen() const
{
	FBO::bind( 0 );
			
	glViewport( 0, 0, SAMPLE_W, SAMPLE_H);

	renderRectWithTexture(m_scratch_fbo[m_current_fbo_index]->getTextureName(), USE_SCRATCH);
}


void OffscreenManager::renderToOnscrMosaic()
{
	m_onscr_mosaic_idx %= ONSCR_SAMPLE_C;

	FBO::bind( m_onscr_mosaic_fbo );
	setOnscrSampleViewPort();
	++m_onscr_mosaic_idx;

	renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->getTextureName(), USE_FULL);	
}


void OffscreenManager::renderOnscrMosaicToBackScreen(const bool isRotated) const
{
	OpenGLStateManager::GlUseProgram(m_onscr_mosaic_program[isRotated]);

	FBO::bind( 0 );
#ifndef STRIP_REDUNDANT_CLEARS
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

	setOnscrMosaicViewPort(isRotated);

	renderRectWithTexture(m_onscr_mosaic_fbo->getTextureName(), USE_FULL);
}

void OffscreenManager::renderOffscrToBackScreen(const bool isRotated) const
{
    OpenGLStateManager::GlUseProgram(m_onscr_mosaic_program[isRotated]);

	FBO::bind( 0 );
#ifndef STRIP_REDUNDANT_CLEARS
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT);
#endif

    /*
	if(isRotated)
    {
        glViewport( 0, 0, m_onscreen_height, m_onscreen_width);
    }
    else
    {
        glViewport( 0, 0, m_onscreen_width, m_onscreen_height);
    }
    */
    glViewport( 0, 0, m_onscreen_width, m_onscreen_height);

    renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->getTextureName(), USE_FULL);
}

void OffscreenManager::setOnscrSampleViewPort() const
{
	glViewport(m_onscr_mosaic_coords_x[m_onscr_mosaic_idx], m_onscr_mosaic_coords_y[m_onscr_mosaic_idx], ONSCR_SAMPLE_W, ONSCR_SAMPLE_H);
}


void OffscreenManager::setOnscrMosaicViewPort(const bool isRotated) const
{
	glViewport( m_onscr_mosaic_x[isRotated], m_onscr_mosaic_y[isRotated], m_onscr_mosaic_viewport_width[isRotated], m_onscr_mosaic_viewport_height[isRotated]);
}


int OffscreenManager::Init( unsigned int onscreen_width, unsigned int onscreen_height, const TestDescriptor &td)
{
	FBO_COLORMODE color_mode;
	FBO_DEPTHMODE depth_mode;

	m_refresh_msec = td.m_hybrid_refresh_msec;

    m_virtual_resolution = td.m_virtual_resolution && (td.GetScreenMode() == SMode_Onscreen);
    m_method = ( (SMode_Offscreen == td.GetScreenMode() || m_virtual_resolution) ? OM_ORIGINAL : OM_HYBRID);

	int w = td.m_viewport_width;
	int h = td.m_viewport_height;

	int max_texture_dim;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_dim);
	INFO("Max texture size: %d" , max_texture_dim);

	if( w > max_texture_dim || h > max_texture_dim)
	{
		return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}

	if( m_offscreen_default_viewport_width > max_texture_dim || m_offscreen_default_viewport_height > max_texture_dim)
	{
		return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}
	

	if( w < 1 || h < 1)
	{
		return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
	}

	if( td.m_play_time < 1)
	{
		assert(0);
	}
	m_width = w;
	m_height = h;

    m_onscreen_width = onscreen_width;
    m_onscreen_height = onscreen_height;

	m_onscr_mosaic_viewport_width [0] = ONSCR_MOSAIC_WIDTH;
	m_onscr_mosaic_viewport_height[0] = ONSCR_MOSAIC_HEIGHT;

	m_onscr_mosaic_viewport_width [1] = ONSCR_MOSAIC_HEIGHT;
	m_onscr_mosaic_viewport_height[1] = ONSCR_MOSAIC_WIDTH;
	
	m_onscr_mosaic_x[0] = onscreen_width  > m_onscr_mosaic_viewport_width [0] ? (onscreen_width  - m_onscr_mosaic_viewport_width [0]) / 2 : 0;
	m_onscr_mosaic_y[0] = onscreen_height > m_onscr_mosaic_viewport_height[0] ? (onscreen_height - m_onscr_mosaic_viewport_height[0]) / 2 : 0;
	
	m_onscr_mosaic_x[1] = onscreen_width  > m_onscr_mosaic_viewport_width [1] ? (onscreen_width  - m_onscr_mosaic_viewport_width [1]) / 2 : 0;
	m_onscr_mosaic_y[1] = onscreen_height > m_onscr_mosaic_viewport_height[1] ? (onscreen_height - m_onscr_mosaic_viewport_height[1]) / 2 : 0;

	createOffscrProg();
	
	m_next_slot_time_interval = (KCL::uint32)td.m_play_time / (int)SAMPLE_C;
	if((KCL::uint32)td.m_play_time % (int)SAMPLE_C)
	{
		++m_next_slot_time_interval;
	}
	m_next_slot_previous_time = 0;
	
	m_globalFBO = FBO::GetGlobalFBO();

	if( td.m_color_bpp >= 24)
	{
		color_mode = RGB888_Linear;
	}
	else
	{
		color_mode = RGB565_Linear;
	}

	if( td.m_depth_bpp >= 24)
	{
		depth_mode = DEPTH_24_RB;
	}
	else
	{
		depth_mode = DEPTH_16_RB;
	}

	try
	{
		m_offscreen_fbo[0] = new FBO(m_width, m_height, td.m_fsaa, color_mode, depth_mode, "m_offscreen_fbo[0]");
		m_offscreen_fbo[1] = new FBO(m_width, m_height, td.m_fsaa, color_mode, depth_mode, "m_offscreen_fbo[1]");
		m_scratch_fbo[0] = new FBO(SCRATCH_WIDTH, SCRATCH_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_scratch_fbo[0]");
		m_scratch_fbo[1] = new FBO(SCRATCH_WIDTH, SCRATCH_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_scratch_fbo[1]");
		m_mosaic_fbo = new FBO(MOSAIC_WIDTH, MOSAIC_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_mosaic_fbo") ;
		m_onscr_mosaic_fbo = new FBO(ONSCR_MOSAIC_WIDTH, ONSCR_MOSAIC_HEIGHT, td.m_fsaa, color_mode, DEPTH_None, "m_onscr_mosaic_fbo");
	}
	catch (...)
	{
		SAFE_DELETE(m_offscreen_fbo[0]);
		SAFE_DELETE(m_offscreen_fbo[1]);
		SAFE_DELETE(m_mosaic_fbo);
		SAFE_DELETE(m_scratch_fbo[0]);
		SAFE_DELETE(m_scratch_fbo[1]);
		SAFE_DELETE(m_onscr_mosaic_fbo);

		if(td.m_fsaa > 0)
			return KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED_IN_MSAA;

		return KCL_TESTERROR_OFFSCREEN_NOT_SUPPORTED;
	}

	{
		bool isRotated = onscreen_width < onscreen_height;
		
		float vw = m_width;
		float vh = m_height;
		float w = !isRotated ? onscreen_width : onscreen_height;
		float h = !isRotated ? onscreen_height : onscreen_width;
		float var = vw / vh;
		float ar = w / h;

		if ( var > ar )
		{
			h = w / var;
		}
		else
		{
			w = var * h;
		}

		if (isRotated)
		{
			float tmp = w;
			w = h;
			h = tmp;
		}

		m_hybrid_onscreen_width = w;
		m_hybrid_onscreen_height = h;
	}

	FBO::SetGlobalFBO( m_globalFBO );
	return KCL_TESTERROR_NOERROR;
}


void OffscreenManager::PreRender() const
{
	OpenGLStateManager::Save();

	FBO::SetGlobalFBO( m_offscreen_fbo[m_current_fbo_index]);
	FBO::bind( m_offscreen_fbo[m_current_fbo_index] );
	glViewport( 0, 0, m_width, m_height);
}


void OffscreenManager::PostRender(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
    DiscardDepthAttachment();

	switch( m_method)
	{
	case OM_ORIGINAL:
		{
			PostRender_original( time, frame, current_viewport_width, current_viewport_height, force_swap_buffer);
			break;
		}
	case OM_HYBRID:
		{
			PostRender_hybrid( time, frame, current_viewport_width, current_viewport_height, force_swap_buffer);
			break;
		}
	}
}

void OffscreenManager_CoreProfileVAO::PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	OpenGLStateManager::GlUseProgram(m_offscreen_program);
	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	
	glUniform1i( m_offscr_text_unif_loc, 0);
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::Commit();
	
	renderToScratch();

	if(time - m_next_slot_previous_time >= m_next_slot_time_interval || force_swap_buffer)
	{
		renderScratchToMosaic();
		save_sample_time(time);
		m_next_slot_previous_time = time;
		m_mosaic_idx = (m_mosaic_idx + 1) % COUNT_OF(m_sample_times);
	}

	renderToOnscrMosaic();

	FBO::SetGlobalFBO( m_globalFBO );

	if(( frame % OFFSCR_RENDER_TIMER) == 0 || force_swap_buffer)
	{
		renderScratchToBackScreen();
	}

    if(!m_virtual_resolution)
    {
	    if( IsSwapBufferNeeded() || force_swap_buffer)
	    {
		    renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
	    }
	    else
	    {
#ifndef SIGNIFICANT_FRAME_MODE 
		    glFlush();
#else
		    glFinish();
#endif
	    }
    }
    else
    {
        renderOffscrToBackScreen(current_viewport_width < current_viewport_height);
    }
		
	glViewport( 0, 0, current_viewport_width, current_viewport_height);
	m_current_fbo_index ^= 1;

	OpenGLStateManager::Restore();
}

void OffscreenManager::PostRender_original(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	OpenGLStateManager::GlUseProgram(m_offscreen_program);
	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	
	glUniform1i( m_offscr_text_unif_loc, 0);
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::GlEnableVertexAttribArray(0);
	OpenGLStateManager::GlEnableVertexAttribArray(1);
	OpenGLStateManager::Commit();
	
	glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
	glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

	renderToScratch();

	if(time - m_next_slot_previous_time >= m_next_slot_time_interval || force_swap_buffer)
	{
		renderScratchToMosaic();
		save_sample_time(time);
		m_next_slot_previous_time = time;
		m_mosaic_idx = (m_mosaic_idx + 1) % COUNT_OF(m_sample_times);
	}

	renderToOnscrMosaic();

	FBO::SetGlobalFBO( m_globalFBO );

	if(( frame % OFFSCR_RENDER_TIMER) == 0 || force_swap_buffer)
	{
		renderScratchToBackScreen();
	}

    if(!m_virtual_resolution)
    {
	    if( IsSwapBufferNeeded() || force_swap_buffer)
	    {
		    renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
	    }
	    else
	    {
#ifndef SIGNIFICANT_FRAME_MODE 
		    glFlush();
#else
		    glFinish();
#endif
	    }
    }
    else
    {
        renderOffscrToBackScreen(current_viewport_width < current_viewport_height);
    }

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
	OpenGLStateManager::GlDisableVertexAttribArray(0);
	OpenGLStateManager::GlDisableVertexAttribArray(1);

	glViewport( 0, 0, current_viewport_width, current_viewport_height);
	m_current_fbo_index ^= 1;

	OpenGLStateManager::Restore();
}




void OffscreenManager::PostRender_hybrid(KCL::uint32 time, KCL::uint32 frame, int current_viewport_width, int current_viewport_height, bool force_swap_buffer)
{
	OpenGLStateManager::GlUseProgram(m_offscreen_program);
	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	
	glUniform1i( m_offscr_text_unif_loc, 0);
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::GlEnableVertexAttribArray(0);
	OpenGLStateManager::GlEnableVertexAttribArray(1);
	OpenGLStateManager::Commit();
	
	glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
	glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

	renderToScratch();

	if(time - m_next_slot_previous_time >= m_next_slot_time_interval)
	{
		renderScratchToMosaic();
		save_sample_time(time);
		m_next_slot_previous_time = time;
		m_mosaic_idx = (m_mosaic_idx + 1) % COUNT_OF(m_sample_times);
	}

	//renderToOnscrMosaic();
	m_onscr_mosaic_idx = 0;

	FBO::SetGlobalFBO( m_globalFBO );

	if(( frame % OFFSCR_RENDER_TIMER) == 0)
	{
		renderScratchToBackScreen();
	}

	//if( IsSwapBufferNeeded())
	//{
		//renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
	//}
	
	if( time - m_last_refresh_msec >= m_refresh_msec)
	{
		bool isRotated = current_viewport_width < current_viewport_height;

		OpenGLStateManager::GlUseProgram(m_onscr_mosaic_program[isRotated]);

		FBO::bind( 0 );
#ifndef STRIP_REDUNDANT_CLEARS
		glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT);
#endif

		glViewport( (current_viewport_width-m_hybrid_onscreen_width)/2, (current_viewport_height-m_hybrid_onscreen_height)/2, m_hybrid_onscreen_width, m_hybrid_onscreen_height);

		renderRectWithTexture( m_offscreen_fbo[!m_current_fbo_index]->getTextureName(), USE_FULL);

		m_last_refresh_msec = time;
		m_onscr_mosaic_idx = ONSCR_SAMPLE_C;
	}
	else
	{
#if (defined __glew_h__ && defined WIN32) || defined SIGNIFICANT_FRAME_MODE 
		glFinish();
#else
		glFlush();
#endif
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
	OpenGLStateManager::GlDisableVertexAttribArray(0);
	OpenGLStateManager::GlDisableVertexAttribArray(1);

	glViewport( 0, 0, current_viewport_width, current_viewport_height);
	m_current_fbo_index ^= 1;

	OpenGLStateManager::Restore();
}


void OffscreenManager::Clear()
{
	deleteOffscrProg();
	clear_saved_sample_data();
}

void OffscreenManager::ResetGlobalFBO(int current_viewport_width, int current_viewport_height) const
{
	FBO::SetGlobalFBO( m_globalFBO );
	FBO::bind( 0 );
	glClear(GL_COLOR_BUFFER_BIT);
	glViewport( 0, 0, current_viewport_width, current_viewport_height);
#ifndef SIGNIFICANT_FRAME_MODE 
	glFlush();
#else
	glFinish();
#endif
}


void OffscreenManager::setMosaicViewPort() const
{
	glViewport(m_mosaic_coords_x[m_mosaic_idx], m_mosaic_coords_y[m_mosaic_idx], SAMPLE_W, SAMPLE_H);
}


void OffscreenManager::SaveForTesting() const
{	
	KCL::uint8* captured_samples = new KCL::uint8[MOSAIC_WIDTH * MOSAIC_HEIGHT * 4];
	std::stringstream ss;
	int timestamp = (int)time(0);

	FBO::bind( m_mosaic_fbo );	
	glReadPixels(0, 0, MOSAIC_WIDTH, MOSAIC_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, captured_samples);
	FBO::bind( 0 );
	
	ss << "mosaic_fbo_" << timestamp << ".tga";
	convertRGBAtoBGR(captured_samples, MOSAIC_WIDTH * MOSAIC_HEIGHT);

	KCL::Image::saveTga(ss.str().c_str(), MOSAIC_WIDTH, MOSAIC_HEIGHT, captured_samples, KCL::Image_RGB888);

	delete[] captured_samples;
}


void OffscreenManager::RenderCurrentMosaic()
{
    OpenGLStateManager::GlUseProgram(m_offscreen_program);
    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

    glUniform1i( m_offscr_text_unif_loc, 0);
    OpenGLStateManager::DisableAllCapabilites();
    OpenGLStateManager::GlEnableVertexAttribArray(0);
    OpenGLStateManager::GlEnableVertexAttribArray(1);
    OpenGLStateManager::Commit();

    glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);	
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

    renderRectWithTexture(m_offscreen_fbo[!m_current_fbo_index]->getTextureName(), USE_FULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    OpenGLStateManager::GlDisableVertexAttribArray(0);
    OpenGLStateManager::GlDisableVertexAttribArray(1);

    OpenGLStateManager::Restore();
}

void OffscreenManager_CoreProfileVAO::RenderCurrentMosaic()
{
    OpenGLStateManager::GlUseProgram(m_offscreen_program);
    OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);

    glUniform1i( m_offscr_text_unif_loc, 0);
    OpenGLStateManager::DisableAllCapabilites();
    OpenGLStateManager::Commit();

    renderRectWithTexture(m_offscreen_fbo[!m_current_fbo_index]->getTextureName(), USE_FULL);

    OpenGLStateManager::Restore();
}


void OffscreenManager::RenderLastFrames(int current_viewport_width, int current_viewport_height)
{
    OpenGLStateManager::GlUseProgram(m_offscreen_program);
	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	
	glUniform1i( m_offscr_text_unif_loc, 0);
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::GlEnableVertexAttribArray(0);
	OpenGLStateManager::GlEnableVertexAttribArray(1);
	OpenGLStateManager::Commit();
	
	glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
	glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

	FBO::SetGlobalFBO( m_globalFBO );

	renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
		
	OpenGLStateManager::GlDisableVertexAttribArray(0);
	OpenGLStateManager::GlDisableVertexAttribArray(1);

	glViewport( 0, 0, current_viewport_width, current_viewport_height);

	OpenGLStateManager::Restore();
}

void OffscreenManager_CoreProfileVAO::RenderLastFrames(int current_viewport_width, int current_viewport_height)
 {     
    OpenGLStateManager::GlUseProgram(m_offscreen_program);
	OpenGLStateManager::GlActiveTexture(GL_TEXTURE0);
	
	glUniform1i( m_offscr_text_unif_loc, 0);
	OpenGLStateManager::DisableAllCapabilites();
	OpenGLStateManager::Commit();
	
	FBO::SetGlobalFBO( m_globalFBO );

    renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
	
	glViewport( 0, 0, current_viewport_width, current_viewport_height);

	OpenGLStateManager::Restore();
 }
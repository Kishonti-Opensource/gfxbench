/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "offscreenmanager.h"

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cassert>

#include <ng/log.h>
#include "oglx/gl.h"

OffscreenManager* OffscreenManager::Create()
{
    return new OffscreenManager();
}


OffscreenManager::OffscreenManager()
{
    srand( time(0));

    memset(m_sample_times, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_sample_coords_x, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_sample_coords_y, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_mosaic_coords_x, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_mosaic_coords_y, 0, sizeof(unsigned int)*SAMPLE_C);

    m_width = 0;
    m_height = 0;
    m_globalFBO = 0;

    m_virtual_resolution = false;

    m_mode = OnscreenMode;

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

    m_swap_buffer_needed = false;
    m_last_swap_time = 0;
    m_finish_fbo = NULL;
}


void OffscreenManager::init_onscr_mosaic_coords()
{
    for(unsigned int i=0; i<ONSCR_SAMPLE_C; ++i)
    {
        m_onscr_mosaic_coords_x[i] = (i % ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_W;
        m_onscr_mosaic_coords_y[i] = (i / ONSCR_SAMPLE_NUM_X) * ONSCR_SAMPLE_H;
    }
}


OffscreenManager::~OffscreenManager()
{
    Clear();
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

    m_offscreen_vertex_shader = initShader(GL_VERTEX_SHADER, srcVert);
    m_offscreen_fragment_shader = initShader(GL_FRAGMENT_SHADER, srcFrag);

    m_offscreen_program = glCreateProgram();

    glAttachShader(m_offscreen_program, m_offscreen_vertex_shader);
    glAttachShader(m_offscreen_program, m_offscreen_fragment_shader);

    glBindAttribLocation(m_offscreen_program, 0, "myVertex");
    glBindAttribLocation(m_offscreen_program, 1, "myTexCoord");

    glLinkProgram(m_offscreen_program);

    m_offscr_text_unif_loc = glGetUniformLocation( m_offscreen_program, "texChars");

    //glsl_log(m_offscreen_program, GL_LINK_STATUS, "link");


    // ONSCREEN MOSAIC PROGRAM CREATION:
    m_onscr_mosaic_program[0] = glCreateProgram();

    glAttachShader(m_onscr_mosaic_program[0], m_offscreen_vertex_shader);
    glAttachShader(m_onscr_mosaic_program[0], m_offscreen_fragment_shader);

    glBindAttribLocation(m_onscr_mosaic_program[0], 0, "myVertex");
    glBindAttribLocation(m_onscr_mosaic_program[0], 1, "myTexCoord");

    glLinkProgram(m_onscr_mosaic_program[0]);

    m_onscr_mosaic_text_unif_loc[0] = glGetUniformLocation( m_onscr_mosaic_program[0], "texChars");

    //glsl_log(m_onscr_mosaic_program[0], GL_LINK_STATUS, "link");


    unsigned int vertex_shader2 = initShader(GL_VERTEX_SHADER, srcVert2);
    m_onscr_mosaic_program[1] = glCreateProgram();

    glAttachShader(m_onscr_mosaic_program[1], vertex_shader2);
    glAttachShader(m_onscr_mosaic_program[1], m_offscreen_fragment_shader);

    glBindAttribLocation(m_onscr_mosaic_program[1], 0, "myVertex");
    glBindAttribLocation(m_onscr_mosaic_program[1], 1, "myTexCoord");

    glLinkProgram(m_onscr_mosaic_program[1]);

    m_onscr_mosaic_text_unif_loc[1] = glGetUniformLocation( m_onscr_mosaic_program[0], "texChars");

    //glsl_log(m_onscr_mosaic_program[1], GL_LINK_STATUS, "link");



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
    for(unsigned int i=1; i<=SAMPLE_C; ++i)
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

        unsigned int idx = i-1;
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

    uint8_t indicesTmp[] =
    {
        0, 1, 2,
        0, 2, 3
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint8_t) * 6, (const void*)indicesTmp, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //m_globalFBO = FBO::GetGlobalFBO();
}

unsigned int OffscreenManager::initShader(unsigned int shaderType, const char *shaderSource)
{
    GLuint shader_id = glCreateShader (shaderType);
    glShaderSource(shader_id, 1, (const char **)&shaderSource, NULL);
    glCompileShader(shader_id);

    GLint compiled = GL_FALSE;
    GLint log_len = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compiled);
    glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &log_len);

    if (log_len)
    {
        std::vector<char> log_buffer(log_len);
        glGetShaderInfoLog(shader_id, log_len, NULL, log_buffer.data());
        NGLOG_ERROR("Compiler log:");
        NGLOG_ERROR("%s", log_buffer.data());
    }

    if (!compiled)
    {
        NGLOG_ERROR("Shader compile error!");
    }

    return shader_id;
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

    delete m_offscreen_fbo[0];
    delete m_offscreen_fbo[1];

    delete m_scratch_fbo[0];
    delete m_scratch_fbo[1];

    delete m_mosaic_fbo;
    delete m_onscr_mosaic_fbo;

    delete m_finish_fbo;

    m_onscr_mosaic_idx = 0;
}


void OffscreenManager::renderRectWithTexture(unsigned int textureId, TextureCoordinatesMode textureCoordinatesMode) const
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
    unsigned int xui = rand() % (m_width - SAMPLE_W);
    unsigned int yui = rand() % (m_height - SAMPLE_H);

    X_L = (float)xui / (float)m_width;
    Y_D = (float)yui / (float)m_height;
    X_R = X_L + (float)SAMPLE_W / m_width;
    Y_U = Y_D + (float)SAMPLE_H / m_height;

    m_sample_coords_x[idx] = xui;
    m_sample_coords_y[idx] = yui;
}


void OffscreenManager::clear_saved_sample_data()
{
    memset(m_sample_times, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_sample_coords_x, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_sample_coords_y, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_mosaic_coords_x, 0, sizeof(unsigned int)*SAMPLE_C);
    memset(m_mosaic_coords_y, 0, sizeof(unsigned int)*SAMPLE_C);
    m_mosaic_idx = 0;
}


void OffscreenManager::renderToScratch() const
{
    //FBO::bind( m_scratch_fbo[m_current_fbo_index] );
    glBindFramebuffer(GL_FRAMEBUFFER, m_scratch_fbo[m_current_fbo_index]->m_fbo);

#ifndef STRIP_REDUNDANT_CLEARS
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    glViewport( 0, 0, SAMPLE_W, SAMPLE_H);

    renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->m_color_texture, USE_MOSAIC);
}


void OffscreenManager::renderScratchToMosaic() const
{
    //FBO::bind( m_mosaic_fbo );
    glBindFramebuffer(GL_FRAMEBUFFER, m_mosaic_fbo->m_fbo);

    setMosaicViewPort();

    renderRectWithTexture(m_scratch_fbo[m_current_fbo_index]->m_color_texture, USE_SCRATCH);
}


void OffscreenManager::renderScratchToBackScreen() const
{
    //FBO::bind( 0 );
    glBindFramebuffer(GL_FRAMEBUFFER, m_globalFBO);

    glViewport( 0, 0, SAMPLE_W, SAMPLE_H);

    renderRectWithTexture(m_scratch_fbo[m_current_fbo_index]->m_color_texture, USE_SCRATCH);
}


void OffscreenManager::renderToOnscrMosaic()
{
    m_onscr_mosaic_idx %= ONSCR_SAMPLE_C;

    //FBO::bind( m_onscr_mosaic_fbo );
    glBindFramebuffer(GL_FRAMEBUFFER, m_onscr_mosaic_fbo->m_fbo);
    setOnscrSampleViewPort();
    ++m_onscr_mosaic_idx;

    renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->m_color_texture, USE_FULL);
}


void OffscreenManager::renderOnscrMosaicToBackScreen(const bool isRotated) const
{
    glUseProgram(m_onscr_mosaic_program[isRotated]);

    //FBO::bind( 0 );
    glBindFramebuffer(GL_FRAMEBUFFER, m_globalFBO);
#ifndef STRIP_REDUNDANT_CLEARS
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT);
#endif

    setOnscrMosaicViewPort(isRotated);

    renderRectWithTexture(m_onscr_mosaic_fbo->m_color_texture, USE_FULL);
}

void OffscreenManager::renderOffscrToBackScreen(const bool isRotated) const
{
    glUseProgram(m_onscr_mosaic_program[isRotated]);

    //FBO::bind( 0 );
    glBindFramebuffer(GL_FRAMEBUFFER, m_globalFBO);
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

    renderRectWithTexture(m_offscreen_fbo[m_current_fbo_index]->m_color_texture, USE_FULL);
}

void OffscreenManager::setOnscrSampleViewPort() const
{
    glViewport(m_onscr_mosaic_coords_x[m_onscr_mosaic_idx], m_onscr_mosaic_coords_y[m_onscr_mosaic_idx], ONSCR_SAMPLE_W, ONSCR_SAMPLE_H);
}


void OffscreenManager::setOnscrMosaicViewPort(const bool isRotated) const
{
    glViewport( m_onscr_mosaic_x[isRotated], m_onscr_mosaic_y[isRotated], m_onscr_mosaic_viewport_width[isRotated], m_onscr_mosaic_viewport_height[isRotated]);
}


int OffscreenManager::Init(ManagerMode mode, unsigned int onscreen_width, unsigned int onscreen_height, unsigned int play_time, unsigned int color_bpp, unsigned int depth_bpp, unsigned int default_fbo)
{
    m_virtual_resolution = false;
    m_globalFBO = default_fbo;

    m_mode = mode;
    if (mode == OnscreenMode)
    {
        return 0;
    }

    // Offscreen resolutions
    int w = width();
    int h = height();

    int max_texture_dim;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_dim);
    //NGLOG_INFO("Max texture size: %d" , max_texture_dim);

    if( w > max_texture_dim || h > max_texture_dim)
    {
        //return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
        return 1;
    }

    if( w < 1 || h < 1)
    {
        //return KCL_TESTERROR_INVALID_SCREEN_RESOLUTION;
        return 1;
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

    m_next_slot_time_interval = (unsigned int)play_time / (int)SAMPLE_C;
    if((unsigned int)play_time % (int)SAMPLE_C)
    {
        ++m_next_slot_time_interval;
    }
    m_next_slot_previous_time = 0;

    FBO::ColorMode color_mode;
    FBO::DepthMode depth_mode;
    if( color_bpp >= 24)
    {
        color_mode = FBO::RGB888_Linear;
    }
    else
    {
        color_mode = FBO::RGB565_Linear;
    }

    if( depth_bpp >= 24)
    {
        depth_mode = FBO::DEPTH_24_RB;
    }
    else
    {
        depth_mode = FBO::DEPTH_16_RB;
    }

    try
    {
        // TODO: error handling
        m_offscreen_fbo[0] = new FBO(m_width, m_height, color_mode, depth_mode, "m_offscreen_fbo[0]");
        m_offscreen_fbo[1] = new FBO(m_width, m_height, color_mode, depth_mode, "m_offscreen_fbo[1]");
        m_scratch_fbo[0] = new FBO(SCRATCH_WIDTH, SCRATCH_HEIGHT, color_mode, FBO::DEPTH_NONE, "m_scratch_fbo[0]");
        m_scratch_fbo[1] = new FBO(SCRATCH_WIDTH, SCRATCH_HEIGHT, color_mode, FBO::DEPTH_NONE, "m_scratch_fbo[1]");
        m_mosaic_fbo = new FBO(MOSAIC_WIDTH, MOSAIC_HEIGHT, color_mode, FBO::DEPTH_NONE, "m_mosaic_fbo") ;
        m_onscr_mosaic_fbo = new FBO(ONSCR_MOSAIC_WIDTH, ONSCR_MOSAIC_HEIGHT, color_mode, FBO::DEPTH_NONE, "m_onscr_mosaic_fbo");
    }
    catch (...)
    {
        delete m_offscreen_fbo[0];
        delete m_offscreen_fbo[1];
        delete m_mosaic_fbo;
        delete m_scratch_fbo[0];
        delete m_scratch_fbo[1];
        delete m_onscr_mosaic_fbo;

        return 1;
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
    }

    m_finish_fbo = new FBO(FINISH_FBO_SIZE, FINISH_FBO_SIZE, FBO::RGB565_Linear, FBO::DEPTH_NONE, "::FinishFBO");

    //FBO::SetGlobalFBO( m_globalFBO );
    //return KCL_TESTERROR_NOERROR;
    return 0;
}


void OffscreenManager::PreRender() const
{
    if (m_mode == OnscreenMode)
    {
        return;
    }

    //OpenGLStateManager::Save();

    //FBO::SetGlobalFBO( m_offscreen_fbo[m_current_fbo_index]);
    //FBO::bind( m_offscreen_fbo[m_current_fbo_index] );

    glBindFramebuffer(GL_FRAMEBUFFER, m_offscreen_fbo[m_current_fbo_index]->m_fbo);
    glViewport( 0, 0, m_width, m_height);
}


bool OffscreenManager::PostRender(unsigned int time, unsigned int frame, int current_viewport_width, int current_viewport_height)
{
    if (m_mode == OnscreenMode)
    {
        return true;
    }

    return PostRender_original( time, frame, current_viewport_width, current_viewport_height);
}

bool OffscreenManager::PostRender_original(unsigned int time, unsigned int frame, int current_viewport_width, int current_viewport_height)
{
    bool swap_interval_elapsed = time - m_last_swap_time > SWAP_BUFFER_INTERVAL;

    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_offscreen_program);
    glActiveTexture(GL_TEXTURE0);

    glUniform1i( m_offscr_text_unif_loc, 0);
    //glDisableAllCapabilites();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);


    glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

    renderToScratch();

    if(time - m_next_slot_previous_time >= m_next_slot_time_interval || swap_interval_elapsed)
    {
        renderScratchToMosaic();
        save_sample_time(time);
        m_next_slot_previous_time = time;
        m_mosaic_idx = (m_mosaic_idx + 1) % (sizeof(m_sample_times)/sizeof(m_sample_times[0]));
    }

    renderToOnscrMosaic();
    bool mosaic_full = m_onscr_mosaic_idx == ONSCR_SAMPLE_C;

    //FBO::SetGlobalFBO( m_globalFBO );

    if(( frame % OFFSCR_RENDER_TIMER) == 0 || swap_interval_elapsed)
    {
        renderScratchToBackScreen();
    }

    if(!m_virtual_resolution)
    {
        if( swap_interval_elapsed || mosaic_full)
        {
            renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);
        }
        else
        {
            glFlush();
        }
    }
    else
    {
        renderOffscrToBackScreen(current_viewport_width < current_viewport_height);
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glViewport( 0, 0, current_viewport_width, current_viewport_height);
    m_current_fbo_index ^= 1;

    //OpenGLStateManager::Restore();

    m_swap_buffer_needed = swap_interval_elapsed || mosaic_full;
    if (m_swap_buffer_needed)
    {
        m_last_swap_time = time;
    }

    return m_swap_buffer_needed;
}

void OffscreenManager::Clear()
{
    deleteOffscrProg();
    clear_saved_sample_data();
}

void OffscreenManager::setMosaicViewPort() const
{
    glViewport(m_mosaic_coords_x[m_mosaic_idx], m_mosaic_coords_y[m_mosaic_idx], SAMPLE_W, SAMPLE_H);
}

void OffscreenManager::FinishRendering()
{
    if (m_mode == OnscreenMode)
    {
        glFinish();
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_finish_fbo->m_fbo);
    glViewport(0, 0, FINISH_FBO_SIZE, FINISH_FBO_SIZE);

    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_offscreen_program);
    glActiveTexture(GL_TEXTURE0);

    glUniform1i( m_offscr_text_unif_loc, 0);
    //DisableAllCapabilites();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    //OpenGLStateManager::Commit();

    glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

    renderRectWithTexture(m_offscreen_fbo[!m_current_fbo_index]->m_color_texture, USE_FULL);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Read back the rendered values
    GLubyte values[FINISH_FBO_SIZE * FINISH_FBO_SIZE * 4];
    glReadPixels(0, 0, FINISH_FBO_SIZE, FINISH_FBO_SIZE, GL_RGBA, GL_UNSIGNED_BYTE, values);
}

bool OffscreenManager::RenderLastFrames(int current_viewport_width, int current_viewport_height)
{
    if (m_mode == OnscreenMode || m_swap_buffer_needed)
    {
        return false;
    }

    glDisable(GL_DEPTH_TEST);

    glUseProgram(m_offscreen_program);
    glActiveTexture(GL_TEXTURE0);

    glUniform1i( m_offscr_text_unif_loc, 0);
    //glDisableAllCapabilites();
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, m_offscreen_vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_offscreen_ebo);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);

    //FBO::SetGlobalFBO( m_globalFBO );

    renderOnscrMosaicToBackScreen(current_viewport_width < current_viewport_height);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    glViewport( 0, 0, current_viewport_width, current_viewport_height);

    //OpenGLStateManager::Restore();

    return true;
}

OffscreenManager::FBO::FBO(unsigned int width, unsigned int height, ColorMode color_mode, DepthMode depth_mode, const char *debug_label)
{
    m_width = width;
    m_height = height;

    m_debug_label = debug_label;

    m_fbo = 0;
    m_color_texture = 0;
    m_depth_renderbuffer = 0;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    GLbitfield clear_mask = GL_COLOR_BUFFER_BIT;

    glGenTextures(1, &m_color_texture);
    glBindTexture(GL_TEXTURE_2D, m_color_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    switch (color_mode)
    {
        case RGB888_Linear:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            break;

        case RGB565_Linear:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
            break;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_texture, 0);


    if (depth_mode)
    {
        clear_mask |= GL_DEPTH_BUFFER_BIT;

        glGenRenderbuffers(1, &m_depth_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_depth_renderbuffer);

        switch (depth_mode)
        {
            case DEPTH_16_RB:
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_width, m_height);

                break;
            case DEPTH_24_RB:
                    #ifndef GL_DEPTH_COMPONENT24_OES
                    #define GL_DEPTH_COMPONENT24_OES GL_DEPTH_COMPONENT24
                    #endif
                glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24_OES, m_width, m_height);
                break;
        }
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_renderbuffer);
    }

    GLenum status = glCheckFramebufferStatus (GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        NGLOG_ERROR("Incomplete FBO %s, %s", m_debug_label, status);
    }
    glClear(clear_mask);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return;
}

OffscreenManager::FBO::~FBO()
{
    if (m_color_texture)
    {
        glDeleteTextures(1, &m_color_texture);
    }
    if (m_depth_renderbuffer)
    {
        glDeleteRenderbuffers(1, &m_depth_renderbuffer);
    }
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
    }
}

int32_t OffscreenManager::width() const
{
    return 1920;
}

int32_t OffscreenManager::height() const
{
    return 1080;
}

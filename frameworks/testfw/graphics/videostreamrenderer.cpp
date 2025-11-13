/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "ng/log.h"
#include "videostream.h"
#include "graphics/videostreamrenderer.h"
#ifdef HAVE_EPOXY
#include <epoxy/gl.h>
#else
#include <GLES2/gl2.h>
    #define EGL_CONTEXT_MAJOR_VERSION_KHR                                   0x3098
    #define EGL_CONTEXT_MINOR_VERSION_KHR                                   0x30FB
    #define GL_TEXTURE_EXTERNAL_OES                                         0x8D65

    #define GL_STACK_OVERFLOW                                               0x0503
    #define GL_STACK_OVERFLOW_KHR                                           0x0503
    #define GL_STACK_UNDERFLOW                                              0x0504
    #define GL_STACK_UNDERFLOW_KHR                                          0x0504
    #define GL_OUT_OF_MEMORY                                                0x0505
    #define GL_INVALID_FRAMEBUFFER_OPERATION                                0x0506
    #define GL_INVALID_FRAMEBUFFER_OPERATION_EXT                            0x0506
    #define GL_INVALID_FRAMEBUFFER_OPERATION_OES                            0x0506

    #define GL_RGBA                                                         0x1908
    #define GL_BGRA                                                         0x80E1
#endif
#include <fstream>

#include "ng/macro_utils.h"
NG_TABLE_START(GL_ERROR_TABLE)
  NG_TABLE_ITEM0(GL_NO_ERROR)
  NG_TABLE_ITEM0(GL_INVALID_ENUM)
  NG_TABLE_ITEM0(GL_INVALID_VALUE)
  NG_TABLE_ITEM0(GL_INVALID_OPERATION)
  NG_TABLE_ITEM0(GL_STACK_OVERFLOW)
  NG_TABLE_ITEM0(GL_STACK_UNDERFLOW)
  NG_TABLE_ITEM0(GL_OUT_OF_MEMORY)
  NG_TABLE_ITEM0(GL_INVALID_FRAMEBUFFER_OPERATION)
NG_TABLE_END(GL_ERROR_TABLE)


#define CHECK_GL_ERROR { static int cnt = -1; ++cnt; GLenum err = glGetError(); if (err != GL_NO_ERROR) { NGLOG_ERROR("[%s] %s:%s glGetError() = %s\n", cnt, __FILE__, __LINE__, GL_ERROR_TABLE(err).c_str()); } }


namespace {

static const char *video_stream_vs =
{
"#ifdef GL_ES\n"
"precision highp float;\n"
"#else\n"
"#define highp\n"
"#endif\n"
"uniform mat4 u_modelview_projection;"
"uniform mat4 u_texmat;"
""
"attribute vec4 a_position;"
"attribute vec4 a_texcoord;"
""
"varying vec2 v_texcoord;"
""
"void main()"
"{"
"   vec4 texcoord = u_texmat * a_texcoord;"
"   v_texcoord = texcoord.xy;"
"   gl_Position= u_modelview_projection * a_position;"
"}"
};

static const char *video_stream_eglimage_fs =
{
"#ifdef GL_ES\n"
"#extension GL_OES_EGL_image_external: require\n"
"precision highp float;\n"
"#else\n"
"#define highp\n"
"#endif\n"
"precision mediump float;"
"varying vec2 v_texcoord;"
"uniform samplerExternalOES u_sampler0;"
""
"void main()"
"{"
"   gl_FragColor = texture2D(u_sampler0, v_texcoord);"
"}"
};

static const char *video_stream_nv12_fs =
{
"#ifdef GL_ES\n"
"precision highp float;\n"
"#else\n"
"#define highp\n"
"#endif\n"
"varying vec2 v_texcoord;"
"uniform sampler2D u_sampler0;"
"uniform sampler2D u_sampler1;"
""
"const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
"const vec3 rcoeff = vec3(1.164, 0.000, 1.596);"
"const vec3 gcoeff = vec3(1.164,-0.391,-0.813);"
"const vec3 bcoeff = vec3(1.164, 2.018, 0.000);"
""
"vec4 yuv2rgb(vec3 yuv)"
"{"
"    yuv += offset;"
"    return vec4("
"        dot(yuv, rcoeff),"
"        dot(yuv, gcoeff),"
"        dot(yuv, bcoeff),"
"        1.0);"
"}"
""
"void main()"
"{"
"    float y = texture2D(u_sampler0, v_texcoord).r;"
"    vec2 uv = texture2D(u_sampler1, v_texcoord).ra;"
"    gl_FragColor = yuv2rgb(vec3(y, uv.r, uv.g));"
"}"
};

const char *video_stream_i420_fs =
"#ifdef GL_ES\n"
"precision highp float;\n"
"#else\n"
"#define highp\n"
"#endif\n"
"varying vec2 v_texcoord;\n"
"uniform sampler2D u_sampler0;\n"
"uniform sampler2D u_sampler1;\n"
"uniform sampler2D u_sampler2;\n"
""
"const vec3 offset = vec3(-0.0625, -0.5, -0.5);"
"const vec3 rcoeff = vec3(1.164, 0.000, 1.596);"
"const vec3 gcoeff = vec3(1.164,-0.391,-0.813);"
"const vec3 bcoeff = vec3(1.164, 2.018, 0.000);"
""
"vec4 yuv2rgb(vec3 yuv)"
"{"
"    yuv += offset;"
"    return vec4("
"        dot(yuv, rcoeff),"
"        dot(yuv, gcoeff),"
"        dot(yuv, bcoeff),"
"        1.0);"
"}"
""
"void main(void) {\n"
"  float y=texture2D(u_sampler0,v_texcoord).r;\n"
"  float u=texture2D(u_sampler1,v_texcoord).r;\n"
"  float v=texture2D(u_sampler2,v_texcoord).r;\n"
"  gl_FragColor = yuv2rgb(vec3(y, u, v));"
"}\n"
;

}

using namespace tfw;

VideoStreamRenderer::VideoStreamRenderer()
: prog_(0)
, vs_(0)
, fs_(0)
{}


void VideoStreamRenderer::setupGL(VideoStream *stream)
{
    glGetError();

    std::string vsSrc = video_stream_vs;
    std::string fsSrc;
#ifdef ANDROID
    fsSrc = video_stream_eglimage_fs;
#else
    if (stream->format() == "I420") {
        fsSrc = video_stream_i420_fs;
    } else if (stream->format() == "NV12") {
        fsSrc = video_stream_nv12_fs;
    } else if (stream->format() == "eglimage") {
        fsSrc = video_stream_eglimage_fs;
    } else {
        NGLOG_ERROR("unknown format: %s", stream->format());
    }
#endif

#if DEBUG_LOAD_SHADERS
    std::ifstream t("video_stream.fs");
    fsSrc.assign(std::istreambuf_iterator<char>(t), std::istreambuf_iterator<char>());
    std::ifstream s("video_stream.vs");
    vsSrc.assign(std::istreambuf_iterator<char>(s), std::istreambuf_iterator<char>());
#endif

    vs_ = glCreateShader(GL_VERTEX_SHADER);
    require(vs_, "Failed to create vertex shader");
    fs_ = glCreateShader(GL_FRAGMENT_SHADER);
    require(fs_, "Failed to create fragment shader");

    GLint length = (GLint)vsSrc.length();
    const GLchar *src = vsSrc.c_str();
    glShaderSource(vs_, 1, &src, &length);

    src = fsSrc.c_str();
    length = (GLint)fsSrc.length();
    glShaderSource(fs_, 1, &src, &length);

    compileShader(vs_);
    compileShader(fs_);
    prog_ = glCreateProgram();
    require(prog_ != 0, "glCreateProgram returned zero");

    glAttachShader(prog_, vs_);
    glAttachShader(prog_, fs_);
    require(glGetError() == GL_NO_ERROR);

    glBindAttribLocation(prog_, 0, "a_position");
    glBindAttribLocation(prog_, 1, "a_texcoord");

    glLinkProgram(prog_);
    GLenum glerr = glGetError();

    GLint linked;
    glGetProgramiv(prog_, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        std::string message = getInfoLog();
        NGLOG_ERROR("VideoStreamRenderer: %s", message);
        throw std::runtime_error(message);
    }
    require(glerr == GL_NO_ERROR, FORMATSTR("gl error in glLinkProgram and GL_LINK_STATUS failed to detect it: %s", glerr));
}

void VideoStreamRenderer::drawGL(VideoStream *stream)
{
    // updateTexImage binds the texture object(s)
    stream->updateTexImage();
    const float *t = stream->transformMatrix();
    float mvp[] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    glUseProgram(prog_);
    static const float pos[] = { -1, -1,   1, -1,   1, 1,   -1, 1 };
    static const float tex[] = {  0,  0,   1,  0,   1, 1,    0, 1 };
    GLuint mvpLoc = glGetUniformLocation(prog_, "u_modelview_projection");
    GLuint texmatLoc = glGetUniformLocation(prog_, "u_texmat");
    GLuint sampler0Loc = glGetUniformLocation(prog_, "u_sampler0");
    GLuint sampler1Loc = glGetUniformLocation(prog_, "u_sampler1");
    GLuint sampler2Loc = glGetUniformLocation(prog_, "u_sampler2");

    glUniform1i(sampler0Loc, 0);
    if (sampler2Loc != GLuint(-1))
    {
        // using three textures
        // 1: Y 8-bit luminance,
        // 2: U 8-bit luminance,
        // 3: V 8-bit luminance
        // shader code will convert YUV -> RGB
        // see: videostream_gstreamer.cpp
        glUniform1i(sampler1Loc, 1);
        glUniform1i(sampler2Loc, 2);
    }
    if (sampler1Loc != GLuint(-1))
    {
        // using two textures, 1: Y 8-bit luminance, 2: UV 8-bit luminance/alpha
        // shader code will convert YUV -> RGB
        // see: videostream_gstreamer.cpp
        glUniform1i(sampler1Loc, 1);
    }

    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvp);
    CHECK_GL_ERROR;
    glUniformMatrix4fv(texmatLoc, 1, GL_FALSE, t);
    CHECK_GL_ERROR;
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, pos);
    CHECK_GL_ERROR;
    glEnableVertexAttribArray(0);
    CHECK_GL_ERROR;
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, 0, tex);
    CHECK_GL_ERROR;
    glEnableVertexAttribArray(1);
    CHECK_GL_ERROR;
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    CHECK_GL_ERROR;
    glUseProgram(0);
    CHECK_GL_ERROR;
}

void VideoStreamRenderer::releaseGL()
{
    glDetachShader(prog_, vs_);
    glDetachShader(prog_, fs_);
    glDeleteShader(vs_);
    glDeleteShader(fs_);
    glDeleteProgram(prog_);
}

std::string VideoStreamRenderer::getInfoLog () const
{
    std::string info;
    GLint blen;
    GLsizei slen;
    glGetProgramiv(prog_, GL_INFO_LOG_LENGTH, &blen);
    if (blen > 1)
    {
        GLchar *buf = new GLchar[blen];
        glGetProgramInfoLog(prog_, blen, &slen, buf);
        info = buf;
        delete [] buf;
    }
    return info;
}

void VideoStreamRenderer::compileShader(GLuint shader)
{
    glCompileShader(shader);
    GLenum glerr = glGetError();
    GLint compiled;

    glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        std::string message;
        GLint blen = 0;
        GLsizei slen = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &blen);
        if (blen > 1)
        {
            GLchar* compileLog = new GLchar[blen];
            glGetShaderInfoLog(shader, blen, &slen, compileLog);
            message.append(compileLog);
            delete [] compileLog;
        }
        throw std::runtime_error(message);
    }
    require(glerr == GL_NO_ERROR, FORMATSTR("gl error in glCompileShader and GL_COMPILE_STATUS failed to detect it: %s", glerr));
}

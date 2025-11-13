/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <string>
#include <fstream>
#include "epoxy/gl.h"
#include "ng/require.h"
#include "ng/format.h"
#include "ng/log.h"
#include "ng/macro_utils.h"
#include "shaderprecision.h"
#include "shaderprecision_shaders.h"
#include "ng/pngio.h"
#include "ng/stream/memstreambuf.h"
#include <Poco/Base64Encoder.h>



NG_TABLE_START(GL_ERROR_TABLE)
    NG_TABLE_ITEM0(GL_NO_ERROR)
    NG_TABLE_ITEM0(GL_INVALID_ENUM)
    NG_TABLE_ITEM0(GL_INVALID_VALUE)
    NG_TABLE_ITEM0(GL_INVALID_OPERATION)
    NG_TABLE_ITEM0(GL_STACK_OVERFLOW)
    NG_TABLE_ITEM0(GL_STACK_UNDERFLOW)
    NG_TABLE_ITEM0(GL_OUT_OF_MEMORY)
    NG_TABLE_ITEM0(GL_INVALID_FRAMEBUFFER_OPERATION)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_COMPLETE)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER)
    NG_TABLE_ITEM0(GL_FRAMEBUFFER_UNSUPPORTED)
NG_TABLE_END(GL_ERROR_TABLE)

#define CHECK_GL_ERROR { static int cnt = -1; ++cnt; GLenum err = glGetError(); if (err != GL_NO_ERROR) { NGLOG_ERROR("[%s] %s:%s glGetError() = %s\n", cnt, __FILE__, __LINE__, GL_ERROR_TABLE(err).c_str()); } }


using namespace sysinf;


ShaderPrecision::ShaderPrecision()
: width_(0)
, height_(0)
, prog_(0)
{}


void ShaderPrecision::setup(int32_t width, int32_t height)
{
    glGetError();

    std::string vsSrc = shaderprecision_shaders("src/shaderprecision.vs", 0);
    std::string fsSrc = shaderprecision_shaders("src/shaderprecision.fs", 0);
    GLint vs = glCreateShader(GL_VERTEX_SHADER);
    require(vs, "Failed to create vertex shader");
    GLint fs = glCreateShader(GL_FRAGMENT_SHADER);
    require(fs, "Failed to create fragment shader");

    GLint length = static_cast<int>( vsSrc.length() );
    const GLchar *src = vsSrc.c_str();
    glShaderSource(vs, 1, &src, &length);

    src = fsSrc.c_str();
    length = static_cast<int>( fsSrc.length() );
    glShaderSource(fs, 1, &src, &length);

    compileShader(vs);
    compileShader(fs);
    prog_ = glCreateProgram();
    require(prog_ != 0, "glCreateProgram returned zero");

    glAttachShader(prog_, vs);
    glAttachShader(prog_, fs);
    require(glGetError() == GL_NO_ERROR);

    glBindAttribLocation(prog_, 0, "a_position");

    glLinkProgram(prog_);
    GLenum glerr = glGetError();
    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint linked;
    glGetProgramiv(prog_, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        std::string message = getInfoLog();
        throw std::runtime_error(message);
    }

    width_ = width;
    height_ = height;
    require(glerr == GL_NO_ERROR, FORMATSTR("gl error in glLinkProgram and GL_LINK_STATUS failed to detect it: %s", glerr));
}

void ShaderPrecision::teardown()
{
    glDeleteProgram(prog_);
}


void ShaderPrecision::calculate(int32_t w, int32_t h)
{
    try
    {
        GLuint colorTex, fbo;
        GLint saveFbo;

        setup(w, h);

        // Create an FBO here with full control over color depth.
        glGenTextures(1, &colorTex);
        glGenFramebuffers(1, &fbo);

        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saveFbo);

        glBindTexture(GL_TEXTURE_2D, colorTex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTex, 0);

        GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            throw std::runtime_error(GL_ERROR_TABLE(status));
        }

        render();
        glFlush();
        glFinish();

        std::vector<char> buf(4*width_*height_);
        glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, buf.data());

        glDeleteTextures(1, &colorTex);
        glDeleteFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, saveFbo);
        teardown();

        ng::PngWriter writer;
        writer.reset(width_, height_, ng::PngIOBase::ct_gray, 8);
        std::vector<uint8_t> row(width_);
        for (int32_t y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
                row[x] = buf[(y*width_*4) + x*4];
            writer.setRow(y, row.data());
        }
        ng::OMemStreamBuf png;
        writer.write(png);
        std::ostringstream os;
        Poco::Base64Encoder enc(os);
        enc.write(reinterpret_cast<const char*>(png.data()), png.size());
        enc.close();
        fragmentPrecisionBase64Png_ = os.str();
#if 0
        std::fstream file("/data/local/tmp/shaderprecision.png", std::fstream::out|std::fstream::binary);
        file.write((const char *)png.data(), png.size());
#endif
    }
    catch(const std::exception &e)
    {
        NGLOG_ERROR("ShaderPrecision: %s", e.what());
    }
}


std::string ShaderPrecision::getInfoLog () const
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


void ShaderPrecision::compileShader(GLuint shader)
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


void ShaderPrecision::render()
{
    glGetString(GL_VERSION);

    glViewport(0, 0, width_, height_);
    glUseProgram(prog_);
    GLuint vbo;

    GLuint vao = 0;
    if (epoxy_is_desktop_gl() && epoxy_gl_version() >= 40) {
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
    }
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const float pos[] = { -1, -1,   1, -1,   1, 1,   -1, 1 };

    glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, 0, 0);
    CHECK_GL_ERROR;
    glEnableVertexAttribArray(0);
    CHECK_GL_ERROR;

    GLuint resolutionLoc = glGetUniformLocation(prog_, "resolution");
    float w = (float) width_;
    float h = (float) height_;
    glUniform2f(resolutionLoc, w, h);
    CHECK_GL_ERROR;

    glValidateProgram(prog_);
    GLint valid = 0;
    glGetProgramiv(prog_, GL_VALIDATE_STATUS, &valid);
    if (!valid)
    {
        NGLOG_INFO("GL_VALIDATE_STATUS: %s", valid);
        NGLOG_INFO("info log: %s", getInfoLog());
    }
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glDeleteBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }
    CHECK_GL_ERROR;
    glUseProgram(0);
    CHECK_GL_ERROR;
}

std::string ShaderPrecision::fragmentPrecisionBase64Png() const
{
    return fragmentPrecisionBase64Png_;
}

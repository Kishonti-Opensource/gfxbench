/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#if defined HAVE_GLES3 || defined HAVE_GLEW

#include "glb_opengl_debug_output.h"
#include "opengl/ext.h"
#include <ng/log.h>

using namespace GLB;

OpenGLDebugOutput::OpenGLDebugOutput()
{

}

void OpenGLDebugOutput::Enable()
{
    if (GLB::g_extension != 0)
    {
        if (GLB::g_extension->hasExtension(GLB::GLBEXT_debug))
        {
            //if (glewIsSupported("GL_ARB_debug_output"))
            {
                glDebugMessageCallbackProc(DebugCallback, NULL);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageControlProc(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
            }
        }
        else
        {
            NGLOG_INFO("Opengl debug output is not supported!");
        }
    }
}

void OpenGLDebugOutput::Disable()
{
    glDisable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
}

void GFXB_APIENTRY OpenGLDebugOutput::DebugCallback(
    KCL::enum_t source,
    KCL::enum_t type,
    KCL::uint32 id,
    KCL::enum_t severity,
    int length,
    const char *message,
    const void *userParam)
{
    std::string source_str;
    switch (source)

    {
        case GL_DEBUG_SOURCE_API_ARB:             source_str = "API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:   source_str = "WINDOW_SYSTEM"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB: source_str = "SHADER_COMPILER"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:     source_str = "THIRD_PARTY"; break;
        case GL_DEBUG_SOURCE_APPLICATION_ARB:     source_str = "APPLICATION"; break;
        case GL_DEBUG_SOURCE_OTHER_ARB:           source_str = "OTHER"; break;
        default:                                  source_str = "UNDEFINED"; break;
    }


    std::string type_str;
    switch (type)
    {

        case GL_DEBUG_TYPE_ERROR_ARB:               type_str = "ERROR"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB: type_str = "DEPRECATED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:  type_str = "UNDEFINED_BEHAVIOR"; break;
        case GL_DEBUG_TYPE_PORTABILITY_ARB:         type_str = "PORTABILITY"; break;
        case GL_DEBUG_TYPE_PERFORMANCE_ARB:         type_str = "PERFORMANCE"; break;
        case GL_DEBUG_TYPE_OTHER_ARB:               type_str = "OTHER"; break;
        default:                                    type_str = "UNDEFINED"; break;
    }


    std::string severity_str;
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH_ARB:        severity_str = "HIGH";   break;
        case GL_DEBUG_SEVERITY_MEDIUM_ARB:      severity_str = "MEDIUM"; break;
        case GL_DEBUG_SEVERITY_LOW_ARB:         severity_str = "LOW"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION:    severity_str = "NOTIFICATION"; break;
        default:                                severity_str = "UNDEFINED"; break;
    }

    NGLOG_DEBUG("ARB Debug: %s [source=%s type=%s severity=%s id=%s]", message, source_str, type_str, severity_str, id);
}

#endif

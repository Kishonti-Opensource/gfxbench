/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include <ng/log.h>

#include "gl_wrapper_private.h"
#include "gl_wrapper_gl31_tokens.h"
#include "opengl/glb_opengl_limits.h"

#ifdef __cplusplus
extern "C" GLboolean glewExperimental;
#endif

GLWrapper* GetGLWrapper()
{
	return GLWrapperImpl::Get();
}


GLWrapperImpl * GLWrapperImpl::instance = NULL;


GLWrapperImpl * GLWrapperImpl::Get()
{
	if (instance == NULL)
	{
		instance = new GLWrapperImpl();
	}
	return instance;
}


GLWrapperImpl::GLWrapperImpl()
{
	m_query_samples = 0;
	m_query_primitives = 0;
	m_query_disabled = false;
    m_measurement_paused = true;

    memset(m_query_pipeline, 0, sizeof(m_query_pipeline));
}


GLWrapperImpl::~GLWrapperImpl()
{
}


void GLWrapperImpl::OnFunctionCall(const char *name)
{
#if !ENABLE_QUICK_STATS
    if (m_measurement_paused)
    {
        return;
    }
	num_fn_called[name]++;
#endif
}


GLB::ContextLimits * GLWrapperImpl::GetLimits()
{
	return GLB::ContextLimits::GetES31_AEP_Limits();
}


void GLWrapperImpl::OnActiveTexture(GLenum texture)
{
#if !ENABLE_QUICK_STATS
	GLB::ContextLimits *limits = GetLimits();
	if (!limits)
	{
		return;
	}
	GLuint unit = texture - GL_TEXTURE0;
	if (unit > limits->m_max_texture_binding)
	{
		NGLOG_ERROR("GL_WRAPPER - glActiveTexture: invalid texture unit: %s max is %s. profile: %s", unit, limits->m_max_texture_binding, limits->name);
		assert(!GL_WRAPPER_ASSERT_ON_ERROR);
	}
#endif
}



void GLWrapperImpl::OnBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)
{
#if !ENABLE_QUICK_STATS
    if (m_measurement_paused)
    {
        return;
    }
    
	int bindingId = 0;
	GL_Impl::glGetIntegerv(GetBufferBinding(target), &bindingId);	
	assert( (bindingId != 0) && "empty binding id");

	session_it it;
	for (it = m_session.begin(); it != m_session.end(); it++)
	{
		if (it->second.measure_flags & MeasureFlags::VertexBuffer)
		{
			it->second.m_bufferObjects[bindingId] = size;
		}
	}
#endif
}


void GLWrapperImpl::OnDeleteBuffers(GLsizei n, const GLuint *buffers)
{
#if !ENABLE_QUICK_STATS
    if (m_measurement_paused)
    {
        return;
    }

	session_it it;
	for (it = m_session.begin(); it != m_session.end(); it++)
	{
		if (it->first & MeasureFlags::VertexBuffer)
		{
			for (int i = 0; i < n; i++)
			{
				it->second.m_bufferObjects.erase(buffers[i]);
			}
		}
	}
#endif
}


void GLWrapperImpl::OnDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)
{
#if !ENABLE_QUICK_STATS
    if (m_measurement_paused)
    {
        return;
    }

	session_it it;
	for (it = m_session.begin(); it != m_session.end(); it++)
	{
		it->second.m_dispatch_count++;
	}
#endif
}


void GLWrapperImpl::OnBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
#if !ENABLE_QUICK_STATS
	GLB::ContextLimits *limits = GetLimits();
	if (!limits)
	{
		return;
	}
	if (unit > limits->m_max_combinded_resource[GLB::ContextLimits::RES_IMAGE_UNIFORM])
	{
		NGLOG_ERROR("GL_WRAPPER - glBindImageTexture: invalid texture unit: %s max is %s. profile: %s", unit, limits->m_max_combinded_resource[GLB::ContextLimits::RES_IMAGE_UNIFORM], limits->name);
		assert(!GL_WRAPPER_ASSERT_ON_ERROR);
	}
#endif
}

void GLWrapperImpl::OnBindSampler(GLuint unit, GLuint sampler)
{
#if !ENABLE_QUICK_STATS
	GLB::ContextLimits *limits = GetLimits();
	if (!limits)
	{
		return;
	}
	if (unit > limits->m_max_texture_binding)
	{
		NGLOG_ERROR("GL_WRAPPER - glBindSampler: invalid texture unit: %s max is %s. profile: %s", unit, limits->m_max_texture_binding, limits->name);
		assert(!GL_WRAPPER_ASSERT_ON_ERROR);
	}
#endif
}


void GLWrapperImpl::OnGenObject(GlObjectType type, GLsizei n, const GLuint *ids)
{
	std::map<GLuint,int> &object_pool = m_object_counter[type] ;
	std::string type_name = GLObjectTypeToString(type) ;

	for (int i = 0 ; i < n ; i++)
	{
		GLuint id = ids[i] ;

		bool key_exists = object_pool.find(id) != object_pool.end() ;

		if (!key_exists)
		{
			object_pool[id] = 0 ;
		}

		if (object_pool[id] != 0)
		{
			NGLOG_ERROR("GL_WRAPPER - create %s id %s count %s",type_name.c_str(),id,object_pool[id]) ;
		}

		object_pool[id]++ ;

		//NGLOG_INFO("GL_WRAPPER - create %s id %s count %s",type_name.c_str(),id,object_pool[id]) ;
	}
}


void GLWrapperImpl::OnDeleteObject(GlObjectType type, GLsizei n, const GLuint *ids)
{
	std::map<GLuint,int> &object_pool = m_object_counter[type] ;
	std::string type_name = GLObjectTypeToString(type) ;

	for (int i = 0 ; i < n ; i++)
	{
		GLuint id = ids[i] ;

		bool key_exists = object_pool.find(id) != object_pool.end() ;

		if (!key_exists)
		{
			object_pool[id] = 0 ;
		}

		if (object_pool[id] != 1)
		{
			// silently ignore id 0
			if (id != 0) 
			{
				NGLOG_ERROR("GL_WRAPPER - delete %s id %s count %s",type_name.c_str(),id,object_pool[id]) ;
			}
		}

		object_pool[id]-- ;

		// silently ignore id 0
		if (id != 0) 
		{
			//NGLOG_INFO("GL_WRAPPER - delete %s id %s count %s",type_name.c_str(),id,object_pool[id]) ;
		}
	}
}


void GLWrapperImpl::CheckLeaks()
{
	for (int type = 0 ; type < TYPE_COUNT ; type++)
	{
		std::map<GLuint,int> &object_pool = m_object_counter[type] ;
		std::string type_name = GLObjectTypeToString(type) ;

		std::stringstream leaks_stream ;

		leaks_stream<<type_name<<" leaks: " ;

		for (std::map<GLuint,int>::const_iterator it = object_pool.begin() ; it != object_pool.end() ; it++)
		{
			// silently ignore the 0 id 
			if (it->first == 0)
			{
				continue ;
			}

			if (it->second == 1)
			{
				leaks_stream<<it->first<<", " ;
			}
		}

		NGLOG_ERROR("%s",leaks_stream.str().c_str()) ;
	}

	for (int type = 0 ; type < TYPE_COUNT ; type++)
	{
		std::map<GLuint,int> &object_pool = m_object_counter[type] ;
		std::string type_name = GLObjectTypeToString(type) ;

		std::stringstream error_stream ;

		error_stream<<type_name<<" reserve count invalid value: " ;

		for (std::map<GLuint,int>::const_iterator it = object_pool.begin() ; it != object_pool.end() ; it++)
		{
			// silently ignore the 0 id 
			if (it->first == 0)
			{
				continue ;
			}

			bool invalid = (it->second != 0) && (it->second != 1) ;
			if (invalid)
			{
				error_stream<<it->first<<" ("<<it->second<<"), " ;
			}
		}

		NGLOG_ERROR("%s",error_stream.str().c_str()) ;
	}
}


void GLWrapperImpl::CheckGLError(const char * fn)
{
#ifdef _DEBUG 
#if ENABLE_GL_ERROR_CHECKING
	GLenum error = GL_Impl::glGetError();
	if (error != GL_NO_ERROR)
	{
		NGLOG_ERROR("GL_WRAPPER - GL error after calling: %s - %s", fn, GetErrorString(error));
		assert(!GL_WRAPPER_ASSERT_ON_ERROR);
	}
#endif
#endif
}

const char * GLWrapperImpl::GetErrorString(GLenum value)
{
	switch(value)
	{
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_OPERATION:
		return("GL_INVALID_OPERATION");
	case GL_INVALID_VALUE:
		return("GL_INVALID_VALUE");
	case GL_OUT_OF_MEMORY:
		return("GL_OUT_OF_MEMORY");
	default:
		return "Unknown error code";
	}
}

GLuint GLWrapperImpl::GetBufferBinding(GLenum target)
{
    switch (target)
    {
    case GL_ARRAY_BUFFER:
        return GL_ARRAY_BUFFER_BINDING;

    case GL_ATOMIC_COUNTER_BUFFER:
        return GL_ATOMIC_COUNTER_BUFFER_BINDING;

    case GL_COPY_READ_BUFFER:
        return GL_COPY_READ_BUFFER_BINDING;

    case GL_COPY_WRITE_BUFFER:
        return GL_COPY_WRITE_BUFFER_BINDING;

    case GL_DRAW_INDIRECT_BUFFER:
        return GL_DRAW_INDIRECT_BUFFER_BINDING;

    case GL_DISPATCH_INDIRECT_BUFFER:
        return GL_DISPATCH_INDIRECT_BUFFER_BINDING;

    case GL_ELEMENT_ARRAY_BUFFER:
        return GL_ELEMENT_ARRAY_BUFFER_BINDING;

     case GL_PIXEL_PACK_BUFFER:
         return GL_PIXEL_PACK_BUFFER_BINDING;

     case GL_PIXEL_UNPACK_BUFFER:
         return GL_PIXEL_UNPACK_BUFFER_BINDING;

     case GL_SHADER_STORAGE_BUFFER:
         return GL_SHADER_STORAGE_BUFFER_BINDING;

     case GL_TRANSFORM_FEEDBACK_BUFFER:
         return GL_TRANSFORM_FEEDBACK_BUFFER_BINDING;

    case GL_UNIFORM_BUFFER:
        return GL_UNIFORM_BUFFER_BINDING;

    default:
        assert("GLWrapperImpl::GetBufferBinding - unkown buffer binding");
        return 0;
    }
}

GLenum glewInit()
{
#ifdef HAVE_GLEW
	NGLOG_INFO("OpenGL wrapper enabled");
	GL_Impl::glewExperimental = glewExperimental;
	return GL_Impl::glewInit();
#else
	return 0;
#endif
}
#ifdef HAVE_GLEW
#pragma push_macro("glBlendFunci")
#undef glBlendFunci
void GL_APIENTRY glBlendFunci(GLuint buf, GLenum sfactor, GLenum dfactor)
{
	#pragma pop_macro("glBlendFunci")
#ifdef HAVE_GLEW
	GL_Impl::glBlendFunci(buf, sfactor, dfactor);
	GLWrapperImpl::Get()->CheckGLError("glBlendFunci");
#endif
}

void GL_APIENTRY glDrawBuffer(GLenum mode)
{
#ifdef HAVE_GLEW
	GL_Impl::glDrawBuffer(mode);
	GLWrapperImpl::Get()->CheckGLError("glDrawBuffer");
#endif
}

void GL_APIENTRY glDepthRange(float zNear, float zFar)
{
#ifdef HAVE_GLEW
	GL_Impl::glDepthRange(zNear, zFar);
	GLWrapperImpl::Get()->CheckGLError("glDepthRange");
#endif
}

void GL_APIENTRY glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels)
{
#ifdef HAVE_GLEW
	GL_Impl::glGetTexImage(target, level, format, type, pixels);
	GLWrapperImpl::Get()->CheckGLError("glGetTexImage");
#endif
}

void GL_APIENTRY glPolygonMode(GLenum face, GLenum mode)
{
#ifdef HAVE_GLEW
	GL_Impl::glPolygonMode(face, mode);
	GLWrapperImpl::Get()->CheckGLError("glPolygonMode");
#endif
}

#pragma push_macro("glPatchParameteri")
#undef glPatchParameteri
void GL_APIENTRY glPatchParameteri(GLenum pname, GLint value)
{
#pragma pop_macro("glPatchParameteri")
#ifdef HAVE_GLEW
	GL_Impl::glPatchParameteri(pname, value);
	GLWrapperImpl::Get()->CheckGLError("glPatchParameteri");
#endif
};

#pragma push_macro("glGetQueryObjectui64v")
#undef glGetQueryObjectui64v
void GL_APIENTRY glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64* params)
{
#pragma pop_macro("glGetQueryObjectui64v")
#ifdef HAVE_GLEW
    GL_Impl::glGetQueryObjectui64v(id, pname, params);
    GLWrapperImpl::Get()->CheckGLError("glGetQueryObjectui64v");
#endif
};

GLboolean glewIsSupported(const char *name)
{
#ifdef HAVE_GLEW
	return GL_Impl::glewIsSupported(name);
#else
	return false;
#endif
}

#pragma push_macro("glDebugMessageCallback")
#undef glDebugMessageCallback
typedef void (GL_APIENTRY *GLDEBUGPROCARB)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
void GL_APIENTRY glDebugMessageCallback(GLDEBUGPROCARB callback, const void *userParam)
{
#pragma pop_macro("glDebugMessageCallback")
	GL_Impl::glDebugMessageCallback(*((GL_Impl::GLDEBUGPROCARB*)&callback), userParam);
}

#pragma push_macro("glGetDebugMessageLog")
#undef glGetDebugMessageLog
GLuint GL_APIENTRY glGetDebugMessageLog(GLuint count, GLsizei bufsize, GLenum* sources, GLenum* types, GLuint* ids, GLenum* severities, GLsizei* lengths, GLchar* messageLog)
{
#pragma pop_macro("glGetDebugMessageLog")
	return GL_Impl::glGetDebugMessageLog(count, bufsize, sources, types, ids, severities, lengths, messageLog);
}

#pragma push_macro("glDebugMessageControl")
#undef glDebugMessageControl
void GL_APIENTRY glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint* ids, GLboolean enabled)
{
#pragma pop_macro("glDebugMessageControl")
	GL_Impl::glDebugMessageControl(source, type, severity, count, ids, enabled);
}

#pragma push_macro("glDebugMessageInsert")
#undef glDebugMessageInsert
void GL_APIENTRY glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* buf)
{
#pragma pop_macro("glDebugMessageInsert")
    GL_Impl::glDebugMessageInsert(source, type, id, severity, length, buf);
}

#pragma push_macro("glGetObjectLabel")
#undef glGetObjectLabel
void GL_APIENTRY glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei* length, GLchar *label)
{
#pragma pop_macro("glGetObjectLabel")
    GL_Impl::glGetObjectLabel(identifier, name, bufSize, length, label);
}

#pragma push_macro("glGetObjectPtrLabel")
#undef glGetObjectPtrLabel
void GL_APIENTRY glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei* length, GLchar *label)
{
#pragma pop_macro("glGetObjectPtrLabel")
    GL_Impl::glGetObjectPtrLabel(ptr, bufSize, length, label);
}

#pragma push_macro("glObjectLabel")
#undef glObjectLabel
void GL_APIENTRY glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar* label)
{
#pragma pop_macro("glObjectLabel")
    GL_Impl::glObjectLabel(identifier, name, length, label);
}

#pragma push_macro("glObjectPtrLabel")
#undef glObjectPtrLabel
void GL_APIENTRY glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar* label)
{
#pragma pop_macro("glObjectPtrLabel")
     GL_Impl::glObjectPtrLabel(ptr, length, label);
}

#pragma push_macro("glPopDebugGroup")
#undef glPopDebugGroup
void GL_APIENTRY glPopDebugGroup(void)
{
#pragma pop_macro("glPopDebugGroup")
    GL_Impl::glPopDebugGroup();
}

#pragma push_macro("glPushDebugGroup")
#undef glPushDebugGroup
void GL_APIENTRY glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message)
{
#pragma pop_macro("glPushDebugGroup")
    GL_Impl::glPushDebugGroup(source, id, length, message);
}

#pragma push_macro("glGetPointerv")
#undef glGetPointerv
void GL_APIENTRY glGetPointerv (GLenum pname, void* *params)
{
    #pragma pop_macro("glGetPointerv")
    GL_Impl::glGetPointerv(pname, params);
}

#endif

void GLWrapperImpl::BeginMeasureQuick()
{
    m_quick_prims = 0; 
    m_quick_draws = 0;

#if ENABLE_QUICK_STATS
#ifdef HAVE_GLEW
    for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
    {
        if(m_query_pipeline[i] == 0)
        {
            GL_Impl::glGenQueries(1, &m_query_pipeline[i]);
        }
    }

    //GL_Impl::glBeginQuery(GL_VERTICES_SUBMITTED_ARB, m_query_pipeline[0]);
    //GL_Impl::glBeginQuery(GL_PRIMITIVES_SUBMITTED_ARB, m_query_pipeline[1]);
    //GL_Impl::glBeginQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB, m_query_pipeline[2]);
    //GL_Impl::glBeginQuery(GL_TESS_CONTROL_SHADER_PATCHES_ARB, m_query_pipeline[3]);
    GL_Impl::glBeginQuery(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB, m_query_pipeline[4]);
    //GL_Impl::glBeginQuery(GL_GEOMETRY_SHADER_INVOCATIONS, m_query_pipeline[5]);
    //GL_Impl::glBeginQuery(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB, m_query_pipeline[6]);
    //GL_Impl::glBeginQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, m_query_pipeline[7]);
    //GL_Impl::glBeginQuery(GL_COMPUTE_SHADER_INVOCATIONS_ARB, m_query_pipeline[8]);
    //GL_Impl::glBeginQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB, m_query_pipeline[9]);
    //GL_Impl::glBeginQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, m_query_pipeline[10]);
#endif
#endif
}

void GLWrapperImpl::EndMeasureQuick(KCL::uint32& o_qd, KCL::uint32& o_qp, KCL::uint32& o_qpq)
{
#if ENABLE_QUICK_STATS
    const int valueToCheck = 4;

    //GL_Impl::glEndQuery(GL_VERTICES_SUBMITTED_ARB); // 0
    //GL_Impl::glEndQuery(GL_PRIMITIVES_SUBMITTED_ARB); // 1 
    //GL_Impl::glEndQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB); // 2 
    //GL_Impl::glEndQuery(GL_TESS_CONTROL_SHADER_PATCHES_ARB); // 3 
    GL_Impl::glEndQuery(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB); // 4 
    //GL_Impl::glEndQuery(GL_GEOMETRY_SHADER_INVOCATIONS); // 5 
    //GL_Impl::glEndQuery(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB); // 6
    //GL_Impl::glEndQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB); // 7
    //GL_Impl::glEndQuery(GL_COMPUTE_SHADER_INVOCATIONS_ARB); // 8
    //GL_Impl::glEndQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB); // 9
    //GL_Impl::glEndQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB); // 10

    // Wait for the result and query it
    GLint ready = GL_FALSE;
    //for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
    //{
        while (!ready)
	    {
            GL_Impl::glGetQueryObjectiv(m_query_pipeline[valueToCheck/*i*/], GL_QUERY_RESULT_AVAILABLE, &ready);
        }
        ready = GL_FALSE;
    //}

    GL_Impl::GLuint pipe_stats_values[DrawCallStatistics::query_stats_count] = {};
    //for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
    {
        GL_Impl::glGetQueryObjectuiv(m_query_pipeline[valueToCheck/*i*/], GL_QUERY_RESULT, &pipe_stats_values[valueToCheck/*i*/]);
    }

    o_qd = m_quick_draws;
    o_qp = m_quick_prims;
    o_qpq = pipe_stats_values[valueToCheck];
#endif
}

void GLWrapperImpl::OnPreDrawCall(GLenum mode, GLsizei vertex_count, GLsizei instance_count)
{
#if !ENABLE_QUICK_STATS
    if (m_measurement_paused)
    {
        return;
    }

	unsigned int measure_flags = 0;

	session_it it;
	for (it = m_session.begin(); it != m_session.end(); it++)
	{
		measure_flags |= it->second.measure_flags;
	}

	if (measure_flags == 0)
	{
		// No measure
		return;
	}
	
	DrawCallStatistics draw_call;

//	NOTE: "manual" vertex + triangle counting is superseded by using ARB_pipeline_statistics_query
//
//	if (measure_flags & MeasureFlags::DrawCalls)
//	{
//		draw_call.num_draw_vertices = vertex_count * instance_count;
//        draw_call.mode = mode;
//
//		switch (mode)
//		{
//		case GL_POINTS:
//		case GL_LINES:
//			break;
//		case GL_TRIANGLES:
//			draw_call.num_draw_triangles = draw_call.num_draw_vertices / 3;
//			break;
//		case GL_TRIANGLE_STRIP:
//		case GL_TRIANGLE_FAN:
//			draw_call.num_draw_triangles = (vertex_count - 2) * instance_count;
//			break;
//#ifdef HAVE_GLEW
//		case GL_PATCHES:
//			//TODO GL_PATCHES stats
//			break;
//#endif
//		default:
//			NGLOG_ERROR("GLWrapperImpl - onDrawCall Unknown mode:%s\n", mode);
//			break;
//		}
//	}


	if (measure_flags & MeasureFlags::TextureCount)
	{
		GL_Impl::glGetIntegerv(GL_CURRENT_PROGRAM, &draw_call.program);
		if (draw_call.program > 0)
		{
			collectActiveTextures(draw_call);
		}
	}

#ifdef HAVE_GLEW
//GL_SAMPLES_PASSED only in GL
	if (measure_flags & MeasureFlags::Samples)
	{
		if (m_query_samples == 0)
		{
			GL_Impl::glGenQueries(1, &m_query_samples);
		}
		if (m_query_primitives == 0)
		{
			GL_Impl::glGenQueries(1, &m_query_primitives);
		}
		

		int active_samples_query;
		int active_any_samples_query;
		GL_Impl::glGetQueryiv(GL_ANY_SAMPLES_PASSED, GL_CURRENT_QUERY, &active_any_samples_query);
		GL_Impl::glGetQueryiv(GL_SAMPLES_PASSED, GL_CURRENT_QUERY, &active_samples_query);
		GL_Impl::glBeginQuery(GL_PRIMITIVES_GENERATED, m_query_primitives);
		if (active_any_samples_query + active_samples_query == 0)
		{
			GL_Impl::glBeginQuery(GL_SAMPLES_PASSED, m_query_samples);
			m_query_disabled = false;
		}
		else
		{
			m_query_disabled = true;
		}
	}
    if(measure_flags & MeasureFlags::PipeStats)
    {
        for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
        {
            if(m_query_pipeline[i] == 0)
            {
                GL_Impl::glGenQueries(1, &m_query_pipeline[i]);
            }
        }
/*
        int a = 0;
		GL_Impl::glGetQueryiv(GL_VERTICES_SUBMITTED_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_PRIMITIVES_SUBMITTED_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_VERTEX_SHADER_INVOCATIONS_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_TESS_CONTROL_SHADER_PATCHES_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_GEOMETRY_SHADER_INVOCATIONS, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_COMPUTE_SHADER_INVOCATIONS_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_CLIPPING_INPUT_PRIMITIVES_ARB, GL_CURRENT_QUERY, &a);
        GL_Impl::glGetQueryiv(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, GL_CURRENT_QUERY, &a);
*/

        GL_Impl::glBeginQuery(GL_VERTICES_SUBMITTED_ARB, m_query_pipeline[0]);
        GL_Impl::glBeginQuery(GL_PRIMITIVES_SUBMITTED_ARB, m_query_pipeline[1]);
        GL_Impl::glBeginQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB, m_query_pipeline[2]);
        GL_Impl::glBeginQuery(GL_TESS_CONTROL_SHADER_PATCHES_ARB, m_query_pipeline[3]);
        GL_Impl::glBeginQuery(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB, m_query_pipeline[4]);
        GL_Impl::glBeginQuery(GL_GEOMETRY_SHADER_INVOCATIONS, m_query_pipeline[5]);
        GL_Impl::glBeginQuery(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB, m_query_pipeline[6]);
        GL_Impl::glBeginQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB, m_query_pipeline[7]);
        GL_Impl::glBeginQuery(GL_COMPUTE_SHADER_INVOCATIONS_ARB, m_query_pipeline[8]);
        GL_Impl::glBeginQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB, m_query_pipeline[9]);
        GL_Impl::glBeginQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB, m_query_pipeline[10]);
    }
#endif


	for (it = m_session.begin(); it != m_session.end(); it++)
	{
		if (measure_flags & it->first)
		{
			it->second.draw_calls.push_back(draw_call);
		}
	}
#else
    if(mode == GL_TRIANGLES)
    {
        m_quick_prims += vertex_count * instance_count / 3;
    }

    ++m_quick_draws;
#endif
}

void GLWrapperImpl::OnPostDrawCall()
{
#if !ENABLE_QUICK_STATS
    if (m_measurement_paused)
    {
        return;
    }

	unsigned int measure_flags = 0;
	if (m_query_disabled)
	{
		return;
	}

	session_it it;
	for (it = m_session.begin(); it != m_session.end(); it++)
	{
		measure_flags |= it->second.measure_flags;
	}
	
#ifdef HAVE_GLEW
	if (measure_flags & MeasureFlags::Samples)
	{
		GL_Impl::glEndQuery(GL_SAMPLES_PASSED);

		// Wait for the result and query it
		GLint ready = GL_FALSE;
		while (!ready)
		{
			GL_Impl::glGetQueryObjectiv(m_query_samples, GL_QUERY_RESULT_AVAILABLE, &ready);
		}
		GLuint num_samples_passed;
		GL_Impl::glGetQueryObjectuiv(m_query_samples, GL_QUERY_RESULT, &num_samples_passed);



		GL_Impl::glEndQuery(GL_PRIMITIVES_GENERATED);

		while (1)
		{
			int avail = 0;
			GL_Impl::glGetQueryObjectiv(m_query_primitives, GL_QUERY_RESULT_AVAILABLE, &avail);
			if (avail)
			{
				break;
			}
		}
		int generated_primitives;
		GL_Impl::glGetQueryObjectiv(m_query_primitives, GL_QUERY_RESULT, &generated_primitives);

		for (it = m_session.begin(); it != m_session.end(); it++)
		{
			if (measure_flags & it->first)
			{
                DrawCallStatistics& dc = it->second.draw_calls.back();
		        
                dc.num_samples_passed = num_samples_passed;
                dc.generated_primitives = generated_primitives;
            }
        }
    }
    
    if(measure_flags & MeasureFlags::PipeStats)
    {
    	GL_Impl::glEndQuery(GL_VERTICES_SUBMITTED_ARB);
        GL_Impl::glEndQuery(GL_PRIMITIVES_SUBMITTED_ARB);
        GL_Impl::glEndQuery(GL_VERTEX_SHADER_INVOCATIONS_ARB);
        GL_Impl::glEndQuery(GL_TESS_CONTROL_SHADER_PATCHES_ARB);
        GL_Impl::glEndQuery(GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB);
        GL_Impl::glEndQuery(GL_GEOMETRY_SHADER_INVOCATIONS);
        GL_Impl::glEndQuery(GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB);
        GL_Impl::glEndQuery(GL_FRAGMENT_SHADER_INVOCATIONS_ARB);
        GL_Impl::glEndQuery(GL_COMPUTE_SHADER_INVOCATIONS_ARB);
        GL_Impl::glEndQuery(GL_CLIPPING_INPUT_PRIMITIVES_ARB);
        GL_Impl::glEndQuery(GL_CLIPPING_OUTPUT_PRIMITIVES_ARB);

		// Wait for the result and query it
		GLint ready = GL_FALSE;
        for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
		{
            while (!ready)
		    {
                GL_Impl::glGetQueryObjectiv(m_query_pipeline[i], GL_QUERY_RESULT_AVAILABLE, &ready);
            }
            ready = GL_FALSE;
		}

        GL_Impl::GLuint pipe_stats_values[DrawCallStatistics::query_stats_count] = {};
        for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
        {
            GL_Impl::glGetQueryObjectuiv(m_query_pipeline[i], GL_QUERY_RESULT, &pipe_stats_values[i]);
        }

		//float factor = 1.0f * iGL_QUERY_RESULT / num_indices;

		//if( factor > 1.0f)
		//printf( "m_GL_PRIMITIVES_GENERATED : %d, %s, %s, %d, %d, %f\n", num_instances, sm->m_name.c_str(), sm->m_mesh->m_name.c_str(), static_cast<GLsizei>(sm->m_mesh->getIndexCount(lod)), iGL_QUERY_RESULT * 3, factor);
		//printf("m_GL_PRIMITIVES_GENERATED : %d\n",  iGL_QUERY_RESULT);


		for (it = m_session.begin(); it != m_session.end(); it++)
		{
			if (measure_flags & it->first)
			{
                DrawCallStatistics& dc = it->second.draw_calls.back();

                /*
                assert(dc.num_draw_vertices == pipe_stats_values[0]);
                if(dc.mode == GL_TRIANGLES)
                {
                    assert(it->second.draw_calls.back().num_draw_triangles == pipe_stats_values[1]);
                }
                if(it->second.draw_calls.back().num_samples_passed > 0)
                {
                    assert(pipe_stats_values[7] > 0);
                }
                */

                for(int i=0; i<DrawCallStatistics::query_stats_count; ++i)
                {
                    dc.num_query_stats[i] = pipe_stats_values[i];
                }

                if(dc.num_query_stats[3] > 0)
                {
                    dc.uses_TES = true;
                }
                if(dc.num_query_stats[5] > 0)
                {
                    dc.uses_GS = true;
                }
			}
		}
	}
#endif
#endif
}


GLint texture_unit[256];

void GLWrapperImpl::collectActiveTextures(DrawCallStatistics & draw_call)
{
#if !ENABLE_QUICK_STATS
	// Save the current active texture unit
	GLint saved_texture_unit;
	GL_Impl::glGetIntegerv(GL_ACTIVE_TEXTURE, &saved_texture_unit);
	int tmp = saved_texture_unit - GL_TEXTURE0;

	// Query the number of uniforms
	GLint num_active_uniforms;
	GLint uniform_len;
	GL_Impl::glGetProgramiv(draw_call.program, GL_ACTIVE_UNIFORMS, &num_active_uniforms);
	GL_Impl::glGetProgramiv(draw_call.program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniform_len);

	std::vector < char > uniform_name;
	uniform_name.resize(uniform_len);
	// Iterate over the unifroms and select the samplers
	for (int uniform_index = 0; uniform_index < num_active_uniforms; uniform_index++)
	{
		GLenum uniform_type = 0;
		GLint uniform_size;

		GLsizei length;
		GL_Impl::glGetActiveUniform(draw_call.program, uniform_index, uniform_len, &length, &uniform_size, &uniform_type, uniform_name.data());
		GLint uniform_location = GL_Impl::glGetUniformLocation(draw_call.program, uniform_name.data());
		// Map the sampler to texture target
		GL_Impl::GLenum target;

		switch (uniform_type)
		{
#ifdef HAVE_GLEW
        case GL_IMAGE_2D:
			int name;
            GL_Impl::glGetIntegeri_v(GL_IMAGE_BINDING_NAME, uniform_index, &name);
            break;
#endif
#ifdef HAVE_GLEW
		case GL_SAMPLER_1D:
			target = GL_TEXTURE_BINDING_1D;
			break;
#endif
		case GL_SAMPLER_2D:
		case GL_SAMPLER_2D_SHADOW:
			target = GL_TEXTURE_BINDING_2D;
			break;
		case GL_SAMPLER_2D_ARRAY_SHADOW:
			target = GL_TEXTURE_BINDING_2D_ARRAY;
			break;
		case GL_SAMPLER_3D:
			target = GL_TEXTURE_BINDING_3D;
			break;
#ifdef HAVE_GLEW
		case GL_SAMPLER_1D_ARRAY:
			target = GL_TEXTURE_BINDING_1D_ARRAY;
			break;
#endif
		case GL_SAMPLER_2D_ARRAY:
			target = GL_TEXTURE_BINDING_2D_ARRAY;
			break;
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_CUBE_SHADOW:
			target = GL_TEXTURE_BINDING_CUBE_MAP;
			break;
#ifdef HAVE_GLEW
		case GL_SAMPLER_CUBE_MAP_ARRAY:
			target = GL_TEXTURE_BINDING_CUBE_MAP_ARRAY;
			break;
#endif
#ifdef HAVE_GLEW
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
			target = GL_TEXTURE_BINDING_RECTANGLE;
			break;
#endif
		default:
			{
				bool s = GLB::ContextLimits::IsImageType(uniform_type);
				bool s2 = GLB::ContextLimits::IsSamplerType(uniform_type);
				assert(!(s == true || s2 == true));
				// Not a sampler
				continue;
			}
		}

		GL_Impl::glGetUniformiv(draw_call.program, uniform_location, texture_unit);

		// Set the texture of the texture unit
		GL_Impl::glActiveTexture(GL_TEXTURE0 + (GLint)texture_unit[0]);

		// Get the texture
		GLint texture_id = 0;
		GL_Impl::glGetIntegerv(target, &texture_id);
		if (texture_id != 0)
		{
			draw_call.texture_ids.insert(texture_id);
			draw_call.num_active_textures++;
		}
	}

	// Restore the state
	GL_Impl::glActiveTexture(saved_texture_unit);
#endif
}


const KCL::uint32 GLWrapperImpl::BeginMeasure(const unsigned int flags)
{
	int size = m_session.size();
	MeasureResults mr;
	mr.measure_flags = flags;
	m_session[++size] = mr;
    m_measurement_paused = false;
	return size;
}


MeasureResults GLWrapperImpl::EndMeasure(const KCL::uint32 id)
{
	MeasureResults res (m_session.at(id));
	m_session.erase(id);
	return res;
}


void GLWrapperImpl::PauseMeasurement()
{
    m_measurement_paused = true;
}

void GLWrapperImpl::ResumeMeasurement()
{
     m_measurement_paused = false;
}

bool GLWrapperImpl::IsMeasurementPaused()
{
    return m_measurement_paused;
}


std::string GLObjectTypeToString(int type)
{
	switch (type)
	{
		case GLWrapperImpl::TYPE_BUFFER             : return "buffer" ;
		case GLWrapperImpl::TYPE_SHADER             : return "shader" ;
		case GLWrapperImpl::TYPE_PROGRAM            : return "program" ;
		case GLWrapperImpl::TYPE_PROGRAM_PIPELINE   : return "program_pipeline" ;
		case GLWrapperImpl::TYPE_TEXTURE            : return "texture" ;
		case GLWrapperImpl::TYPE_SAMPLER            : return "sampler" ;
		case GLWrapperImpl::TYPE_RENDERBUFFER       : return "renderbuffer" ;
		case GLWrapperImpl::TYPE_FRAMEBUFFER        : return "framebuffer" ;
		case GLWrapperImpl::TYPE_VERTEX_ARRAY       : return "vertex_array" ;
		case GLWrapperImpl::TYPE_TRANSFORM_FEEDBACK : return "transform feedback" ;
		case GLWrapperImpl::TYPE_QUERY              : return "query" ;
		case GLWrapperImpl::TYPE_SYNC               : return "sync" ;
	}
	return "Unknown" ;
	
}

